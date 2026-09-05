#include "HAMqtt.h"

#include <cstdio>
#include <new>
#include <cstring>

#ifndef ARDUINOHA_TEST
#include <PubSubClient.h>
#endif

#include "ArduinoHALog.h"
#include "ArduinoHALogTemplates.h"
#include "HADevice.h"
#include "device-types/HABaseDeviceType.h"
#include "mocks/PubSubClientMock.h"
#include "utils/HADictionary.h"
#include "utils/HAJson.h"
#include "utils/HASerializer.h"

namespace {
constexpr char kMqtt[] = "mqtt";
constexpr char kDiscovery[] = "discovery";
} // namespace

#define HAMQTT_INIT \
    _device(device), \
    _messageCallback(nullptr), \
    _connectedCallback(nullptr), \
    _disconnectedCallback(nullptr), \
    _stateChangedCallback(nullptr), \
    _initialized(false), \
    _discoveryPrefix(DefaultDiscoveryPrefix), \
    _dataPrefix(DefaultDataPrefix), \
    _deviceDiscoveryEnabled(false), \
    _deviceDiscoveryMigrationState(DeviceDiscoveryMigrationIdle), \
    _originSupportUrl(nullptr), \
    _username(nullptr), \
    _password(nullptr), \
    _lastConnectionAttemptAt(0), \
    _reconnectInterval(DefaultReconnectInterval), \
    _devicesTypesNb(0), \
    _maxDevicesTypesNb(maxDevicesTypesNb), \
    _devicesTypes(new HABaseDeviceType*[maxDevicesTypesNb]), \
    _lastWillTopic(nullptr), \
    _deviceTypeRegistrationFailures(0), \
    _lastWillMessage(nullptr), \
    _lastWillRetain(false), \
    _currentState(StateDisconnected), \
    _lastLoopOkAt(0), \
    _lastMessageAt(0), \
    _lastPublishAt(0), \
    _lastDisconnectAt(0), \
    _lastDisconnectReason(DiagnosticDisconnectReason::None), \
    _messageDispatchDepth(0), \
    _deferredQueue{}, \
    _deferredHead(0), \
    _deferredCount(0), \
    _deferredBuilder()

static const char* DefaultDiscoveryPrefix = "homeassistant";
static const char* DefaultDataPrefix = "aha";
static const char* DeviceDiscoveryOriginName = "ArduinoHA";
static const char* DeviceDiscoveryMigrationPayload = "{\"migrate_discovery\":true}";

HAMqtt* HAMqtt::_instance = nullptr;

void onMessageReceived(char* topic, uint8_t* payload, unsigned int length)
{
    if (HAMqtt::instance() == nullptr || length > UINT16_MAX) {
        return;
    }

    HAMqtt::instance()->processMessage(topic, payload, static_cast<uint16_t>(length));
}

#ifdef ARDUINOHA_TEST
HAMqtt::HAMqtt(
    PubSubClientMock* pubSub,
    HADevice& device,
    uint8_t maxDevicesTypesNb
) :
    _mqtt(pubSub),
    _directPublishBufferLength(0),
    _directPublishActive(false),
    HAMQTT_INIT
{
    _instance = this;
    HABaseDeviceType::registerAllWith(*this);
}
#else
HAMqtt::HAMqtt(
    Client& netClient,
    HADevice& device,
    uint8_t maxDevicesTypesNb
) :
    _mqttStorage(netClient),
    _mqtt(&_mqttStorage),
    _directPublishBufferLength(0),
    _directPublishActive(false),
    HAMQTT_INIT
{
    _instance = this;
    HABaseDeviceType::registerAllWith(*this);
}
#endif

HAMqtt::~HAMqtt()
{
    clearDeferredBuilder();
    clearDeferredQueue();
    delete[] _devicesTypes;

#ifdef ARDUINOHA_TEST
    if (_mqtt) {
        delete _mqtt;
    }
#endif

    _instance = nullptr;
}

const char* HAMqtt::diagnosticDisconnectReasonText(DiagnosticDisconnectReason reason)
{
    switch (reason) {
        case DiagnosticDisconnectReason::None:
            return "none";
        case DiagnosticDisconnectReason::LoopReturnedFalse:
            return "loop_false";
        case DiagnosticDisconnectReason::DeferredBeginPublishFailed:
            return "deferred_begin_publish";
        case DiagnosticDisconnectReason::DeferredEndPublishFailed:
            return "deferred_end_publish";
        case DiagnosticDisconnectReason::NotConnectedDuringDeferredFlush:
            return "deferred_not_connected";
        case DiagnosticDisconnectReason::UnderlyingPubSubStateChanged:
            return "pubsub_state";
        case DiagnosticDisconnectReason::ExplicitDisconnect:
            return "explicit";
        default:
            return "?";
    }
}

int HAMqtt::getPubSubState() const
{
    return _mqtt->state();
}

String HAMqtt::formatDirectPublishFailureDiagnostics(bool hamqttConnectedBefore, int pubsubStateBefore) const
{
    const bool hamqttConnectedAfter = isConnected();
    const int pubsubStateAfter = getPubSubState();
    String d = String(F(" hamqttConnBefore=")) + String(hamqttConnectedBefore ? 1 : 0) +
        F(" hamqttConnAfter=") + String(hamqttConnectedAfter ? 1 : 0) +
        F(" pubsubBefore=") + String(pubsubStateBefore) +
        F(" pubsubAfter=") + String(pubsubStateAfter);
    const int wifi = arduinoHANetworkStatusOptional();
    if (wifi >= 0) {
        d += F(" wifi=");
        d += String(wifi);
    }
    return d;
}

