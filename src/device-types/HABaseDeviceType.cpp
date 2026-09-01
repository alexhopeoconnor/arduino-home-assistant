#include "HABaseDeviceType.h"
#include "../HAMqtt.h"
#include "../HADevice.h"
#include "../utils/HAUtils.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"
#include <string.h>

HABaseDeviceType* HABaseDeviceType::_firstInstance = nullptr;

void HABaseDeviceType::registerAllWith(HAMqtt& mqttInstance)
{
    for (HABaseDeviceType* entity = _firstInstance; entity; entity = entity->_nextInstance) {
        mqttInstance.addDeviceType(entity);
    }
}

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
    _hasEnabledByDefault(false),
    _enabledByDefault(true),
    _entityPicture(nullptr),
    _hasQos(false),
    _qosNumeric(),
    _encoding(nullptr),
    _payloadAvailable(nullptr),
    _payloadNotAvailable(nullptr),
    _availabilityMode(nullptr),
    _availabilityList(),
    _deviceDiscoveryRemoved(false),
    _availability(AvailabilityDefault),
    _nextInstance(_firstInstance)
{
    _firstInstance = this;
    if (HAMqtt* mqttInstance = mqtt()) {
        mqttInstance->addDeviceType(this);
    }
}

HABaseDeviceType::~HABaseDeviceType()
{
    if (_firstInstance == this) {
        _firstInstance = _nextInstance;
    } else {
        HABaseDeviceType* previous = _firstInstance;
        while (previous && previous->_nextInstance != this) {
            previous = previous->_nextInstance;
        }
        if (previous) {
            previous->_nextInstance = _nextInstance;
        }
    }

    if (HAMqtt* mqttInstance = mqtt()) {
        mqttInstance->removeDeviceType(this);
    }

    destroySerializer();
}

void HABaseDeviceType::setAvailability(bool online)
{
    _availability = (online ? AvailabilityOnline : AvailabilityOffline);
    publishAvailability();
}

bool HABaseDeviceType::removeFromDiscovery()
{
    HAMqtt* mqttInstance = mqtt();
    if (!mqttInstance) {
        return false;
    }

    if (mqttInstance->isDeviceDiscoveryEnabled() && supportsDeviceDiscovery()) {
        return mqttInstance->removeDeviceDiscoveryComponent(this);
    }

    return removeSingleComponentDiscovery();
}

bool HABaseDeviceType::removeSingleComponentDiscovery()
{
    HAMqtt* mqttInstance = mqtt();
    if (!mqttInstance) {
        return false;
    }

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
    if (!mqttInstance->beginPublish(topic, 0, true)) {
        return false;
    }

    return mqttInstance->endPublish();
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

    return mqttInstance->republishDeviceDiscoveryComponent(this);
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
    HAMqtt* mqttInstance = mqtt();
    if (!mqttInstance) {
        return false;
    }

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
        if (!HASerializer::generateConfigTopic(
            topic,
            componentName(),
            uniqueId()
        )) {
            destroySerializer();
            return false;
        }

        if (mqttInstance->beginPublish(topic, dataLength, true)) {
            const bool flushed = _serializer->flush();
            const bool ended = mqttInstance->endPublish();
            published = flushed && ended;
        }
    }

    destroySerializer();
    return published;
}

