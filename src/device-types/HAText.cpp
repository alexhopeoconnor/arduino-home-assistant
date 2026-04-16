#include "HAText.h"
#ifndef EX_ARDUINOHA_TEXT

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HAText::HAText(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentText), uniqueId),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _mode(ModeText),
    _minValue(),
    _maxValue(),
    _pattern(nullptr),
    _currentState(nullptr),
    _commandCallback(nullptr)
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    , _commandStdCallback()
#endif
{

}

bool HAText::setState(const char* state, const bool force)
{
    if (!state) {
        return false;
    }

    if (
        !force &&
        _currentState &&
        strcmp(state, _currentState) == 0
    ) {
        return true;
    }

    if (publishState(state)) {
        _currentState = state;
        return true;
    }

    return false;
}

void HAText::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 14); // 14 - max properties nb
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    _serializer->set(AHATOFSTR(HAObjectIdProperty), _objectId);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(
        AHATOFSTR(HAModeProperty),
        getModeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(AHATOFSTR(HAPatternProperty), _pattern);

    if (_minValue.isSet()) {
        _serializer->set(
            AHATOFSTR(HAMinProperty),
            &_minValue,
            HASerializer::NumberPropertyType
        );
    }

    if (_maxValue.isSet()) {
        _serializer->set(
            AHATOFSTR(HAMaxProperty),
            &_maxValue,
            HASerializer::NumberPropertyType
        );
    }

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

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
    _serializer->topic(AHATOFSTR(HACommandTopic));
}

void HAText::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();

    if (!_retain && _currentState) {
        publishState(_currentState);
    }

    subscribeTopic(uniqueId(), AHATOFSTR(HACommandTopic));
}

void HAText::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    const bool hasCommandCallback =
        _commandCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_commandStdCallback)
#endif
    ;

    if (hasCommandCallback && HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        AHATOFSTR(HACommandTopic)
    )) {
        char value[length + 1];
        value[length] = 0;
        memcpy(value, payload, length);
        if (_commandCallback) {
            _commandCallback(value, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_commandStdCallback) {
            _commandStdCallback(value, this);
        }
#endif
    }
}

bool HAText::publishState(const char* state)
{
    return publishOnDataTopic(
        AHATOFSTR(HAStateTopic),
        state,
        true
    );
}

const __FlashStringHelper* HAText::getModeProperty() const
{
    return _mode == ModePassword
        ? AHATOFSTR(HAModePassword)
        : nullptr;
}

#endif