bool HAMqtt::begin(
    const IPAddress serverIp,
    const uint16_t serverPort,
    const char* username,
    const char* password
)
{
    arduinoHALogf(ArduinoHALogLevel::Info, kMqtt, F("init server "), serverIp);
    arduinoHALogf(ArduinoHALogLevel::Info, kMqtt, F("init port "), serverPort);

    if (_device.getUniqueId() == nullptr) {
        arduinoHALog(ArduinoHALogLevel::Error, kMqtt, F("init failed: missing device unique ID"));
        return false;
    }

    if (_initialized) {
        arduinoHALog(ArduinoHALogLevel::Warn, kMqtt, F("begin ignored: already initialized"));
        return false;
    }

    _username = username;
    _password = password;
    _initialized = true;

    _mqtt->setServer(serverIp, serverPort);
    _mqtt->setCallback(onMessageReceived);

    return true;
}

bool HAMqtt::begin(
    const IPAddress serverIp,
    const char* username,
    const char* password
)
{
    return begin(serverIp, HAMQTT_DEFAULT_PORT, username, password);
}

bool HAMqtt::begin(
    const char* serverHostname,
    const uint16_t serverPort,
    const char* username,
    const char* password
)
{
    arduinoHALogf(ArduinoHALogLevel::Info, kMqtt, F("init server "), serverHostname);
    arduinoHALogf(ArduinoHALogLevel::Info, kMqtt, F("init port "), serverPort);

    if (_device.getUniqueId() == nullptr) {
        arduinoHALog(ArduinoHALogLevel::Error, kMqtt, F("init failed: missing device unique ID"));
        return false;
    }

    if (_initialized) {
        arduinoHALog(ArduinoHALogLevel::Warn, kMqtt, F("begin ignored: already initialized"));
        return false;
    }

    _username = username;
    _password = password;
    _initialized = true;

    _mqtt->setServer(serverHostname, serverPort);
    _mqtt->setCallback(onMessageReceived);

    return true;
}

bool HAMqtt::begin(
    const char* serverHostname,
    const char* username,
    const char* password
)
{
    return begin(serverHostname, HAMQTT_DEFAULT_PORT, username, password);
}

bool HAMqtt::disconnect()
{
    if (!_initialized) {
        return false;
    }

    _lastDisconnectReason = DiagnosticDisconnectReason::ExplicitDisconnect;
    _lastDisconnectAt = millis();
    arduinoHALog(ArduinoHALogLevel::Info, kMqtt, F("disconnect requested"));

    clearDeferredBuilder();
    _initialized = false;
    _lastConnectionAttemptAt = 0;
    _mqtt->disconnect();
    if (_currentState != StateDisconnected) {
        setState(StateDisconnected);
    }

    return true;
}

void HAMqtt::loop()
{
    if (!_initialized) {
        return;
    }

    bool result = _mqtt->loop();
    const int rawState = _mqtt->state();

    if (result) {
        _lastLoopOkAt = millis();
    }

    if (_currentState != rawState) {
        setState(static_cast<ConnectionState>(rawState));
    }

    if (!result) {
        _lastDisconnectReason = DiagnosticDisconnectReason::LoopReturnedFalse;
        connectToServer();
    }

    if (_messageDispatchDepth == 0 && isConnected()) {
        const uint8_t before = _deferredCount;
        const bool ok = flushDeferredPublishes();
        if (!ok) {
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred flush failed queue=")) + String(_deferredCount) +
                    F(" reason=") + diagnosticDisconnectReasonText(_lastDisconnectReason)
            );
        } else if (before > 0) {
            arduinoHALog(
                ArduinoHALogLevel::Debug,
                kMqtt,
                String(F("deferred flush ok count=")) + String(before)
            );
        }
    }
}

bool HAMqtt::isConnected() const
{
    return _mqtt->connected();
}

void HAMqtt::setKeepAlive(uint16_t keepAlive)
{
    _mqtt->setKeepAlive(keepAlive);
}

bool HAMqtt::setBufferSize(uint16_t size)
{
    return _mqtt->setBufferSize(size);
}

void HAMqtt::setReconnectInterval(uint16_t interval)
{
    if (interval > 0) {
        _reconnectInterval = interval;
    }
}

bool HAMqtt::addDeviceType(HABaseDeviceType* deviceType)
{
    if (!deviceType) {
        return false;
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        if (_devicesTypes[i] == deviceType) {
            return true;
        }
    }

    if (_devicesTypesNb >= _maxDevicesTypesNb) {
        _deviceTypeRegistrationFailures++;
        arduinoHALog(
            ArduinoHALogLevel::Error,
            kMqtt,
            String(F("entity registration dropped registered=")) + String(_devicesTypesNb) +
                F(" limit=") + String(_maxDevicesTypesNb)
        );
        return false;
    }

    _devicesTypes[_devicesTypesNb++] = deviceType;
    return true;
}

bool HAMqtt::removeDeviceType(HABaseDeviceType* deviceType)
{
    if (!deviceType) {
        return false;
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        if (_devicesTypes[i] != deviceType) {
            continue;
        }

        for (uint8_t j = i + 1; j < _devicesTypesNb; j++) {
            _devicesTypes[j - 1] = _devicesTypes[j];
        }
        _devicesTypes[--_devicesTypesNb] = nullptr;
        return true;
    }

    return false;
}

bool HAMqtt::publish(const char* topic, const char* payload, bool retained)
{
    if (!isConnected() || !topic || !payload) {
        return false;
    }

    const size_t len = strlen(payload);
    if (len > UINT16_MAX) {
        return false;
    }

    const uint16_t payloadLength = static_cast<uint16_t>(len);

    arduinoHALog(
        ArduinoHALogLevel::Debug,
        kMqtt,
        String(F("publish topic=")) + topic + F(" len=") + String(payloadLength)
    );

    if (isProcessingMessage()) {
        arduinoHALog(
            ArduinoHALogLevel::Trace,
            kMqtt,
            String(F("queue publish during callback topic=")) + topic +
                F(" len=") + String(payloadLength)
        );
        return enqueueDeferredPublish(
            topic,
            reinterpret_cast<const uint8_t*>(payload),
            payloadLength,
            retained
        );
    }

    if (!beginPublish(topic, payloadLength, retained)) {
        return false;
    }

    const bool written = writePayload(
        reinterpret_cast<const uint8_t*>(payload),
        payloadLength
    );
    const bool ended = endPublish();
    return written && ended;
}

