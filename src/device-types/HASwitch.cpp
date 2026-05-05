#include "HASwitch.h"
#ifndef EX_ARDUINOHA_SWITCH

#include "../HAMqtt.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"
#include <string.h>

static bool payloadEquals(
    const uint8_t* data,
    const uint16_t length,
    const char* ram,
    const __FlashStringHelper* flash
)
{
    if (ram) {
        const size_t n = strlen(ram);
        return (length == n) && (memcmp(data, ram, length) == 0);
    }

    const size_t n = strlen_P(AHAFROMFSTR(flash));
    return (length == n) && (memcmp_P(data, AHAFROMFSTR(flash), n) == 0);
}

HASwitch::HASwitch(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentSwitch), uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _currentState(false),
    _payloadOn(nullptr),
    _payloadOff(nullptr),
    _stateOn(nullptr),
    _stateOff(nullptr),
    _valueTemplate(nullptr),
    _commandTemplate(nullptr),
    _commandCallback(nullptr)
{

}

void HASwitch::setValueTemplate(const char* valueTemplate)
{
    _valueTemplate = valueTemplate;
}

void HASwitch::setCommandTemplate(const char* commandTemplate)
{
    _commandTemplate = commandTemplate;
}

bool HASwitch::setState(const bool state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    const bool published = publishState(state);
    _currentState = state;
    return published;
}

void HASwitch::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 24);
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

    if (nonEmptyString(_stateOn)) {
        _serializer->set(AHATOFSTR(HAStateOnDiscoveryProperty), _stateOn);
    }

    if (nonEmptyString(_stateOff)) {
        _serializer->set(AHATOFSTR(HAStateOffDiscoveryProperty), _stateOff);
    }

    if (nonEmptyString(_valueTemplate)) {
        _serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_commandTemplate)) {
        _serializer->set(AHATOFSTR(HACommandTemplateProperty), _commandTemplate);
    }

    // optional property
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

HASerializer* HASwitch::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 24);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentSwitch),
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

    if (nonEmptyString(_stateOn)) {
        serializer->set(AHATOFSTR(HAStateOnDiscoveryProperty), _stateOn);
    }

    if (nonEmptyString(_stateOff)) {
        serializer->set(AHATOFSTR(HAStateOffDiscoveryProperty), _stateOff);
    }

    if (nonEmptyString(_valueTemplate)) {
        serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_commandTemplate)) {
        serializer->set(AHATOFSTR(HACommandTemplateProperty), _commandTemplate);
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

void HASwitch::onMqttConnected()
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
    }

    subscribeTopic(uniqueId(), AHATOFSTR(HACommandTopic));
}

void HASwitch::onMqttMessage(
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
        const bool isOn = payloadEquals(
            payload,
            length,
            _payloadOn,
            AHATOFSTR(HAStateOn)
        );
        const bool isOff = payloadEquals(
            payload,
            length,
            _payloadOff,
            AHATOFSTR(HAStateOff)
        );

        if (!isOn && !isOff) {
            return;
        }

        const bool state = isOn;
        if (_commandCallback) {
            _commandCallback(state, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_commandStdCallback) {
            _commandStdCallback(state, this);
        }
#endif
    }
}

bool HASwitch::publishState(const bool state)
{
    if (state) {
        if (nonEmptyString(_stateOn)) {
            return publishOnDataTopic(
                AHATOFSTR(HAStateTopic),
                _stateOn,
                true
            );
        }

        return publishOnDataTopic(
            AHATOFSTR(HAStateTopic),
            AHATOFSTR(HAStateOn),
            true
        );
    }

    if (nonEmptyString(_stateOff)) {
        return publishOnDataTopic(
            AHATOFSTR(HAStateTopic),
            _stateOff,
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
