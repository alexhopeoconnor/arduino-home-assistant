#include "HAMqtt.h"

#include <cstdio>
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
    _originSupportUrl(nullptr), \
    _username(nullptr), \
    _password(nullptr), \
    _lastConnectionAttemptAt(0), \
    _reconnectInterval(DefaultReconnectInterval), \
    _devicesTypesNb(0), \
    _maxDevicesTypesNb(maxDevicesTypesNb), \
    _devicesTypes(new HABaseDeviceType*[maxDevicesTypesNb]), \
    _lastWillTopic(nullptr), \
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
    HAMQTT_INIT
{
    _instance = this;
}
#else
HAMqtt::HAMqtt(
    Client& netClient,
    HADevice& device,
    uint8_t maxDevicesTypesNb
) :
    _mqttStorage(netClient),
    _mqtt(&_mqttStorage),
    HAMQTT_INIT
{
    _instance = this;
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
        const uint32_t now = millis();
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kMqtt,
            String(F("loop returned false pubsub=")) + String(rawState) +
                F(" deferred=") + String(_deferredCount) +
                F(" depth=") + String(_messageDispatchDepth) +
                F(" sinceLastMsgMs=") +
                String(_lastMessageAt ? (now - _lastMessageAt) : static_cast<uint32_t>(0)) +
                F(" sinceLastPubMs=") +
                String(_lastPublishAt ? (now - _lastPublishAt) : static_cast<uint32_t>(0))
        );
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

void HAMqtt::addDeviceType(HABaseDeviceType* deviceType)
{
    if (_devicesTypesNb + 1 > _maxDevicesTypesNb) {
        return;
    }

    _devicesTypes[_devicesTypesNb++] = deviceType;
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

    const bool connBeforePub = isConnected();
    const int psBeforePub = getPubSubState();
    if (!_mqtt->beginPublish(topic, payloadLength, retained)) {
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kMqtt,
            String(F("beginPublish failed topic=")) + topic + F(" len=") + String(payloadLength) +
                formatDirectPublishFailureDiagnostics(connBeforePub, psBeforePub)
        );
        return false;
    }
    _mqtt->write(reinterpret_cast<const uint8_t*>(payload), payloadLength);
    const bool connBeforeEnd = isConnected();
    const int psBeforeEnd = getPubSubState();
    const bool ok = _mqtt->endPublish();
    if (!ok) {
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kMqtt,
            String(F("endPublish failed topic=")) + topic +
                formatDirectPublishFailureDiagnostics(connBeforeEnd, psBeforeEnd)
        );
    } else {
        _lastPublishAt = millis();
    }
    return ok;
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
        }
        return ok;
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

void HAMqtt::writePayload(const char* data, const uint16_t length)
{
    writePayload(reinterpret_cast<const uint8_t*>(data), length);
}

void HAMqtt::writePayload(const uint8_t* data, const uint16_t length)
{
    if (isProcessingMessage() && _deferredBuilder.active) {
        if (!_deferredBuilder.valid ||
            (static_cast<uint32_t>(_deferredBuilder.writtenLength) + length) > _deferredBuilder.expectedLength) {
            _deferredBuilder.valid = false;
            return;
        }

        if (length > 0) {
            memcpy(
                _deferredBuilder.payload + _deferredBuilder.writtenLength,
                data,
                length
            );
        }

        _deferredBuilder.writtenLength = static_cast<uint16_t>(_deferredBuilder.writtenLength + length);
        return;
    }

    _mqtt->write(data, length);
}

void HAMqtt::writePayload(const __FlashStringHelper* src)
{
    if (isProcessingMessage() && _deferredBuilder.active) {
        PGM_P p = reinterpret_cast<PGM_P>(src);
        const uint16_t chunkLen = static_cast<uint16_t>(strlen_P(p));
        if (!_deferredBuilder.valid ||
            (static_cast<uint32_t>(_deferredBuilder.writtenLength) + chunkLen) > _deferredBuilder.expectedLength) {
            _deferredBuilder.valid = false;
            return;
        }

        if (chunkLen > 0) {
            memcpy_P(_deferredBuilder.payload + _deferredBuilder.writtenLength, p, chunkLen);
            _deferredBuilder.writtenLength = static_cast<uint16_t>(_deferredBuilder.writtenLength + chunkLen);
        }

        return;
    }

    _mqtt->print(src);
}

