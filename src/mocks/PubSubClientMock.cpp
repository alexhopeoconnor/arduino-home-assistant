#include "PubSubClientMock.h"
#ifdef ARDUINOHA_TEST

#include "../ArduinoHADefines.h"
#include <stdlib.h>

PubSubClientMock::PubSubClientMock() :
    _pendingMessage(nullptr),
    _flushedMessages(nullptr),
    _keepAlive(15),
    _bufferSize(256),
    _state(-1),
    _connectCallsNb(0),
    _flushedMessagesNb(0),
    _subscriptions(nullptr),
    _subscriptionsNb(0),
    _insideCallback(false),
    _failNextBeginPublish(false),
    _failNextEndPublish(false),
    callback(nullptr)
{

}

PubSubClientMock::~PubSubClientMock()
{
    if (_pendingMessage) {
        delete _pendingMessage;
    }

    clearFlushedMessages();
    clearSubscriptions();
}

bool PubSubClientMock::loop()
{
    return connected();
}

void PubSubClientMock::disconnect()
{
    _connection.connected = false;
}

bool PubSubClientMock::connected()
{
    return _connection.connected;
}

bool PubSubClientMock::connect(
    const char *id,
    const char *user,
    const char *pass,
    const char* willTopic,
    uint8_t willQos,
    bool willRetain,
    const char* willMessage,
    bool cleanSession
)
{
    (void)willQos;
    (void)cleanSession;

    _connectCallsNb++;
    _connection.connected = true;
    _connection.id = id;
    _connection.user = user;
    _connection.pass = pass;

    _lastWill.topic = willTopic;
    _lastWill.message = willMessage;
    _lastWill.retain = willRetain;

    return true;
}

bool PubSubClientMock::connectDummy()
{
    _connection.connected = true;
    _connection.id = "dummyId";
    _connection.user = nullptr;
    _connection.pass = nullptr;

    _lastWill.topic = nullptr;
    _lastWill.message = nullptr;
    _lastWill.retain = false;

    return true;
}

PubSubClientMock& PubSubClientMock::setServer(IPAddress ip, uint16_t port)
{
    _connection.ip = ip;
    _connection.port = port;

    return *this;
}

PubSubClientMock& PubSubClientMock::setServer(
    const char * domain,
    uint16_t port
)
{
    _connection.domain = domain;
    _connection.port = port;

    return *this;
}

PubSubClientMock& PubSubClientMock::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    this->callback = callback;
    return *this;
}

bool PubSubClientMock::beginPublish(
    const char* topic,
    unsigned int plength,
    bool retained
)
{
    if (!connected()) {
        return false;
    }

    if (_failNextBeginPublish) {
        _failNextBeginPublish = false;
        return false;
    }

    if (_pendingMessage) {
        delete _pendingMessage;
    }

    _pendingMessage = new MqttMessage();
    _pendingMessage->retained = retained;

    {
        size_t size = strlen(topic) + 1;
        _pendingMessage->topic = new char[size];
        _pendingMessage->topicSize = size;

        memset(_pendingMessage->topic, 0, size);
        memcpy(_pendingMessage->topic, topic, size);
    }

    {
        size_t size = plength + 1;
        _pendingMessage->buffer = new char[size];
        _pendingMessage->bufferSize = size;

        memset(_pendingMessage->buffer, 0, size);
    }

    return true;
}

size_t PubSubClientMock::write(const uint8_t *buffer, size_t size)
{
    if (!_pendingMessage || !_pendingMessage->buffer || !buffer) {
        return 0;
    }

    const size_t capacity = _pendingMessage->bufferSize - 1;
    if (_pendingMessage->writtenSize >= capacity) {
        return 0;
    }

    const size_t available = capacity - _pendingMessage->writtenSize;
    const size_t written = size < available ? size : available;
    if (written == 0) {
        return 0;
    }

    memcpy(_pendingMessage->buffer + _pendingMessage->writtenSize, buffer, written);
    _pendingMessage->writtenSize += written;
    _pendingMessage->buffer[_pendingMessage->writtenSize] = 0;
    return written;
}

