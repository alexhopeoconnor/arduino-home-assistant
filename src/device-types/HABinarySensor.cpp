#include "HABinarySensor.h"
#ifndef EX_ARDUINOHA_BINARY_SENSOR

#include "../HAMqtt.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"

HABinarySensor::HABinarySensor(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentBinarySensor), uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _payloadOn(nullptr),
    _payloadOff(nullptr),
    _forceUpdate(false),
    _offDelay(),
    _valueTemplate(nullptr),
    _currentState(false)
{

}

bool HABinarySensor::setState(const bool state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    const bool published = publishState(state);
    _currentState = state;
    return published;
}

void HABinarySensor::setExpireAfter(uint16_t expireAfter)
{
    if (expireAfter > 0) {
        _expireAfter.setBaseValue(expireAfter);
    } else {
        _expireAfter.reset();
    }
}

void HABinarySensor::setOffDelay(uint16_t seconds)
{
    if (seconds > 0) {
        _offDelay.setBaseValue(seconds);
    } else {
        _offDelay.reset();
    }
}

void HABinarySensor::clearOffDelay()
{
    _offDelay.reset();
}

void HABinarySensor::setValueTemplate(const char* valueTemplate)
{
    _valueTemplate = valueTemplate;
}

void HABinarySensor::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 20);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HADeviceClassProperty), _class);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (nonEmptyString(_payloadOn)) {
        _serializer->set(AHATOFSTR(HAPayloadOnProperty), _payloadOn);
    }

    if (nonEmptyString(_payloadOff)) {
        _serializer->set(AHATOFSTR(HAPayloadOffProperty), _payloadOff);
    }

    if (_forceUpdate) {
        _serializer->set(
            AHATOFSTR(HAForceUpdateProperty),
            &_forceUpdate,
            HASerializer::BoolPropertyType
        );
    }

    if (_offDelay.isSet()) {
        _serializer->set(
            AHATOFSTR(HAOffDelayProperty),
            &_offDelay,
            HASerializer::NumberPropertyType
        );
    }

    if (nonEmptyString(_valueTemplate)) {
        _serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (_expireAfter.isSet()) {
        _serializer->set(
            AHATOFSTR(HAExpireAfterProperty),
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
}

HASerializer* HABinarySensor::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 20);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentBinarySensor),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HADeviceClassProperty), _class);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (nonEmptyString(_payloadOn)) {
        serializer->set(AHATOFSTR(HAPayloadOnProperty), _payloadOn);
    }

    if (nonEmptyString(_payloadOff)) {
        serializer->set(AHATOFSTR(HAPayloadOffProperty), _payloadOff);
    }

    if (_forceUpdate) {
        serializer->set(
            AHATOFSTR(HAForceUpdateProperty),
            &_forceUpdate,
            HASerializer::BoolPropertyType
        );
    }

    if (_offDelay.isSet()) {
        serializer->set(
            AHATOFSTR(HAOffDelayProperty),
            &_offDelay,
            HASerializer::NumberPropertyType
        );
    }

    if (nonEmptyString(_valueTemplate)) {
        serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (_expireAfter.isSet()) {
        serializer->set(
            AHATOFSTR(HAExpireAfterProperty),
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    return serializer;
}

void HABinarySensor::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
    publishAvailability();
    publishState(_currentState);
}

bool HABinarySensor::publishState(const bool state)
{
    if (state) {
        if (nonEmptyString(_payloadOn)) {
            return publishOnDataTopic(
                AHATOFSTR(HAStateTopic),
                _payloadOn,
                true
            );
        }

        return publishOnDataTopic(
            AHATOFSTR(HAStateTopic),
            AHATOFSTR(HAStateOn),
            true
        );
    }

    if (nonEmptyString(_payloadOff)) {
        return publishOnDataTopic(
            AHATOFSTR(HAStateTopic),
            _payloadOff,
            true
        );
    }

    return publishOnDataTopic(
        AHATOFSTR(HAStateTopic),
        AHATOFSTR(HAStateOff),
        true
    );
}

#endif