bool HAMqtt::beginPublish(
    const char* topic,
    uint16_t payloadLength,
    bool retained
)
{
    arduinoHALog(
        ArduinoHALogLevel::Debug,
        kMqtt,
        String(F("beginPublish topic=")) + (topic ? topic : "") +
            F(" len=") + String(payloadLength)
    );

    if (!isConnected() || !topic) {
        return false;
    }

    if (!isProcessingMessage()) {
        if (_directPublishActive) {
            return false;
        }

        const bool connBefore = isConnected();
        const int psBefore = getPubSubState();
        const bool ok = _mqtt->beginPublish(topic, payloadLength, retained);
        if (!ok) {
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("beginPublish failed topic=")) + topic + F(" len=") + String(payloadLength) +
                    formatDirectPublishFailureDiagnostics(connBefore, psBefore)
            );
            return false;
        }

        clearDirectPublishBuffer();
        _directPublishActive = true;
        return true;
    }

    if (_deferredBuilder.active) {
        return false;
    }

    const size_t topicLen = strlen(topic);

    _deferredBuilder.topic = new char[topicLen + 1];
    memcpy(_deferredBuilder.topic, topic, topicLen + 1);
    _deferredBuilder.payload = payloadLength > 0 ? new uint8_t[payloadLength] : nullptr;
    _deferredBuilder.expectedLength = payloadLength;
    _deferredBuilder.writtenLength = 0;
    _deferredBuilder.retained = retained;
    _deferredBuilder.active = true;
    _deferredBuilder.valid = true;
    return true;
}

bool HAMqtt::writePayload(const char* data, const uint16_t length)
{
    if (!data && length > 0) {
        return false;
    }

    return writePayload(reinterpret_cast<const uint8_t*>(data), length);
}

bool HAMqtt::writePayload(const uint8_t* data, const uint16_t length)
{
    if (!data && length > 0) {
        return false;
    }

    if (isProcessingMessage() && _deferredBuilder.active) {
        if (!_deferredBuilder.valid ||
            (static_cast<uint32_t>(_deferredBuilder.writtenLength) + length) > _deferredBuilder.expectedLength) {
            _deferredBuilder.valid = false;
            return false;
        }

        if (length > 0) {
            memcpy(
                _deferredBuilder.payload + _deferredBuilder.writtenLength,
                data,
                length
            );
        }

        _deferredBuilder.writtenLength = static_cast<uint16_t>(_deferredBuilder.writtenLength + length);
        return true;
    }

    if (_directPublishActive) {
        return appendDirectPublishPayload(data, length);
    }

    return _mqtt->write(data, length) == length;
}

bool HAMqtt::writePayload(const __FlashStringHelper* src)
{
    if (!src) {
        return false;
    }

    if (isProcessingMessage() && _deferredBuilder.active) {
        PGM_P p = reinterpret_cast<PGM_P>(src);
        const uint16_t chunkLen = static_cast<uint16_t>(strlen_P(p));
        if (!_deferredBuilder.valid ||
            (static_cast<uint32_t>(_deferredBuilder.writtenLength) + chunkLen) > _deferredBuilder.expectedLength) {
            _deferredBuilder.valid = false;
            return false;
        }

        if (chunkLen > 0) {
            memcpy_P(_deferredBuilder.payload + _deferredBuilder.writtenLength, p, chunkLen);
            _deferredBuilder.writtenLength = static_cast<uint16_t>(_deferredBuilder.writtenLength + chunkLen);
        }

        return true;
    }

    if (_directPublishActive) {
        return appendDirectPublishProgmemPayload(src);
    }

    const uint16_t length = static_cast<uint16_t>(strlen_P(reinterpret_cast<PGM_P>(src)));
    return _mqtt->print(src) == length;
}

bool HAMqtt::endPublish()
{
    if (!isProcessingMessage()) {
        if (!_directPublishActive) {
            return false;
        }

        const bool payloadFlushed = flushDirectPublishBuffer();
        const bool connBefore = isConnected();
        const int psBefore = getPubSubState();
        const bool ended = _mqtt->endPublish();
        clearDirectPublishBuffer();
        _directPublishActive = false;
        const bool ok = payloadFlushed && ended;
        if (ok) {
            _lastPublishAt = millis();
        } else {
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("endPublish failed (direct)")) +
                    formatDirectPublishFailureDiagnostics(connBefore, psBefore)
            );
        }
        return ok;
    }

    if (!_deferredBuilder.active ||
        !_deferredBuilder.valid ||
        _deferredBuilder.writtenLength != _deferredBuilder.expectedLength) {
        clearDeferredBuilder();
        return false;
    }

    const bool ok = enqueueDeferredPublish(
        _deferredBuilder.topic,
        _deferredBuilder.payload,
        _deferredBuilder.expectedLength,
        _deferredBuilder.retained
    );
    if (ok) {
        arduinoHALog(
            ArduinoHALogLevel::Trace,
            kMqtt,
            String(F("queued deferred built publish len=")) + String(_deferredBuilder.expectedLength)
        );
    }
    clearDeferredBuilder();
    return ok;
}

bool HAMqtt::flushDirectPublishBuffer()
{
    if (_directPublishBufferLength == 0) {
        return true;
    }

    const uint16_t length = _directPublishBufferLength;
    _directPublishBufferLength = 0;
    return _mqtt->write(_directPublishBuffer, length) == length;
}