void HABaseDeviceType::publishAvailability()
{
    HAMqtt* mqttInstance = mqtt();
    if (!mqttInstance) {
        return;
    }

    const HADevice* device = mqttInstance->getDevice();
    if (
        !device ||
        device->isSharedAvailabilityEnabled() ||
        !isAvailabilityConfigured()
    ) {
        return;
    }

    const bool online = (_availability == AvailabilityOnline);

    if (_availabilityList.count() > 0) {
        char defaultBuf[12];
        const __FlashStringHelper* flashPayload = online
            ? AHATOFSTR(HAOnline)
            : AHATOFSTR(HAOffline);

        for (uint8_t i = 0; i < _availabilityList.count(); i++) {
            const HAAvailabilityConfig::Entry& e = _availabilityList.getEntry(i);
            const char* payload = nullptr;

            if (online) {
                if (e.payloadAvailable && e.payloadAvailable[0] != '\0') {
                    payload = e.payloadAvailable;
                } else if (_payloadAvailable && _payloadAvailable[0] != '\0') {
                    payload = _payloadAvailable;
                } else {
                    strncpy_P(
                        defaultBuf,
                        reinterpret_cast<PGM_P>(AHAFROMFSTR(flashPayload)),
                        sizeof(defaultBuf) - 1
                    );
                    defaultBuf[sizeof(defaultBuf) - 1] = 0;
                    payload = defaultBuf;
                }
            } else {
                if (e.payloadNotAvailable && e.payloadNotAvailable[0] != '\0') {
                    payload = e.payloadNotAvailable;
                } else if (_payloadNotAvailable && _payloadNotAvailable[0] != '\0') {
                    payload = _payloadNotAvailable;
                } else {
                    strncpy_P(
                        defaultBuf,
                        reinterpret_cast<PGM_P>(AHAFROMFSTR(flashPayload)),
                        sizeof(defaultBuf) - 1
                    );
                    defaultBuf[sizeof(defaultBuf) - 1] = 0;
                    payload = defaultBuf;
                }
            }

            publishAbsolute(e.topic, payload, true);
        }

        return;
    }

    const char* ramPayload = online
        ? effectivePayloadAvailable()
        : effectivePayloadNotAvailable();

    if (ramPayload) {
        publishOnDataTopic(AHATOFSTR(HAAvailabilityTopic), ramPayload, true);
    } else {
        publishOnDataTopic(
            AHATOFSTR(HAAvailabilityTopic),
            online ? AHATOFSTR(HAOnline) : AHATOFSTR(HAOffline),
            true
        );
    }
}

