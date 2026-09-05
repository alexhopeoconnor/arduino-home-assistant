#include "HAText.h"
#ifndef EX_ARDUINOHA_TEXT

#include "../HAMqtt.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"
#include <new>
#include <string.h>

HAText::HAText(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentText), uniqueId),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _mode(ModeText),
    _minValue(),
    _maxValue(),
    _pattern(nullptr),
    _valueTemplate(nullptr),
    _commandTemplate(nullptr),
    _currentState(nullptr),
    _commandCallback(nullptr)
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    , _commandStdCallback()
#endif
{

}

HAText::~HAText()
{
    delete[] _currentState;
}

void HAText::setCurrentState(const char* state)
{
    setCurrentStateInternal(state);
}

bool HAText::setCurrentStateInternal(const char* state)
{
    if (!state) {
        delete[] _currentState;
        _currentState = nullptr;
        return true;
    }

    const size_t stateLength = strlen(state);
    if (stateLength > MaxCommandLength) {
        return false;
    }

    char* stateCopy = new (std::nothrow) char[stateLength + 1];
    if (!stateCopy) {
        return false;
    }

    memcpy(stateCopy, state, stateLength + 1);
    delete[] _currentState;
    _currentState = stateCopy;
    return true;
}

void HAText::setValueTemplate(const char* valueTemplate)
{
    _valueTemplate = valueTemplate;
}

void HAText::setCommandTemplate(const char* commandTemplate)
{
    _commandTemplate = commandTemplate;
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

    if (!setCurrentStateInternal(state)) {
        return false;
    }

    return publishState(_currentState);
}

void HAText::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 22);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(
        AHATOFSTR(HAModeProperty),
        getModeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(AHATOFSTR(HAPatternProperty), _pattern);

    if (nonEmptyString(_valueTemplate)) {
        _serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_commandTemplate)) {
        _serializer->set(AHATOFSTR(HACommandTemplateProperty), _commandTemplate);
    }

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

HASerializer* HAText::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 22);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentText),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);
    serializer->set(
        AHATOFSTR(HAModeProperty),
        getModeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HAPatternProperty), _pattern);

    if (nonEmptyString(_valueTemplate)) {
        serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_commandTemplate)) {
        serializer->set(AHATOFSTR(HACommandTemplateProperty), _commandTemplate);
    }

    if (_minValue.isSet()) {
        serializer->set(
            AHATOFSTR(HAMinProperty),
            &_minValue,
            HASerializer::NumberPropertyType
        );
    }

    if (_maxValue.isSet()) {
        serializer->set(
            AHATOFSTR(HAMaxProperty),
            &_maxValue,
            HASerializer::NumberPropertyType
        );
    }

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

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    serializer->topic(AHATOFSTR(HACommandTopic));
    return serializer;
}

void HAText::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
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

    if (
        hasCommandCallback &&
        length <= MaxCommandLength &&
        (length == 0 || payload) &&
        HASerializer::compareDataTopics(
            topic,
            uniqueId(),
            AHATOFSTR(HACommandTopic)
        )
    ) {
        char* value = new (std::nothrow) char[static_cast<size_t>(length) + 1];
        if (!value) {
            return;
        }

        value[length] = 0;
        if (length > 0) {
            memcpy(value, payload, length);
        }

        if (_commandCallback) {
            _commandCallback(value, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_commandStdCallback) {
            _commandStdCallback(value, this);
        }
#endif

        delete[] value;
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