bool HAMqtt::appendDirectPublishPayload(const uint8_t* data, uint16_t length)
{
    while (length > 0) {
        if (_directPublishBufferLength == DirectPublishBufferSize &&
            !flushDirectPublishBuffer()) {
            return false;
        }

        const uint16_t available =
            static_cast<uint16_t>(DirectPublishBufferSize - _directPublishBufferLength);
        const uint16_t copied = length < available ? length : available;
        memcpy(_directPublishBuffer + _directPublishBufferLength, data, copied);
        _directPublishBufferLength = static_cast<uint16_t>(_directPublishBufferLength + copied);
        data += copied;
        length = static_cast<uint16_t>(length - copied);
    }

    return true;
}

bool HAMqtt::appendDirectPublishProgmemPayload(const __FlashStringHelper* src)
{
    PGM_P data = reinterpret_cast<PGM_P>(src);
    uint16_t remaining = static_cast<uint16_t>(strlen_P(data));
    uint16_t offset = 0;
    while (remaining > 0) {
        if (_directPublishBufferLength == DirectPublishBufferSize &&
            !flushDirectPublishBuffer()) {
            return false;
        }

        const uint16_t available =
            static_cast<uint16_t>(DirectPublishBufferSize - _directPublishBufferLength);
        const uint16_t copied = remaining < available ? remaining : available;
        memcpy_P(_directPublishBuffer + _directPublishBufferLength, data + offset, copied);
        _directPublishBufferLength = static_cast<uint16_t>(_directPublishBufferLength + copied);
        offset = static_cast<uint16_t>(offset + copied);
        remaining = static_cast<uint16_t>(remaining - copied);
    }

    return true;
}

void HAMqtt::clearDirectPublishBuffer()
{
    _directPublishBufferLength = 0;
}

void HAMqtt::abortDirectPublish()
{
    clearDirectPublishBuffer();
    _directPublishActive = false;
    _mqtt->disconnect();
}

bool HAMqtt::subscribe(const char* topic)
{
    arduinoHALog(ArduinoHALogLevel::Debug, kMqtt, String(F("subscribe ")) + (topic ? topic : ""));
    return _mqtt->subscribe(topic);
}

void HAMqtt::processMessage(const char* topic, const uint8_t* payload, uint16_t length)
{
    _lastMessageAt = millis();
    arduinoHALog(
        ArduinoHALogLevel::Debug,
        kMqtt,
        String(F("rx topic=")) + (topic ? topic : "") + F(" len=") + String(length) +
            F(" depth=") + String(_messageDispatchDepth) + F(" deferred=") + String(_deferredCount)
    );

    _messageDispatchDepth++;

    if (_messageCallback) {
        _messageCallback(topic, payload, length);
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        _devicesTypes[i]->onMqttMessage(topic, payload, length);
    }

    _messageDispatchDepth--;

    if (_messageDispatchDepth == 0) {
        const uint8_t before = _deferredCount;
        const bool ok = flushDeferredPublishes();
        if (!ok) {
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred flush failed after rx queue=")) + String(_deferredCount) +
                    F(" reason=") + diagnosticDisconnectReasonText(_lastDisconnectReason)
            );
        } else if (before > 0) {
            arduinoHALog(
                ArduinoHALogLevel::Debug,
                kMqtt,
                String(F("deferred flush ok after rx count=")) + String(before)
            );
        }
    }
}

void HAMqtt::connectToServer()
{
    if (_lastConnectionAttemptAt > 0 &&
            (millis() - _lastConnectionAttemptAt) < _reconnectInterval) {
        return;
    }

    const uint32_t now = millis();
    const uint32_t sinceLastAttempt = (_lastConnectionAttemptAt > 0) ? (now - _lastConnectionAttemptAt) : 0;
    _lastConnectionAttemptAt = now;
    setState(StateConnecting);

    arduinoHALog(
        ArduinoHALogLevel::Info,
        kMqtt,
        String(F("connect attempt clientId=")) + (_device.getUniqueId() ? _device.getUniqueId() : "") +
            F(" sincePrevAttemptMs=") + String(sinceLastAttempt) +
            F(" reconnectIntervalMs=") + String(_reconnectInterval)
    );

    _mqtt->connect(
        _device.getUniqueId(),
        _username,
        _password,
        _lastWillTopic,
        0,
        _lastWillRetain,
        _lastWillMessage,
        true
    );

    if (isConnected()) {
        setState(StateConnected);
    } else {
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kMqtt,
            String(F("connect failed pubsub=")) + String(_mqtt->state())
        );
    }
}

bool HAMqtt::beginDeviceDiscoveryMigration()
{
    const char* deviceUniqueId = _device.getUniqueId();
    if (
        _deviceDiscoveryMigrationState != DeviceDiscoveryMigrationIdle ||
        !deviceUniqueId ||
        !HAJson::isValidDiscoveryTopicToken(deviceUniqueId) ||
        deviceUniqueId[0] == '\0'
    ) {
        return false;
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
        if (
            deviceType &&
            deviceType->supportsDeviceDiscovery() &&
            uniqueId &&
            HAJson::isValidDiscoveryTopicToken(uniqueId) &&
            uniqueId[0] != '\0'
        ) {
            _deviceDiscoveryEnabled = true;
            _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationMarkersPending;
            return true;
        }
    }

    return false;
}

bool HAMqtt::publishDeviceDiscoveryMigrationMarker(HABaseDeviceType* deviceType)
{
    if (
        !deviceType ||
        !deviceType->supportsDeviceDiscovery() ||
        !deviceType->uniqueId() ||
        !HAJson::isValidDiscoveryTopicToken(deviceType->uniqueId())
    ) {
        return false;
    }

    const uint16_t topicLength = HASerializer::calculateConfigTopicLength(
        deviceType->componentName(),
        deviceType->uniqueId()
    );
    if (topicLength == 0) {
        return false;
    }

    char topic[topicLength];
    if (!HASerializer::generateConfigTopic(
        topic,
        deviceType->componentName(),
        deviceType->uniqueId()
    )) {
        return false;
    }

    return publish(topic, DeviceDiscoveryMigrationPayload, true);
}