bool HABaseDeviceType::publishAbsolute(
    const char* fullTopic,
    const char* payload,
    bool retained
)
{
    if (!fullTopic || !payload || !mqtt()) {
        return false;
    }

    HAMqtt* mqttInstance = mqtt();
    const uint16_t len = strlen(payload);
    if (!mqttInstance->beginPublish(fullTopic, len, retained)) {
        return false;
    }

    const bool written = mqttInstance->writePayload(payload, len);
    const bool ended = mqttInstance->endPublish();
    return written && ended;
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
    HAMqtt* mqttInstance = mqtt();
    if (!payload || !mqttInstance) {
        return false;
    }

    if (!topic) {
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

    if (mqttInstance->beginPublish(fullTopic, length, retained)) {
        bool written = false;
        if (isProgmemData) {
            written = mqttInstance->writePayload(AHATOFSTR(payload));
        } else {
            written = mqttInstance->writePayload(payload, length);
        }

        const bool ended = mqttInstance->endPublish();
        return written && ended;
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

void HABaseDeviceType::setEnabledByDefault(bool enabled)
{
    _enabledByDefault = enabled;
    _hasEnabledByDefault = true;
}

void HABaseDeviceType::clearEnabledByDefault()
{
    _hasEnabledByDefault = false;
}

void HABaseDeviceType::setEntityPicture(const char* url)
{
    _entityPicture = url;
}

void HABaseDeviceType::clearEntityPicture()
{
    _entityPicture = nullptr;
}

void HABaseDeviceType::setQos(uint8_t qos)
{
    if (qos > 2) {
        qos = 2;
    }

    _qosNumeric = HANumeric(static_cast<uint8_t>(qos), 0);
    _hasQos = true;
}

void HABaseDeviceType::clearQos()
{
    _hasQos = false;
    _qosNumeric.reset();
}

void HABaseDeviceType::setEncoding(const char* encoding)
{
    _encoding = encoding;
}

void HABaseDeviceType::clearEncoding()
{
    _encoding = nullptr;
}

void HABaseDeviceType::setPayloadAvailable(const char* payload)
{
    _payloadAvailable = payload;
}

void HABaseDeviceType::setPayloadNotAvailable(const char* payload)
{
    _payloadNotAvailable = payload;
}

void HABaseDeviceType::setAvailabilityMode(const char* mode)
{
    _availabilityMode = mode;
}

bool HABaseDeviceType::addAvailabilityEntry(
    const char* topic,
    const char* valueTemplate,
    const char* payloadAvailable,
    const char* payloadNotAvailable
)
{
    return _availabilityList.add(
        topic,
        valueTemplate,
        payloadAvailable,
        payloadNotAvailable
    );
}

void HABaseDeviceType::clearAvailabilityEntries()
{
    _availabilityList.clear();
}

void HABaseDeviceType::applyCommonEntityProperties(
    HASerializer* serializer,
    bool includeEncoding
) const
{
    if (!serializer) {
        return;
    }

    if (_hasEnabledByDefault) {
        serializer->set(
            AHATOFSTR(HAEnabledByDefaultProperty),
            &_enabledByDefault,
            HASerializer::BoolPropertyType
        );
    }

    if (nonEmptyString(_entityPicture)) {
        serializer->set(AHATOFSTR(HAEntityPictureProperty), _entityPicture);
    }

    if (_hasQos) {
        serializer->set(
            AHATOFSTR(HAQosProperty),
            &_qosNumeric,
            HASerializer::NumberPropertyType
        );
    }

    if (includeEncoding && nonEmptyString(_encoding)) {
        serializer->set(AHATOFSTR(HAEncodingProperty), _encoding);
    }
}

const char* HABaseDeviceType::effectivePayloadAvailable() const
{
    if (_payloadAvailable && _payloadAvailable[0] != '\0') {
        return _payloadAvailable;
    }

    HAMqtt* m = mqtt();
    if (!m) {
        return nullptr;
    }

    const HADevice* d = m->getDevice();
    if (d && d->isSharedAvailabilityEnabled()) {
        return d->getPayloadAvailable();
    }

    return nullptr;
}

const char* HABaseDeviceType::effectivePayloadNotAvailable() const
{
    if (_payloadNotAvailable && _payloadNotAvailable[0] != '\0') {
        return _payloadNotAvailable;
    }

    HAMqtt* m = mqtt();
    if (!m) {
        return nullptr;
    }

    const HADevice* d = m->getDevice();
    if (d && d->isSharedAvailabilityEnabled()) {
        return d->getPayloadNotAvailable();
    }

    return nullptr;
}

void HABaseDeviceType::configureAvailabilityEntries(HASerializer* serializer) const
{
    if (!serializer || !mqtt()) {
        return;
    }

    const HADevice* device = mqtt()->getDevice();
    if (!device) {
        return;
    }

    const bool isSharedAvailability = device->isSharedAvailabilityEnabled();
    const bool availabilityActive = isAvailabilityConfigured();

    if (!isSharedAvailability && !availabilityActive) {
        return;
    }

    const char* pa = effectivePayloadAvailable();
    if (nonEmptyString(pa)) {
        serializer->set(
            AHATOFSTR(HAPayloadAvailableProperty),
            pa,
            HASerializer::ConstCharPropertyValue
        );
    }

    const char* pn = effectivePayloadNotAvailable();
    if (nonEmptyString(pn)) {
        serializer->set(
            AHATOFSTR(HAPayloadNotAvailableProperty),
            pn,
            HASerializer::ConstCharPropertyValue
        );
    }

    if (nonEmptyString(_availabilityMode)) {
        serializer->set(
            AHATOFSTR(HAAvailabilityModeProperty),
            _availabilityMode,
            HASerializer::ConstCharPropertyValue
        );
    }

    const bool useList =
        !isSharedAvailability
        && availabilityActive
        && _availabilityList.count() > 0;

    if (useList) {
        auto* entry = serializer->addEntry();
        if (!entry) {
            return;
        }

        entry->type = HASerializer::AvailabilityArrayEntryType;
        entry->subtype = 0;
        entry->property = AHATOFSTR(HAAvailabilityListProperty);
        entry->value = const_cast<HAAvailabilityConfig*>(&_availabilityList);
        return;
    }

    auto* e = serializer->addEntry();
    if (!e) {
        return;
    }

    e->type = HASerializer::TopicEntryType;
    e->subtype = 0;
    e->property = AHATOFSTR(HAAvailabilityTopic);
    e->value = isSharedAvailability
        ? const_cast<char*>(device->getAvailabilityTopic())
        : nullptr;
}