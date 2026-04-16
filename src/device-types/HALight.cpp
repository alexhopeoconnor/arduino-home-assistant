#include "HALight.h"
#ifndef EX_ARDUINOHA_LIGHT

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

const uint8_t HALight::RGBStringMaxLength = 3*4; // 4 characters per color

void HALight::RGBColor::fromBuffer(const uint8_t* data, const uint16_t length)
{
    if (length > RGBStringMaxLength) {
        return;
    }

    uint8_t firstCommaPos = 0;
    uint8_t secondCommaPos = 0;

    for (uint8_t i = 0; i < length; i++) {
        if (data[i] == ',') {
            if (firstCommaPos == 0) {
                firstCommaPos = i;
            } else if (secondCommaPos == 0) {
                secondCommaPos = i;
            }
        }
    }

    if (firstCommaPos == 0 || secondCommaPos == 0) {
        return;
    }

    const uint8_t redLen = firstCommaPos;
    const uint8_t greenLen = secondCommaPos - firstCommaPos - 1; // minus comma
    const uint8_t blueLen = length - redLen - greenLen - 2; // minus two commas

    const HANumeric& r = HANumeric::fromStr(data, redLen);
    const HANumeric& g = HANumeric::fromStr(&data[redLen + 1], greenLen);
    const HANumeric& b = HANumeric::fromStr(&data[redLen + greenLen + 2], blueLen);

    if (r.isUInt8() && g.isUInt8() && b.isUInt8()) {
        red = r.toUInt8();
        green = g.toUInt8();
        blue = b.toUInt8();
        isSet = true;
    }
}

HALight::HALight(const char* uniqueId, const uint8_t features) :
    HABaseDeviceType(AHATOFSTR(HAComponentLight), uniqueId),
    _features(features),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _brightnessScale(),
    _currentState(false),
    _currentBrightness(0),
    _minMireds(),
    _maxMireds(),
    _currentColorTemperature(0),
    _currentRGBColor(),
    _stateCallback(nullptr),
    _brightnessCallback(nullptr),
    _colorTemperatureCallback(nullptr),
    _rgbColorCallback(nullptr)
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    , _stateStdCallback(),
    _brightnessStdCallback(),
    _colorTemperatureStdCallback(),
    _rgbColorStdCallback()
#endif
{

}

bool HALight::setState(const bool state, const bool force)
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

bool HALight::setBrightness(const uint8_t brightness, const bool force)
{
    if (!force && brightness == _currentBrightness) {
        return true;
    }

    if (publishBrightness(brightness)) {
        _currentBrightness = brightness;
        return true;
    }

    return false;
}

bool HALight::setColorTemperature(const uint16_t temperature, const bool force)
{
    if (!force && temperature == _currentColorTemperature) {
        return true;
    }

    if (publishColorTemperature(temperature)) {
        _currentColorTemperature = temperature;
        return true;
    }

    return false;
}

bool HALight::setRGBColor(const RGBColor& color, const bool force)
{
    if (!force && color == _currentRGBColor) {
        return true;
    }

    if (publishRGBColor(color)) {
        _currentRGBColor = color;
        return true;
    }

    return false;
}

void HALight::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 20); // 20 - max properties nb
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

    if (_features & BrightnessFeature) {
        _serializer->topic(AHATOFSTR(HABrightnessStateTopic));
        _serializer->topic(AHATOFSTR(HABrightnessCommandTopic));

        if (_brightnessScale.isSet()) {
            _serializer->set(
                AHATOFSTR(HABrightnessScaleProperty),
                &_brightnessScale,
                HASerializer::NumberPropertyType
            );
        }
    }

    if (_features & ColorTemperatureFeature) {
        _serializer->topic(AHATOFSTR(HAColorTemperatureStateTopic));
        _serializer->topic(AHATOFSTR(HAColorTemperatureCommandTopic));

        if (_minMireds.isSet()) {
            _serializer->set(
                AHATOFSTR(HAMinMiredsProperty),
                &_minMireds,
                HASerializer::NumberPropertyType
            );
        }

        if (_maxMireds.isSet()) {
            _serializer->set(
                AHATOFSTR(HAMaxMiredsProperty),
                &_maxMireds,
                HASerializer::NumberPropertyType
            );
        }
    }

    if (_features & RGBFeature) {
        _serializer->topic(AHATOFSTR(HARGBCommandTopic));
        _serializer->topic(AHATOFSTR(HARGBStateTopic));
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
    _serializer->topic(AHATOFSTR(HACommandTopic));
}

HASerializer* HALight::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 20);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentLight),
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

    if (_features & BrightnessFeature) {
        serializer->topic(AHATOFSTR(HABrightnessStateTopic));
        serializer->topic(AHATOFSTR(HABrightnessCommandTopic));

        if (_brightnessScale.isSet()) {
            serializer->set(
                AHATOFSTR(HABrightnessScaleProperty),
                &_brightnessScale,
                HASerializer::NumberPropertyType
            );
        }
    }

    if (_features & ColorTemperatureFeature) {
        serializer->topic(AHATOFSTR(HAColorTemperatureStateTopic));
        serializer->topic(AHATOFSTR(HAColorTemperatureCommandTopic));

        if (_minMireds.isSet()) {
            serializer->set(
                AHATOFSTR(HAMinMiredsProperty),
                &_minMireds,
                HASerializer::NumberPropertyType
            );
        }

        if (_maxMireds.isSet()) {
            serializer->set(
                AHATOFSTR(HAMaxMiredsProperty),
                &_maxMireds,
                HASerializer::NumberPropertyType
            );
        }
    }

    if (_features & RGBFeature) {
        serializer->topic(AHATOFSTR(HARGBCommandTopic));
        serializer->topic(AHATOFSTR(HARGBStateTopic));
    }

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    serializer->topic(AHATOFSTR(HACommandTopic));
    return serializer;
}

