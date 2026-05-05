#include "HACamera.h"
#ifndef EX_ARDUINOHA_CAMERA

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HACamera::HACamera(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentCamera), uniqueId),
    _encoding(EncodingBinary),
    _icon(nullptr)
{

}

bool HACamera::publishImage(const uint8_t* data, const uint16_t length)
{
    if (!data) {
        return false;
    }

    return publishOnDataTopic(AHATOFSTR(HATopic), data, length, true);
}

void HACamera::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 18);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer, false);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(
        AHATOFSTR(HAEncodingProperty),
        getEncodingProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HATopic));
}

HASerializer* HACamera::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 18);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentCamera),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer, false);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);
    serializer->set(
        AHATOFSTR(HAEncodingProperty),
        getEncodingProperty(),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HATopic));
    return serializer;
}

void HACamera::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
    publishAvailability();
}

const __FlashStringHelper* HACamera::getEncodingProperty() const
{
    switch (_encoding) {
    case EncodingBase64:
        return AHATOFSTR(HAEncodingBase64);

    default:
        return nullptr;
    }
}

#endif
