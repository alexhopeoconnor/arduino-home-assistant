#include "HABaseDeviceType.h"
#include "../HAMqtt.h"
#include "../HADevice.h"
#include "../utils/HAUtils.h"
#include "../utils/HASerializer.h"

HABaseDeviceType::HABaseDeviceType(
    const __FlashStringHelper* componentName,
    const char* uniqueId
) :
    _componentName(componentName),
    _uniqueId(uniqueId),
    _name(nullptr),
    _objectId(nullptr),
    _defaultEntityId(nullptr),
    _entityCategory(nullptr),
    _serializer(nullptr),
    _availability(AvailabilityDefault)
{
    if (mqtt()) {
        mqtt()->addDeviceType(this);
    }
}

HABaseDeviceType::~HABaseDeviceType()
{
    destroySerializer();
}

void HABaseDeviceType::setAvailability(bool online)
{
    _availability = (online ? AvailabilityOnline : AvailabilityOffline);
    publishAvailability();
}

bool HABaseDeviceType::removeFromDiscovery()
{
    const uint16_t topicLength = HASerializer::calculateConfigTopicLength(
        componentName(),
        uniqueId()
    );
    if (topicLength == 0) {
        return false;
    }

    char topic[topicLength];
    if (!HASerializer::generateConfigTopic(topic, componentName(), uniqueId())) {
        return false;
    }

    destroySerializer();
    if (!mqtt()->beginPublish(topic, 0, true)) {
        return false;
    }

    return mqtt()->endPublish();
}

bool HABaseDeviceType::republishDiscovery()
{
    HAMqtt* mqttInstance = mqtt();
    if (!mqttInstance) {
        return false;
    }

    if (shouldPublishSingleComponentConfig()) {
        return publishConfig();
    }

    // Clear any stale per-entity retained config so device discovery remains
    // the single source of truth for supported entities.
    removeFromDiscovery();
    return mqttInstance->publishDeviceDiscovery();
}

HAMqtt* HABaseDeviceType::mqtt()
{
    return HAMqtt::instance();
}

void HABaseDeviceType::subscribeTopic(
    const char* uniqueId,
    const __FlashStringHelper* topic
)
{
    const uint16_t topicLength = HASerializer::calculateDataTopicLength(
        uniqueId,
        topic
    );
    if (topicLength == 0) {
        return;
    }

    char fullTopic[topicLength];
    if (!HASerializer::generateDataTopic(
        fullTopic,
        uniqueId,
        topic
    )) {
        return;
    }

    HAMqtt::instance()->subscribe(fullTopic);
}

void HABaseDeviceType::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    (void)topic;
    (void)payload;
    (void)length;
}

void HABaseDeviceType::destroySerializer()
{
    if (_serializer) {
        delete _serializer;
        _serializer = nullptr;
    }
}

bool HABaseDeviceType::publishConfig()
{
    buildSerializer();

    if (_serializer == nullptr) {
        return false;
    }

    const uint16_t topicLength = HASerializer::calculateConfigTopicLength(
        componentName(),
        uniqueId()
    );
    const uint16_t dataLength = _serializer->calculateSize();

    bool published = false;
    if (topicLength > 0 && dataLength > 0) {
        char topic[topicLength];
        HASerializer::generateConfigTopic(
            topic,
            componentName(),
            uniqueId()
        );

        if (mqtt()->beginPublish(topic, dataLength, true)) {
            _serializer->flush();
            published = mqtt()->endPublish();
        }
    }

    destroySerializer();
    return published;
}

void HABaseDeviceType::publishAvailability()
{
    const HADevice* device = mqtt()->getDevice();
    if (
        !device ||
        device->isSharedAvailabilityEnabled() ||
        !isAvailabilityConfigured()
    ) {
        return;
    }

    publishOnDataTopic(
        AHATOFSTR(HAAvailabilityTopic),
        _availability == AvailabilityOnline
            ? AHATOFSTR(HAOnline)
            : AHATOFSTR(HAOffline),
        true
    );
}

bool HABaseDeviceType::publishOnDataTopic(
    const __FlashStringHelper* topic,
    const __FlashStringHelper* payload,
    bool retained
)
{
    if (!payload) {
        return false;
    }

    return publishOnDataTopic(
        topic,
        reinterpret_cast<const uint8_t*>(payload),
        strlen_P(AHAFROMFSTR(payload)),
        retained,
        true
    );
}

bool HABaseDeviceType::publishOnDataTopic(
    const __FlashStringHelper* topic,
    const char* payload,
    bool retained
)
{
    if (!payload) {
        return false;
    }

    return publishOnDataTopic(
        topic,
        reinterpret_cast<const uint8_t*>(payload),
        strlen(payload),
        retained
    );
}

bool HABaseDeviceType::publishOnDataTopic(
    const __FlashStringHelper* topic,
    const uint8_t* payload,
    const uint16_t length,
    bool retained,
    bool isProgmemData
)
{
    if (!payload) {
        return false;
    }

    const uint16_t topicLength = HASerializer::calculateDataTopicLength(
        uniqueId(),
        topic
    );
    if (topicLength == 0) {
        return false;
    }

    char fullTopic[topicLength];
    if (!HASerializer::generateDataTopic(
        fullTopic,
        uniqueId(),
        topic
    )) {
        return false;
    }

    if (mqtt()->beginPublish(fullTopic, length, retained)) {
        if (isProgmemData) {
            mqtt()->writePayload(AHATOFSTR(payload));
        } else {
            mqtt()->writePayload(payload, length);
        }

        return mqtt()->endPublish();
    }

    return false;
}

bool HABaseDeviceType::shouldPublishSingleComponentConfig() const
{
    return !mqtt() || !mqtt()->isDeviceDiscoveryEnabled() || !supportsDeviceDiscovery();
}

void HABaseDeviceType::setEntityIdProperty(HASerializer* serializer) const
{
    if (!serializer) {
        return;
    }

    const char* defaultEntityId = nonEmptyString(_defaultEntityId);
    if (defaultEntityId) {
        serializer->set(AHATOFSTR(HADefaultEntityIdProperty), defaultEntityId);
        return;
    }

    const char* objectId = nonEmptyString(_objectId);
    if (objectId) {
        serializer->set(AHATOFSTR(HAObjectIdProperty), objectId);
    }
}

const char* HABaseDeviceType::nonEmptyString(const char* value)
{
    return (value && value[0] != '\0') ? value : nullptr;
}

HASerializer* HABaseDeviceType::buildDeviceDiscoverySerializer()
{
    return nullptr;
}

bool HABaseDeviceType::supportsDeviceDiscovery() const
{
    return false;
}