bool HAMqtt::publishDeviceDiscoveryMigrationMarker()
{
    const char* deviceUniqueId = _device.getUniqueId();
    if (
        !_discoveryPrefix ||
        !deviceUniqueId ||
        !HAJson::isValidDiscoveryTopicToken(deviceUniqueId)
    ) {
        return false;
    }

    const size_t topicLength =
        strlen(_discoveryPrefix) + 1 +
        strlen_P(HAComponentDevice) + 1 +
        strlen(deviceUniqueId) + 1 +
        strlen_P(HAConfigTopic) + 1;
    if (topicLength > UINT16_MAX) {
        return false;
    }

    char topic[topicLength];
    strcpy(topic, _discoveryPrefix);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAComponentDevice);
    strcat_P(topic, HASerializerSlash);
    strcat(topic, deviceUniqueId);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAConfigTopic);

    return publish(topic, DeviceDiscoveryMigrationPayload, true);
}

bool HAMqtt::publishDeviceDiscoveryMigrationMarkers()
{
    if (
        _deviceDiscoveryMigrationState != DeviceDiscoveryMigrationMarkersPending ||
        isProcessingMessage()
    ) {
        return false;
    }

    bool hasComponents = false;
    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
        if (
            !deviceType ||
            !deviceType->supportsDeviceDiscovery() ||
            !uniqueId ||
            !HAJson::isValidDiscoveryTopicToken(uniqueId) ||
            uniqueId[0] == '\0'
        ) {
            continue;
        }

        hasComponents = true;
        if (!publishDeviceDiscoveryMigrationMarker(deviceType)) {
            return false;
        }
    }

    if (!hasComponents) {
        return false;
    }

    _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationMarkersPublished;
    return true;
}

bool HAMqtt::publishDeviceDiscoveryMigrationConfig()
{
    if (
        _deviceDiscoveryMigrationState != DeviceDiscoveryMigrationMarkersPublished ||
        isProcessingMessage()
    ) {
        return false;
    }

    if (!publishDeviceDiscoveryPayload()) {
        return false;
    }

    _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationDevicePublished;
    return true;
}

bool HAMqtt::completeDeviceDiscoveryMigration()
{
    if (
        _deviceDiscoveryMigrationState != DeviceDiscoveryMigrationDevicePublished ||
        isProcessingMessage()
    ) {
        return false;
    }

    bool hasComponents = false;
    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
        if (
            !deviceType ||
            !deviceType->supportsDeviceDiscovery() ||
            !uniqueId ||
            !HAJson::isValidDiscoveryTopicToken(uniqueId) ||
            uniqueId[0] == '\0'
        ) {
            continue;
        }

        hasComponents = true;
        if (!deviceType->removeSingleComponentDiscovery()) {
            return false;
        }
    }

    if (!hasComponents) {
        return false;
    }

    _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationCompleted;
    return true;
}

bool HAMqtt::rollbackDeviceDiscoveryMigration()
{
    if (
        _deviceDiscoveryMigrationState == DeviceDiscoveryMigrationIdle ||
        isProcessingMessage()
    ) {
        return false;
    }

    if (_deviceDiscoveryMigrationState == DeviceDiscoveryMigrationMarkersPending) {
        _deviceDiscoveryEnabled = false;
        _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationIdle;
        return true;
    }

    const bool clearDeviceConfig =
        _deviceDiscoveryMigrationState == DeviceDiscoveryMigrationDevicePublished ||
        _deviceDiscoveryMigrationState == DeviceDiscoveryMigrationCompleted ||
        _deviceDiscoveryMigrationState == DeviceDiscoveryMigrationRollbackPending;
    if (
        _deviceDiscoveryMigrationState != DeviceDiscoveryMigrationMarkersPublished &&
        !clearDeviceConfig
    ) {
        return false;
    }

    if (clearDeviceConfig) {
        _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationRollbackPending;
    }

    if (clearDeviceConfig && !publishDeviceDiscoveryMigrationMarker()) {
        return false;
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
        if (
            !deviceType ||
            !deviceType->supportsDeviceDiscovery() ||
            !uniqueId ||
            !HAJson::isValidDiscoveryTopicToken(uniqueId) ||
            deviceType->_deviceDiscoveryRemoved
        ) {
            continue;
        }

        if (!deviceType->publishConfig()) {
            return false;
        }
    }

    if (clearDeviceConfig && !clearDeviceDiscoveryConfig()) {
        return false;
    }

    _deviceDiscoveryEnabled = false;
    _deviceDiscoveryMigrationState = DeviceDiscoveryMigrationIdle;
    return true;
}

void HAMqtt::onConnectedLogic()
{
    if (_deviceDiscoveryEnabled && !isDeviceDiscoveryMigrationInProgress()) {
        publishDeviceDiscovery();
    }

    if (!isConnected()) {
        return;
    }

    if (_connectedCallback) {
        _connectedCallback();
    }

    _device.publishAvailability();

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        _devicesTypes[i]->onMqttConnected();
    }
}

bool HAMqtt::publishDeviceDiscovery()
{
    if (isDeviceDiscoveryMigrationInProgress()) {
        return false;
    }

    return publishDeviceDiscoveryPayload();
}

HASerializer* HAMqtt::buildDeviceDiscoveryComponentSerializer(
    HABaseDeviceType* deviceType,
    HABaseDeviceType* removalType
)
{
    if (deviceType != removalType) {
        return deviceType->buildDeviceDiscoverySerializer();
    }

    HASerializer* serializer = new (std::nothrow) HASerializer(deviceType, 1);
    if (serializer) {
        serializer->set(
            AHATOFSTR(HAPlatformProperty),
            deviceType->componentName(),
            HASerializer::ProgmemPropertyValue
        );
    }
    return serializer;
}