void HALight::onMqttConnected()
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
        publishBrightness(_currentBrightness);
        publishColorTemperature(_currentColorTemperature);
        publishRGBColor(_currentRGBColor);
    }

    subscribeTopic(uniqueId(), AHATOFSTR(HACommandTopic));

    if (_features & BrightnessFeature) {
        subscribeTopic(uniqueId(), AHATOFSTR(HABrightnessCommandTopic));
    }

    if (_features & ColorTemperatureFeature) {
        subscribeTopic(uniqueId(), AHATOFSTR(HAColorTemperatureCommandTopic));
    }

    if (_features & RGBFeature) {
        subscribeTopic(uniqueId(), AHATOFSTR(HARGBCommandTopic));
    }
}

void HALight::onMqttMessage(
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
        AHATOFSTR(HABrightnessCommandTopic)
    )) {
        handleBrightnessCommand(payload, length);
    } else if (HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        AHATOFSTR(HAColorTemperatureCommandTopic)
    )) {
        handleColorTemperatureCommand(payload, length);
    } else if (
        HASerializer::compareDataTopics(
            topic,
            uniqueId(),
            AHATOFSTR(HARGBCommandTopic)
        )
    ) {
        handleRGBCommand(payload, length);
    }
}

bool HALight::publishState(const bool state)
{
    return publishOnDataTopic(
        AHATOFSTR(HAStateTopic),
        AHATOFSTR(state ? HAStateOn : HAStateOff),
        true
    );
}

bool HALight::publishBrightness(const uint8_t brightness)
{
    if (!(_features & BrightnessFeature)) {
        return false;
    }

    char str[3 + 1] = {0}; // uint8_t digits with null terminator
    HANumeric(brightness, 0).toStr(str);

    return publishOnDataTopic(AHATOFSTR(HABrightnessStateTopic), str, true);
}

bool HALight::publishColorTemperature(const uint16_t temperature)
{
    if (!(_features & ColorTemperatureFeature)) {
        return false;
    }

    char str[5 + 1] = {0}; // uint16_t digits with null terminator
    HANumeric(temperature, 0).toStr(str);

    return publishOnDataTopic(AHATOFSTR(HAColorTemperatureStateTopic), str, true);
}

bool HALight::publishRGBColor(const RGBColor& color)
{
    if (!(_features & RGBFeature) || !color.isSet) {
        return false;
    }

    char str[RGBStringMaxLength] = {0};
    uint16_t len = 0;

    // append red color with comma
    len += HANumeric(color.red, 0).toStr(&str[0]);
    str[len++] = ',';

    // append green color with comma
    len += HANumeric(color.green, 0).toStr(&str[len]);
    str[len++] = ',';

    // append blue color
    HANumeric(color.blue, 0).toStr(&str[len]);

    return publishOnDataTopic(AHATOFSTR(HARGBStateTopic), str, true);
}

void HALight::handleStateCommand(const uint8_t* cmd, const uint16_t length)
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

void HALight::handleBrightnessCommand(const uint8_t* cmd, const uint16_t length)
{
    const bool hasBrightnessCallback =
        _brightnessCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_brightnessStdCallback)
#endif
    ;

    if (!hasBrightnessCallback) {
        return;
    }

    const HANumeric& number = HANumeric::fromStr(cmd, length);
    if (number.isUInt8()) {
        const uint8_t brightness = number.toUInt8();
        if (_brightnessCallback) {
            _brightnessCallback(brightness, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_brightnessStdCallback) {
            _brightnessStdCallback(brightness, this);
        }
#endif
    }
}

void HALight::handleColorTemperatureCommand(
    const uint8_t* cmd,
    const uint16_t length
)
{
    const bool hasColorTemperatureCallback =
        _colorTemperatureCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_colorTemperatureStdCallback)
#endif
    ;

    if (!hasColorTemperatureCallback) {
        return;
    }

    const HANumeric& number = HANumeric::fromStr(cmd, length);
    if (number.isUInt16()) {
        const uint16_t temperature = number.toUInt16();
        if (_colorTemperatureCallback) {
            _colorTemperatureCallback(temperature, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_colorTemperatureStdCallback) {
            _colorTemperatureStdCallback(temperature, this);
        }
#endif
    }
}

void HALight::handleRGBCommand(const uint8_t* cmd, const uint16_t length)
{
    const bool hasRgbCallback =
        _rgbColorCallback
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        || static_cast<bool>(_rgbColorStdCallback)
#endif
    ;

    if (!hasRgbCallback) {
        return;
    }

    RGBColor color;
    color.fromBuffer(cmd, length);

    if (color.isSet) {
        if (_rgbColorCallback) {
            _rgbColorCallback(color, this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_rgbColorStdCallback) {
            _rgbColorStdCallback(color, this);
        }
#endif
    }
}

#endif