bool HAMqtt::endPublish()
{
    if (!isProcessingMessage()) {
        const bool connBefore = isConnected();
        const int psBefore = getPubSubState();
        const bool ok = _mqtt->endPublish();
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

void HAMqtt::onConnectedLogic()
{
    if (_deviceDiscoveryEnabled) {
        publishDeviceDiscovery();
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
    if (!_device.getUniqueId()) {
        return false;
    }

    const HASerializer* deviceSerializer = _device.getSerializer();
    if (!deviceSerializer) {
        return false;
    }

    HABaseDeviceType* componentTypes[_devicesTypesNb];
    HASerializer* componentSerializers[_devicesTypesNb];
    uint8_t componentSerializerCount = 0;
    uint16_t componentsPayloadLength = 2; // {}

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        HABaseDeviceType* deviceType = _devicesTypes[i];
        if (!deviceType || !deviceType->supportsDeviceDiscovery()) {
            continue;
        }

        HASerializer* serializer = deviceType->buildDeviceDiscoverySerializer();
        if (!serializer) {
            continue;
        }

        if (componentSerializerCount > 0) {
            componentsPayloadLength += strlen_P(HASerializerJsonPropertiesSeparator);
        }

        componentsPayloadLength +=
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen(deviceType->uniqueId()) +
            strlen_P(HASerializerJsonPropertySuffix) +
            serializer->calculateSize();

        componentTypes[componentSerializerCount] = deviceType;
        componentSerializers[componentSerializerCount++] = serializer;
    }

    if (componentSerializerCount == 0) {
        return false;
    }

    char originPayload[192];
    originPayload[0] = 0;
    if (_originSupportUrl && _originSupportUrl[0] != '\0') {
        snprintf(
            originPayload,
            sizeof(originPayload),
            "{\"name\":\"%s\",\"sw\":\"%s\",\"url\":\"%s\"}",
            DeviceDiscoveryOriginName,
            ARDUINOHA_LIBRARY_VERSION,
            _originSupportUrl
        );
    } else {
        snprintf(
            originPayload,
            sizeof(originPayload),
            "{\"name\":\"%s\",\"sw\":\"%s\"}",
            DeviceDiscoveryOriginName,
            ARDUINOHA_LIBRARY_VERSION
        );
    }
    const uint16_t originPayloadLength = strlen(originPayload);

    const uint16_t topicLength =
        strlen(_discoveryPrefix) + 1 +
        strlen_P(HAComponentDevice) + 1 +
        strlen(_device.getUniqueId()) + 1 +
        strlen_P(HAConfigTopic) + 1;
    if (topicLength == 0) {
        for (uint8_t i = 0; i < componentSerializerCount; i++) {
            delete componentSerializers[i];
        }

        return false;
    }

    const uint16_t payloadLength =
        strlen_P(HASerializerJsonDataPrefix) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HADeviceProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        deviceSerializer->calculateSize() +
        strlen_P(HASerializerJsonPropertiesSeparator) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HAOriginProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        originPayloadLength +
        strlen_P(HASerializerJsonPropertiesSeparator) +
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(HAComponentsProperty) +
        strlen_P(HASerializerJsonPropertySuffix) +
        componentsPayloadLength +
        strlen_P(HASerializerJsonDataSuffix);

    char topic[topicLength];
    strcpy(topic, _discoveryPrefix);
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAComponentDevice);
    strcat_P(topic, HASerializerSlash);
    strcat(topic, _device.getUniqueId());
    strcat_P(topic, HASerializerSlash);
    strcat_P(topic, HAConfigTopic);

    const bool discConnBefore = isConnected();
    const int discPsBefore = getPubSubState();
    if (!_mqtt->beginPublish(topic, payloadLength, true)) {
        arduinoHALog(
            ArduinoHALogLevel::Warn,
            kDiscovery,
            String(F("device discovery beginPublish failed topic=")) + topic +
                F(" len=") + String(payloadLength) +
                formatDirectPublishFailureDiagnostics(discConnBefore, discPsBefore)
        );
        for (uint8_t i = 0; i < componentSerializerCount; i++) {
            delete componentSerializers[i];
        }

        return false;
    }

    writePayload(AHATOFSTR(HASerializerJsonDataPrefix));

    writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix));
    writePayload(AHATOFSTR(HADeviceProperty));
    writePayload(AHATOFSTR(HASerializerJsonPropertySuffix));
    deviceSerializer->flush();

    writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator));
    writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix));
    writePayload(AHATOFSTR(HAOriginProperty));
    writePayload(AHATOFSTR(HASerializerJsonPropertySuffix));
    writePayload(originPayload, originPayloadLength);

    writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator));
    writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix));
    writePayload(AHATOFSTR(HAComponentsProperty));
    writePayload(AHATOFSTR(HASerializerJsonPropertySuffix));
    writePayload(AHATOFSTR(HASerializerJsonDataPrefix));

    for (uint8_t i = 0; i < componentSerializerCount; i++) {
        if (i > 0) {
            writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator));
        }

        writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix));
        writePayload(componentTypes[i]->uniqueId(), strlen(componentTypes[i]->uniqueId()));
        writePayload(AHATOFSTR(HASerializerJsonPropertySuffix));
        componentSerializers[i]->flush();
        delete componentSerializers[i];
    }

    writePayload(AHATOFSTR(HASerializerJsonDataSuffix));
    writePayload(AHATOFSTR(HASerializerJsonDataSuffix));
    const bool published = endPublish();
    if (!published) {
        arduinoHALog(ArduinoHALogLevel::Warn, kDiscovery, F("device discovery endPublish failed"));
    }
    return published;
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