bool HAMqtt::publishDeviceDiscoveryPayload(HABaseDeviceType* removalType)
{
    const char* deviceUniqueId = _device.getUniqueId();
    if (
        !_discoveryPrefix ||
        _discoveryPrefix[0] == '\0' ||
        !deviceUniqueId ||
        !HAJson::isValidDiscoveryTopicToken(deviceUniqueId)
    ) {
        return false;
    }

    const HASerializer* deviceSerializer = _device.getSerializer();
    const uint16_t deviceSerializerSize = deviceSerializer
        ? deviceSerializer->calculateSize()
        : 0;
    if (deviceSerializerSize == 0 || _devicesTypesNb == 0) {
        return false;
    }

    HABaseDeviceType* componentTypes[_devicesTypesNb];
    uint8_t componentSerializerCount = 0;
    uint32_t componentsPayloadLength = 2; // {}

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        if (!deviceType || !deviceType->supportsDeviceDiscovery()) {
            continue;
        }

        const char* uniqueId = deviceType->uniqueId();
        if (!uniqueId || !HAJson::isValidDiscoveryTopicToken(uniqueId)) {
            arduinoHALog(ArduinoHALogLevel::Error, kDiscovery, F("device discovery rejected invalid component ID"));
            return false;
        }

        if (deviceType->_deviceDiscoveryRemoved && deviceType != removalType) {
            continue;
        }

        HASerializer* serializer = buildDeviceDiscoveryComponentSerializer(deviceType, removalType);
        if (!serializer) {
            arduinoHALog(ArduinoHALogLevel::Error, kDiscovery, F("device discovery component serializer unavailable"));
            return false;
        }

        const uint16_t serializerSize = serializer->calculateSize();
        const uint16_t componentKeySize = HAJson::calculateEscapedStringSize(uniqueId);
        if (
            serializerSize == 0 ||
            componentKeySize == 0 ||
            componentKeySize != static_cast<uint32_t>(strlen(uniqueId)) + 2
        ) {
            delete serializer;
            arduinoHALog(ArduinoHALogLevel::Error, kDiscovery, F("device discovery component serializer invalid or too large"));
            return false;
        }

        if (componentSerializerCount > 0) {
            componentsPayloadLength += strlen_P(HASerializerJsonPropertiesSeparator);
        }
        componentsPayloadLength += componentKeySize + 1 + serializerSize;
        if (componentsPayloadLength > UINT16_MAX) {
            delete serializer;
            return false;
        }

        componentTypes[componentSerializerCount++] = deviceType;
        delete serializer;
    }

    if (componentSerializerCount == 0) {
        return false;
    }

    HASerializer originSerializer(nullptr, 3);
    originSerializer.set(AHATOFSTR(HANameProperty), DeviceDiscoveryOriginName);
    originSerializer.set(
        AHATOFSTR(HADeviceSoftwareVersionProperty),
        ARDUINOHA_LIBRARY_VERSION
    );
    if (_originSupportUrl && _originSupportUrl[0] != '\0') {
        originSerializer.set(AHATOFSTR(HAOriginSupportUrlProperty), _originSupportUrl);
    }
    const uint16_t originSerializerSize = originSerializer.calculateSize();
    if (originSerializerSize == 0) {
        return false;
    }

    const uint32_t topicLength =
        strlen(_discoveryPrefix) + 1 +
        strlen_P(HAComponentDevice) + 1 +
        strlen(deviceUniqueId) + 1 +
        strlen_P(HAConfigTopic) + 1;
    if (topicLength == 0 || topicLength > UINT16_MAX) {
        return false;
    }

    const uint32_t payloadLength =
        strlen_P(HASerializerJsonDataPrefix) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HADeviceProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        deviceSerializerSize +
        strlen_P(HASerializerJsonPropertiesSeparator) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HAOriginProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        originSerializerSize +
        strlen_P(HASerializerJsonPropertiesSeparator) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HAComponentsProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        componentsPayloadLength +
        strlen_P(HASerializerJsonDataSuffix);
    if (payloadLength > UINT16_MAX) {
        return false;
    }

    char topic[topicLength];
    strcpy(topic, _discoveryPrefix);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAComponentDevice);
    strcat_P(topic, HASerializerSlash);
    strcat(topic, deviceUniqueId);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAConfigTopic);

    const bool discConnBefore = isConnected();
    const int discPsBefore = getPubSubState();
    if (!beginPublish(topic, static_cast<uint16_t>(payloadLength), true)) {
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kDiscovery,
            String(F("device discovery beginPublish failed topic=")) + topic +
                F(" len=") + String(payloadLength) +
                formatDirectPublishFailureDiagnostics(discConnBefore, discPsBefore)
        );
        return false;
    }

    bool written =
        writePayload(AHATOFSTR(HASerializerJsonDataPrefix)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) &&
        writePayload(AHATOFSTR(HADeviceProperty)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertySuffix)) &&
        deviceSerializer->flush() &&
        writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) &&
        writePayload(AHATOFSTR(HAOriginProperty)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertySuffix)) &&
        originSerializer.flush() &&
        writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) &&
        writePayload(AHATOFSTR(HAComponentsProperty)) &&
        writePayload(AHATOFSTR(HASerializerJsonPropertySuffix)) &&
        writePayload(AHATOFSTR(HASerializerJsonDataPrefix));

    for (uint8_t i = 0; i < componentSerializerCount; i++) {
        if (written && i > 0) {
            written = writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator));
        }

        if (written) {
            HABaseDeviceType* componentType = componentTypes[i];
            HASerializer* serializer =
                buildDeviceDiscoveryComponentSerializer(componentType, removalType);
            if (!serializer) {
                written = false;
                continue;
            }

            const char* componentId = componentType->uniqueId();
            const char quote = '"';
            const char colon = ':';
            written = writePayload(&quote, 1) &&
                writePayload(componentId, strlen(componentId)) &&
                writePayload(&quote, 1) &&
                writePayload(&colon, 1) &&
                serializer->flush();
            delete serializer;
        }
    }

    if (!written) {
        abortDirectPublish();
        arduinoHALog(ArduinoHALogLevel::Warn, kDiscovery, F("device discovery stream aborted"));
        return false;
    }

    written = writePayload(AHATOFSTR(HASerializerJsonDataSuffix)) &&
        writePayload(AHATOFSTR(HASerializerJsonDataSuffix));
    if (!written) {
        abortDirectPublish();
        arduinoHALog(ArduinoHALogLevel::Warn, kDiscovery, F("device discovery stream aborted"));
        return false;
    }

    const bool published = endPublish();
    if (!published) {
        arduinoHALog(ArduinoHALogLevel::Warn, kDiscovery, F("device discovery publish failed"));
    }
    return published;
}

