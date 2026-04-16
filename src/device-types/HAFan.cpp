#include "HAFan.h"
#ifndef EX_ARDUINOHA_FAN

#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HASerializer.h"

HAFan::HAFan(const char* uniqueId, const uint8_t features) :
    HABaseDeviceType(AHATOFSTR(HAComponentFan), uniqueId),
    _features(features),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _speedRangeMax(),
    _speedRangeMin(),
    _currentState(false),
    _currentSpeed(0),
    _stateCallback(nullptr),
    _speedCallback(nullptr)
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    , _stateStdCallback(),
    _speedStdCallback()
#endif
{

}

bool HAFan::setState(const bool state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    if (publishState(state)) {
        _currentState = state;
        return true;
    }

    return false;
}

bool HAFan::setSpeed(const uint16_t speed, const bool force)
{
    if (!force && speed == _currentSpeed) {
        return true;
    }

    if (publishSpeed(speed)) {
        _currentSpeed = speed;
        return true;
    }

    return false;
}

void HAFan::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 15); // 15 - max properties nb
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (_retain) {
        _serializer->set(
            AHATOFSTR(HARetainProperty),
            &_retain,
            HASerializer::BoolPropertyType
        );
    }

    if (_optimistic) {
        _serializer->set(
            AHATOFSTR(HAOptimisticProperty),
            &_optimistic,
            HASerializer::BoolPropertyType
        );
    }

    if (_features & SpeedsFeature) {
        _serializer->topic(AHATOFSTR(HAPercentageStateTopic));
        _serializer->topic(AHATOFSTR(HAPercentageCommandTopic));

        if (_speedRangeMax.isSet()) {
            _serializer->set(
                AHATOFSTR(HASpeedRangeMaxProperty),
                &_speedRangeMax,
                HASerializer::NumberPropertyType
            );
        }

        if (_speedRangeMin.isSet()) {
            _serializer->set(
                AHATOFSTR(HASpeedRangeMinProperty),
                &_speedRangeMin,
                HASerializer::NumberPropertyType
            );
        }
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
    _serializer->topic(AHATOFSTR(HACommandTopic));
}

HASerializer* HAFan::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 15);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentFan),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (_retain) {
        serializer->set(
            AHATOFSTR(HARetainProperty),
            &_retain,
            HASerializer::BoolPropertyType
        );
    }

    if (_optimistic) {
        serializer->set(
            AHATOFSTR(HAOptimisticProperty),
            &_optimistic,
            HASerializer::BoolPropertyType
        );
    }

    if (_features & SpeedsFeature) {
        serializer->topic(AHATOFSTR(HAPercentageStateTopic));
        serializer->topic(AHATOFSTR(HAPercentageCommandTopic));

        if (_speedRangeMax.isSet()) {
            serializer->set(
                AHATOFSTR(HASpeedRangeMaxProperty),
                &_speedRangeMax,
                HASerializer::NumberPropertyType
            );
        }

        if (_speedRangeMin.isSet()) {
            serializer->set(
                AHATOFSTR(HASpeedRangeMinProperty),
                &_speedRangeMin,
                HASerializer::NumberPropertyType
            );
        }
    }

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    serializer->topic(AHATOFSTR(HACommandTopic));
    return serializer;
}

void HAFan::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
    publishAvailability();

    if (!_retain) {
        publishState(_currentState);
        publishSpeed(_currentSpeed);
    }

    subscribeTopic(uniqueId(), AHATOFSTR(HACommandTopic));

    if (_features & SpeedsFeature) {
        subscribeTopic(uniqueId(), AHATOFSTR(HAPercentageCommandTopic));
    }
}

void HAFan::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    if (HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        AHATOFSTR(HACommandTopic)
    )) {
        handleStateCommand(payload, length);
    } else if (HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        AHATOFSTR(HAPercentageCommandTopic)
    )) {
        handleSpeedCommand(payload, length);
    }
}

bool HAFan::publishState(const bool state)
{
    return publishOnDataTopic(
        AHATOFSTR(HAStateTopic),
        AHATOFSTR(state ? HAStateOn : HAStateOff),
        true
    );
}

bool HAFan::publishSpeed(const uint16_t speed)
{
    if (!(_features & SpeedsFeature)) {
        return false;
    }

    char str[5 + 1] = {0}; // uint16_t digits with null terminator
    HANumeric(speed, 0).toStr(str);

    return publishOnDataTopic(AHATOFSTR(HAPercentageStateTopic), str, true);
}

void HAFan::handleStateCommand(const uint8_t* cmd, const uint16_t length)
{
    (void)cmd;

    const bool hasStateCallback =
        _stateCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_stateStdCallback)
#endif
    ;

    if (!hasStateCallback) {
        return;
    }

    bool state = length == strlen_P(HAStateOn);
    if (_stateCallback) {
        _stateCallback(state, this);
    }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    if (_stateStdCallback) {
        _stateStdCallback(state, this);
    }
#endif
}

void HAFan::handleSpeedCommand(const uint8_t* cmd, const uint16_t length)
{
    const bool hasSpeedCallback =
        _speedCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_speedStdCallback)
#endif
    ;

    if (!hasSpeedCallback) {
        return;
    }

    const HANumeric& number = HANumeric::fromStr(cmd, length);
    if (number.isUInt16()) {
        const uint16_t speed = number.toUInt16();
        if (_speedCallback) {
            _speedCallback(speed, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_speedStdCallback) {
            _speedStdCallback(speed, this);
        }
#endif
    }
}

#endif