size_t PubSubClientMock::print(const __FlashStringHelper* buffer)
{
    const size_t len = strlen_P(reinterpret_cast<const char*>(buffer));
    char data[len + 1]; // including null terminator
    strcpy_P(data, reinterpret_cast<const char*>(buffer));

    return write((const uint8_t*)(data), len);
}

int PubSubClientMock::endPublish()
{
    if (!_pendingMessage) {
        return 0;
    }

    if (_failNextEndPublish) {
        _failNextEndPublish = false;
        return 0;
    }

    if (_pendingMessage->writtenSize != _pendingMessage->bufferSize - 1 ||
        _flushedMessagesNb == UINT8_MAX) {
        return 0;
    }

    MqttMessage** expanded = static_cast<MqttMessage**>(
        realloc(_flushedMessages, (_flushedMessagesNb + 1) * sizeof(MqttMessage*))
    );
    if (!expanded) {
        return 0;
    }

    _flushedMessages = expanded;
    _flushedMessages[_flushedMessagesNb++] = _pendingMessage;

    _pendingMessage = nullptr; // do not call destructor

    return _flushedMessages[_flushedMessagesNb - 1]->bufferSize;
}

bool PubSubClientMock::subscribe(const char* topic)
{
    if (!topic || _subscriptionsNb == UINT8_MAX) {
        return false;
    }

    MqttSubscription** expanded = static_cast<MqttSubscription**>(
        realloc(_subscriptions, (_subscriptionsNb + 1) * sizeof(MqttSubscription*))
    );
    if (!expanded) {
        return false;
    }

    size_t topicSize = strlen(topic) + 1;
    MqttSubscription* subscription = new MqttSubscription();
    subscription->topic = new char[topicSize];
    memcpy(subscription->topic, topic, topicSize);

    _subscriptions = expanded;
    _subscriptions[_subscriptionsNb++] = subscription;
    return true;
}

void PubSubClientMock::clearFlushedMessages()
{
    if (_flushedMessages) {
        for (uint8_t i = 0; i < _flushedMessagesNb; i++) {
            delete _flushedMessages[i];
        }

        free(_flushedMessages);
        _flushedMessages = nullptr;
    }

    _flushedMessagesNb = 0;
}

void PubSubClientMock::clearSubscriptions()
{
    if (_subscriptions) {
        for (uint8_t i = 0; i < _subscriptionsNb; i++) {
            delete _subscriptions[i];
        }

        free(_subscriptions);
        _subscriptions = nullptr;
    }

    _subscriptionsNb = 0;
}

void PubSubClientMock::fakeMessage(const char* topic, const char* message)
{
    if (!callback) {
        return;
    }

    uint16_t len = strlen(message);
    uint8_t data[len];
    memcpy(data, message, len);

    _insideCallback = true;
    callback(const_cast<char*>(topic), data, len);
    _insideCallback = false;
}

void PubSubClientMock::fakeMessage(
    const __FlashStringHelper* topic,
    const char* message
)
{
    char topicStr[strlen_P(AHAFROMFSTR(topic)) + 1];
    topicStr[0] = 0;
    strcpy_P(topicStr, AHAFROMFSTR(topic));

    fakeMessage(topicStr, message);
}

void PubSubClientMock::fakeMessage(
    const __FlashStringHelper* topic,
    const __FlashStringHelper* message
)
{
    char topicStr[strlen_P(AHAFROMFSTR(topic)) + 1];
    topicStr[0] = 0;
    strcpy_P(topicStr, AHAFROMFSTR(topic));

    char messageStr[strlen_P(AHAFROMFSTR(message)) + 1];
    messageStr[0] = 0;
    strcpy_P(messageStr, AHAFROMFSTR(message));

    fakeMessage(topicStr, messageStr);
}

#endif