bool HAMqtt::clearDeviceDiscoveryConfig()
{
    const char* deviceUniqueId = _device.getUniqueId();
    if (
        !_discoveryPrefix ||
        !deviceUniqueId ||
        !HAJson::isValidDiscoveryTopicToken(deviceUniqueId)
    ) {
        return false;
    }

    const size_t topicLength =
        strlen(_discoveryPrefix) + 1 +
        strlen_P(HAComponentDevice) + 1 +
        strlen(deviceUniqueId) + 1 +
        strlen_P(HAConfigTopic) + 1;
    if (topicLength > UINT16_MAX) {
        return false;
    }

    char topic[topicLength];
    strcpy(topic, _discoveryPrefix);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAComponentDevice);
    strcat_P(topic, HASerializerSlash);
    strcat(topic, deviceUniqueId);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAConfigTopic);

    return publish(topic, "", true);
}

bool HAMqtt::removeDeviceDiscoveryComponent(HABaseDeviceType* deviceType)
{
    const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
    if (
        !_deviceDiscoveryEnabled ||
        isDeviceDiscoveryMigrationInProgress() ||
        !deviceType ||
        !deviceType->supportsDeviceDiscovery() ||
        !uniqueId ||
        !HAJson::isValidDiscoveryTopicToken(uniqueId) ||
        deviceType->_deviceDiscoveryRemoved
    ) {
        return false;
    }

    bool isRegistered = false;
    bool hasOtherComponents = false;
    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* candidate = _devicesTypes[i];
        if (candidate == deviceType) {
            isRegistered = true;
        }

        const char* candidateUniqueId = candidate ? candidate->uniqueId() : nullptr;
        if (
            candidate == deviceType ||
            !candidate ||
            !candidate->supportsDeviceDiscovery() ||
            !candidateUniqueId ||
            !HAJson::isValidDiscoveryTopicToken(candidateUniqueId) ||
            candidate->_deviceDiscoveryRemoved
        ) {
            continue;
        }

        hasOtherComponents = true;
    }

    if (!isRegistered || !publishDeviceDiscoveryPayload(deviceType)) {
        return false;
    }

    deviceType->_deviceDiscoveryRemoved = true;
    if (!hasOtherComponents) {
        return true;
    }

    return publishDeviceDiscoveryPayload();
}

bool HAMqtt::republishDeviceDiscoveryComponent(HABaseDeviceType* deviceType)
{
    const char* uniqueId = deviceType ? deviceType->uniqueId() : nullptr;
    if (
        !_deviceDiscoveryEnabled ||
        isDeviceDiscoveryMigrationInProgress() ||
        !deviceType ||
        !deviceType->supportsDeviceDiscovery() ||
        !uniqueId ||
        !HAJson::isValidDiscoveryTopicToken(uniqueId)
    ) {
        return false;
    }

    bool isRegistered = false;
    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        if (_devicesTypes[i] == deviceType) {
            isRegistered = true;
            break;
        }
    }
    if (!isRegistered) {
        return false;
    }

    const bool wasRemoved = deviceType->_deviceDiscoveryRemoved;
    deviceType->_deviceDiscoveryRemoved = false;
    if (publishDeviceDiscoveryPayload()) {
        return true;
    }

    deviceType->_deviceDiscoveryRemoved = wasRemoved;
    return false;
}

void HAMqtt::setState(ConnectionState state)
{
    ConnectionState previousState = _currentState;
    _currentState = state;

    arduinoHALog(
        ArduinoHALogLevel::Info,
        kMqtt,
        String(F("mqtt state ")) + String(static_cast<int>(previousState)) +
            F(" -> ") + String(static_cast<int>(_currentState)) +
            F(" pubsub=") + String(_mqtt->state())
    );

    if (_currentState == StateConnected) {
        _lastDisconnectReason = DiagnosticDisconnectReason::None;
        arduinoHALog(ArduinoHALogLevel::Info, kMqtt, F("connected"));
        onConnectedLogic();
    } else if (previousState == StateConnected && _currentState != StateConnected) {
        const uint32_t now = millis();
        _lastDisconnectAt = now;
        if (_lastDisconnectReason == DiagnosticDisconnectReason::None) {
            _lastDisconnectReason = DiagnosticDisconnectReason::UnderlyingPubSubStateChanged;
        }

        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kMqtt,
            String(F("disconnect ahaState=")) + String(static_cast<int>(_currentState)) +
                F(" pubsub=") + String(_mqtt->state()) +
                F(" reason=") + diagnosticDisconnectReasonText(_lastDisconnectReason) +
                F(" deferred=") + String(_deferredCount) +
                F(" depth=") + String(_messageDispatchDepth) +
                F(" sinceLastMsgMs=") +
                String(_lastMessageAt ? (now - _lastMessageAt) : static_cast<uint32_t>(0)) +
                F(" sinceLastPubMs=") +
                String(_lastPublishAt ? (now - _lastPublishAt) : static_cast<uint32_t>(0)) +
                F(" sinceLastLoopOkMs=") +
                String(_lastLoopOkAt ? (now - _lastLoopOkAt) : static_cast<uint32_t>(0))
        );

        if (_disconnectedCallback) {
            _disconnectedCallback();
        }
    }

    if (_stateChangedCallback) {
        _stateChangedCallback(_currentState);
    }
}

