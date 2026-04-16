#include "HAMqtt.h"

#include <cstdio>

#ifndef ARDUINOHA_TEST
#include <PubSubClient.h>
#endif

#include "HADevice.h"
#include "device-types/HABaseDeviceType.h"
#include "mocks/PubSubClientMock.h"
#include "utils/HADictionary.h"
#include "utils/HASerializer.h"

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
    _currentState(StateDisconnected)

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
    delete[] _devicesTypes;

#ifdef ARDUINOHA_TEST
    if (_mqtt) {
        delete _mqtt;
    }
#endif

    _instance = nullptr;
}

bool HAMqtt::begin(
    const IPAddress serverIp,
    const uint16_t serverPort,
    const char* username,
    const char* password
)
{
    ARDUINOHA_DEBUG_PRINT(F("AHA: init server "))
    ARDUINOHA_DEBUG_PRINT(serverIp)
    ARDUINOHA_DEBUG_PRINT(F(":"))
    ARDUINOHA_DEBUG_PRINTLN(serverPort)

    if (_device.getUniqueId() == nullptr) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: init failed. Missing device unique ID"))
        return false;
    }

    if (_initialized) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: already initialized"))
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
    ARDUINOHA_DEBUG_PRINT(F("AHA: init server "))
    ARDUINOHA_DEBUG_PRINT(serverHostname)
    ARDUINOHA_DEBUG_PRINT(F(":"))
    ARDUINOHA_DEBUG_PRINTLN(serverPort)

    if (_device.getUniqueId() == nullptr) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: init failed. Missing device unique ID"))
        return false;
    }

    if (_initialized) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: already initialized"))
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

    ARDUINOHA_DEBUG_PRINTLN(F("AHA: disconnecting"))

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
    if (_currentState != _mqtt->state()) {
        setState(static_cast<ConnectionState>(_mqtt->state()));
    }

    if (!result) {
        connectToServer();
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
    if (!isConnected()) {
        return false;
    }

    ARDUINOHA_DEBUG_PRINT(F("AHA: publishing "))
    ARDUINOHA_DEBUG_PRINT(topic)
    ARDUINOHA_DEBUG_PRINT(F(", len: "))
    ARDUINOHA_DEBUG_PRINTLN(strlen(payload))

    _mqtt->beginPublish(topic, strlen(payload), retained);
    _mqtt->write((const uint8_t*)(payload), strlen(payload));
    return _mqtt->endPublish();
}

bool HAMqtt::beginPublish(
    const char* topic,
    uint16_t payloadLength,
    bool retained
)
{
    ARDUINOHA_DEBUG_PRINT(F("AHA: begin publish "))
    ARDUINOHA_DEBUG_PRINT(topic)
    ARDUINOHA_DEBUG_PRINT(F(", len: "))
    ARDUINOHA_DEBUG_PRINTLN(payloadLength)

    return _mqtt->beginPublish(topic, payloadLength, retained);
}

void HAMqtt::writePayload(const char* data, const uint16_t length)
{
    writePayload(reinterpret_cast<const uint8_t*>(data), length);
}

void HAMqtt::writePayload(const uint8_t* data, const uint16_t length)
{
    _mqtt->write(data, length);
}

void HAMqtt::writePayload(const __FlashStringHelper* src)
{
    _mqtt->print(src);
}

bool HAMqtt::endPublish()
{
    return _mqtt->endPublish();
}

bool HAMqtt::subscribe(const char* topic)
{
    ARDUINOHA_DEBUG_PRINT(F("AHA: subscribing "))
    ARDUINOHA_DEBUG_PRINTLN(topic)

    return _mqtt->subscribe(topic);
}

void HAMqtt::processMessage(const char* topic, const uint8_t* payload, uint16_t length)
{
    ARDUINOHA_DEBUG_PRINT(F("AHA: received call "))
    ARDUINOHA_DEBUG_PRINT(topic)
    ARDUINOHA_DEBUG_PRINT(F(", len: "))
    ARDUINOHA_DEBUG_PRINTLN(length)

    if (_messageCallback) {
        _messageCallback(topic, payload, length);
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        _devicesTypes[i]->onMqttMessage(topic, payload, length);
    }
}

void HAMqtt::connectToServer()
{
    if (_lastConnectionAttemptAt > 0 &&
            (millis() - _lastConnectionAttemptAt) < _reconnectInterval) {
        return;
    }

    _lastConnectionAttemptAt = millis();
    setState(StateConnecting);

    ARDUINOHA_DEBUG_PRINT(F("AHA: MQTT connecting, client ID: "))
    ARDUINOHA_DEBUG_PRINTLN(_device.getUniqueId())

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
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: failed to connect"))
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

    char originPayload[64];
    originPayload[0] = 0;
    snprintf(
        originPayload,
        sizeof(originPayload),
        "{\"name\":\"%s\",\"sw\":\"%s\"}",
        DeviceDiscoveryOriginName,
        ARDUINOHA_LIBRARY_VERSION
    );
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

    if (!_mqtt->beginPublish(topic, payloadLength, true)) {
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
    return endPublish();
}

void HAMqtt::setState(ConnectionState state)
{
    ConnectionState previousState = _currentState;
    _currentState = state;

    ARDUINOHA_DEBUG_PRINT(F("AHA: MQTT state changed to "))
    ARDUINOHA_DEBUG_PRINT(_currentState)
    ARDUINOHA_DEBUG_PRINT(F(", previous state: "))
    ARDUINOHA_DEBUG_PRINTLN(previousState)

    if (_currentState == StateConnected) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: MQTT connected"))
        onConnectedLogic();
    } else if (previousState == StateConnected && _currentState != StateConnected) {
        ARDUINOHA_DEBUG_PRINTLN(F("AHA: MQTT disconnected"))

        if (_disconnectedCallback) {
            _disconnectedCallback();
        }
    }

    if (_stateChangedCallback) {
        _stateChangedCallback(_currentState);
    }
}