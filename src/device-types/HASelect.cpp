#include "HASelect.h"
#ifndef EX_ARDUINOHA_SELECT

#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"

HASelect::HASelect(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentSelect), uniqueId),
    _options(nullptr),
    _currentState(-1),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _valueTemplate(nullptr),
    _commandTemplate(nullptr),
    _commandCallback(nullptr)
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    , _commandStdCallback()
#endif
{

}

HASelect::~HASelect()
{
    if (_options) {
        const uint8_t optionsNb = _options->getItemsNb();
        const HASerializerArray::ItemType* options = _options->getItems();

        if (optionsNb > 1) {
            for (uint8_t i = 0; i < optionsNb; i++) {
                delete options[i];
            }
        }

        delete _options;
    }
}

void HASelect::setValueTemplate(const char* valueTemplate)
{
    _valueTemplate = valueTemplate;
}

void HASelect::setCommandTemplate(const char* commandTemplate)
{
    _commandTemplate = commandTemplate;
}

void HASelect::setOptions(const char* options)
{
    if (!options || _options) { // options can be set only once
        return;
    }

    const uint16_t optionsNb = HAUtils::countSemicolonSeparatedOptions(options);
    if (optionsNb == 0) {
        return;
    }

    const uint16_t optionsLen = strlen(options) + 1; // include null terminator
    _options = new HASerializerArray(optionsNb, false);

    if (optionsNb == 1) {
        _options->add(options);
        return;
    }

    uint16_t optionLen = 0;
    for (uint16_t i = 0; i < optionsLen; i++) {
        if (options[i] == ';' || options[i] == 0) {
            if (optionLen == 0) {
                break;
            }

            char* option = new char[optionLen + 1]; // including null terminator
            option[optionLen] = 0;
            memcpy(option, &options[i - optionLen], optionLen);

            if (!_options->add(option)) {
                delete[] option;
                break;
            }
            optionLen = 0;
            continue;
        }

        optionLen++;
    }
}

bool HASelect::setState(const int8_t state, const bool force)
{
    if (!force && _currentState == state) {
        return true;
    }

    if (!_options || state >= _options->getItemsNb()) {
        return false;
    }

    const bool published = publishState(state);
    _currentState = state;
    return published;
}

const char* HASelect::getCurrentOption() const
{
    return _options->getItem(getCurrentState());
}

void HASelect::buildSerializer()
{
    if (_serializer || !uniqueId() || !_options) {
        return;
    }

    _serializer = new HASerializer(this, 20);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(
        AHATOFSTR(HAOptionsProperty),
        _options,
        HASerializer::ArrayPropertyType
    );

    if (nonEmptyString(_valueTemplate)) {
        _serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_commandTemplate)) {
        _serializer->set(AHATOFSTR(HACommandTemplateProperty), _commandTemplate);
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

HASerializer* HASelect::buildDeviceDiscoverySerializer()
{
    if (!uniqueId() || !_options) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 20);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentSelect),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);
    serializer->set(
        AHATOFSTR(HAOptionsProperty),
        _options,
        HASerializer::ArrayPropertyType
    );

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

void HASelect::onMqttConnected()
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

void HASelect::onMqttMessage(
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
        const uint8_t optionsNb = _options->getItemsNb();
        const HASerializerArray::ItemType* options = _options->getItems();

        for (uint8_t i = 0; i < optionsNb; i++) {
            if (memcmp(payload, options[i], length) == 0) {
                if (_commandCallback) {
                    _commandCallback(i, this);
                }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
                if (_commandStdCallback) {
                    _commandStdCallback(i, this);
                }
#endif
                return;
            }
        }
    }
}

bool HASelect::publishState(const int8_t state)
{
    if (!_options || state >= _options->getItemsNb()) {
        return false;
    }

    if (state < 0) {
        return publishOnDataTopic(AHATOFSTR(HAStateTopic), AHATOFSTR(HAStateNone), true);
    }

    const char* item = _options->getItems()[state];
    if (!item) {
        return false;
    }

    return publishOnDataTopic(AHATOFSTR(HAStateTopic), item, true);
}

uint8_t HASelect::countOptionsInString(const char* options) const
{
    return HAUtils::countSemicolonSeparatedOptions(options);
}

#endif
