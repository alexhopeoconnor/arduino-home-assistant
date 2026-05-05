#include "HASensor.h"
#ifndef EX_ARDUINOHA_SENSOR

#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"
#include "../utils/HASerializerArray.h"
#include <string.h>

HASensor::HASensor(const char* uniqueId, const uint16_t features) :
    HABaseDeviceType(AHATOFSTR(HAComponentSensor), uniqueId),
    _features(features),
    _deviceClass(nullptr),
    _stateClass(nullptr),
    _forceUpdate(false),
    _icon(nullptr),
    _unitOfMeasurement(nullptr),
    _expireAfter(),
    _options(nullptr),
    _suggestedDisplayPrecision(),
    _valueTemplate(nullptr),
    _jsonAttributesTemplate(nullptr),
    _lastResetValueTemplate(nullptr)
{

}

HASensor::~HASensor()
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

void HASensor::setSuggestedDisplayPrecision(uint8_t precision)
{
    _suggestedDisplayPrecision = HANumeric(static_cast<uint8_t>(precision), 0);
}

void HASensor::clearSuggestedDisplayPrecision()
{
    _suggestedDisplayPrecision.reset();
}

void HASensor::setOptions(const char* options)
{
    if (!options || _options) {
        return;
    }

    const uint16_t optionsNb = HAUtils::countSemicolonSeparatedOptions(options);
    if (optionsNb == 0) {
        return;
    }

    const uint16_t optionsLen = strlen(options) + 1;
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

            char* option = new char[optionLen + 1];
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

void HASensor::setValueTemplate(const char* valueTemplate)
{
    _valueTemplate = valueTemplate;
}

void HASensor::setJsonAttributesTemplate(const char* jsonAttributesTemplate)
{
    _jsonAttributesTemplate = jsonAttributesTemplate;
}

void HASensor::setLastResetValueTemplate(const char* lastResetValueTemplate)
{
    _lastResetValueTemplate = lastResetValueTemplate;
}

bool HASensor::setValue(const char* value)
{
    if (!value) {
        return publishOnDataTopic(AHATOFSTR(HAStateTopic), AHATOFSTR(HAStateNone), true);
    }

    return publishOnDataTopic(AHATOFSTR(HAStateTopic), value, true);
}

bool HASensor::setJsonAttributes(const char* json)
{
    return publishOnDataTopic(AHATOFSTR(HAJsonAttributesTopic), json, true);
}

void HASensor::setExpireAfter(uint16_t expireAfter)
{
    if (expireAfter > 0) {
        _expireAfter.setBaseValue(expireAfter);
    } else {
        _expireAfter.reset();
    }
}

void HASensor::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 24);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HADeviceClassProperty), _deviceClass);
    _serializer->set(AHATOFSTR(HAStateClassProperty), _stateClass);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(AHATOFSTR(HAUnitOfMeasurementProperty), nonEmptyString(_unitOfMeasurement));

    if (_suggestedDisplayPrecision.isSet()) {
        _serializer->set(
            AHATOFSTR(HASuggestedDisplayPrecisionProperty),
            &_suggestedDisplayPrecision,
            HASerializer::NumberPropertyType
        );
    }

    if (nonEmptyString(_valueTemplate)) {
        _serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_lastResetValueTemplate)) {
        _serializer->set(
            AHATOFSTR(HALastResetValueTemplateProperty),
            _lastResetValueTemplate
        );
    }

    if (_options) {
        _serializer->set(
            AHATOFSTR(HASensorOptionsProperty),
            _options,
            HASerializer::ArrayPropertyType
        );
    }

    if (_forceUpdate) {
        _serializer->set(
            AHATOFSTR(HAForceUpdateProperty),
            &_forceUpdate,
            HASerializer::BoolPropertyType
        );
    }

    if (_expireAfter.isSet()) {
        _serializer->set(
            AHATOFSTR(HAExpireAfterProperty),
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    if (_features & JsonAttributesFeature) {
        _serializer->topic(AHATOFSTR(HAJsonAttributesTopic));
        if (nonEmptyString(_jsonAttributesTemplate)) {
            _serializer->set(
                AHATOFSTR(HAJsonAttributesTemplateProperty),
                _jsonAttributesTemplate
            );
        }
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
}

HASerializer* HASensor::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 24);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentSensor),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HADeviceClassProperty), _deviceClass);
    serializer->set(AHATOFSTR(HAStateClassProperty), _stateClass);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);
    serializer->set(AHATOFSTR(HAUnitOfMeasurementProperty), nonEmptyString(_unitOfMeasurement));

    if (_suggestedDisplayPrecision.isSet()) {
        serializer->set(
            AHATOFSTR(HASuggestedDisplayPrecisionProperty),
            &_suggestedDisplayPrecision,
            HASerializer::NumberPropertyType
        );
    }

    if (nonEmptyString(_valueTemplate)) {
        serializer->set(AHATOFSTR(HAValueTemplateProperty), _valueTemplate);
    }

    if (nonEmptyString(_lastResetValueTemplate)) {
        serializer->set(
            AHATOFSTR(HALastResetValueTemplateProperty),
            _lastResetValueTemplate
        );
    }

    if (_options) {
        serializer->set(
            AHATOFSTR(HASensorOptionsProperty),
            _options,
            HASerializer::ArrayPropertyType
        );
    }

    if (_forceUpdate) {
        serializer->set(
            AHATOFSTR(HAForceUpdateProperty),
            &_forceUpdate,
            HASerializer::BoolPropertyType
        );
    }

    if (_expireAfter.isSet()) {
        serializer->set(
            AHATOFSTR(HAExpireAfterProperty),
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    if (_features & JsonAttributesFeature) {
        serializer->topic(AHATOFSTR(HAJsonAttributesTopic));
        if (nonEmptyString(_jsonAttributesTemplate)) {
            serializer->set(
                AHATOFSTR(HAJsonAttributesTemplateProperty),
                _jsonAttributesTemplate
            );
        }
    }

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    return serializer;
}

void HASensor::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
    publishAvailability();
}

#endif