bool HAMqtt::enqueueDeferredPublish(
    const char* topic,
    const uint8_t* payload,
    uint16_t length,
    bool retained
)
{
    if (!topic || _deferredCount >= DeferredQueueCapacity) {
        if (topic && _deferredCount >= DeferredQueueCapacity) {
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred queue full; drop topic=")) + topic + F(" len=") + String(length)
            );
        }
        return false;
    }

    if (length > 0 && payload == nullptr) {
        return false;
    }

    const uint8_t slot = static_cast<uint8_t>((_deferredHead + _deferredCount) % DeferredQueueCapacity);
    DeferredPublishMessage& msg = _deferredQueue[slot];

    const size_t topicLen = strlen(topic);
    msg.topic = new char[topicLen + 1];
    memcpy(msg.topic, topic, topicLen + 1);

    if (length > 0) {
        msg.payload = new uint8_t[length];
        memcpy(msg.payload, payload, length);
    } else {
        msg.payload = nullptr;
    }

    msg.length = length;
    msg.retained = retained;
    _deferredCount++;

#ifdef ARDUINOHA_TEST
    _deferredPublishEnqueueCountForTest++;
#endif

    return true;
}

void HAMqtt::clearDeferredMessage(DeferredPublishMessage& msg)
{
    delete[] msg.topic;
    delete[] msg.payload;
    msg.topic = nullptr;
    msg.payload = nullptr;
    msg.length = 0;
    msg.retained = false;
}

void HAMqtt::clearDeferredQueue()
{
    while (_deferredCount > 0) {
        DeferredPublishMessage& msg = _deferredQueue[_deferredHead];
        clearDeferredMessage(msg);
        _deferredHead = static_cast<uint8_t>((_deferredHead + 1) % DeferredQueueCapacity);
        _deferredCount--;
    }

    _deferredHead = 0;
}

void HAMqtt::clearDeferredBuilder()
{
    delete[] _deferredBuilder.topic;
    delete[] _deferredBuilder.payload;
    _deferredBuilder.topic = nullptr;
    _deferredBuilder.payload = nullptr;
    _deferredBuilder.expectedLength = 0;
    _deferredBuilder.writtenLength = 0;
    _deferredBuilder.retained = false;
    _deferredBuilder.active = false;
    _deferredBuilder.valid = false;
}

bool HAMqtt::flushDeferredPublishes()
{
    while (_deferredCount > 0) {
        DeferredPublishMessage& msg = _deferredQueue[_deferredHead];

        const bool defConnSnap = isConnected();
        const int defPsSnap = getPubSubState();
        if (!isConnected()) {
            _lastDisconnectReason = DiagnosticDisconnectReason::NotConnectedDuringDeferredFlush;
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred flush: not connected topic=")) +
                    (msg.topic ? msg.topic : "") + F(" queue=") + String(_deferredCount) +
                    formatDirectPublishFailureDiagnostics(defConnSnap, defPsSnap)
            );
#ifdef ARDUINOHA_TEST
            _deferredFlushFailedForTest = true;
            _lastDeferredFlushErrorForTest = DeferredFlushErrorNotConnected;
#endif
            return false;
        }

        const bool defBpConnBefore = isConnected();
        const int defBpPsBefore = getPubSubState();
        if (!_mqtt->beginPublish(msg.topic, msg.length, msg.retained)) {
            _lastDisconnectReason = DiagnosticDisconnectReason::DeferredBeginPublishFailed;
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred beginPublish failed topic=")) +
                    (msg.topic ? msg.topic : "") + F(" len=") + String(msg.length) +
                    formatDirectPublishFailureDiagnostics(defBpConnBefore, defBpPsBefore)
            );
#ifdef ARDUINOHA_TEST
            _deferredFlushFailedForTest = true;
            _lastDeferredFlushErrorForTest = DeferredFlushErrorBeginPublish;
#endif
            return false;
        }

        if (msg.length > 0 && msg.payload != nullptr) {
            _mqtt->write(msg.payload, msg.length);
        }

        const bool defEpConnBefore = isConnected();
        const int defEpPsBefore = getPubSubState();
        if (!_mqtt->endPublish()) {
            _lastDisconnectReason = DiagnosticDisconnectReason::DeferredEndPublishFailed;
            arduinoHALog(
                ArduinoHALogLevel::Warn,
                kMqtt,
                String(F("deferred endPublish failed topic=")) + (msg.topic ? msg.topic : "") +
                    formatDirectPublishFailureDiagnostics(defEpConnBefore, defEpPsBefore)
            );
#ifdef ARDUINOHA_TEST
            _deferredFlushFailedForTest = true;
            _lastDeferredFlushErrorForTest = DeferredFlushErrorEndPublish;
#endif
            return false;
        }

        _lastPublishAt = millis();
        clearDeferredMessage(msg);
        _deferredHead = static_cast<uint8_t>((_deferredHead + 1) % DeferredQueueCapacity);
        _deferredCount--;
    }

#ifdef ARDUINOHA_TEST
    _deferredFlushFailedForTest = false;
    _lastDeferredFlushErrorForTest = DeferredFlushErrorNone;
#endif
    return true;
}