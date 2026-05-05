#include "HAButton.h"
#ifndef EX_ARDUINOHA_BUTTON

#include "../HAMqtt.h"
#include "../utils/HADictionary.h"
#include "../utils/HASerializer.h"
#include <string.h>

HAButton::HAButton(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentButton), uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _retain(false),
    _payloadPress(nullptr),
    _commandTemplate(nullptr),
    _commandCallback(nullptr)
{

}

void HAButton::setPayloadPress(const char* payload)
{
    _payloadPress = payload;
}

void HAButton::setCommandTemplate(const char* commandTemplate)
{
    _commandTemplate = commandTemplate;
}

void HAButton::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 18);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HADeviceClassProperty), _class);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (nonEmptyString(_payloadPress)) {
        _serializer->set(AHATOFSTR(HAPayloadPressProperty), _payloadPress);
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

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HACommandTopic));
}

HASerializer* HAButton::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 18);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentButton),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HADeviceClassProperty), _class);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);

    if (nonEmptyString(_payloadPress)) {
        serializer->set(AHATOFSTR(HAPayloadPressProperty), _payloadPress);
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

    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HACommandTopic));
    return serializer;
}

void HAButton::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    if (shouldPublishSingleComponentConfig()) {
        publishConfig();
    }
    publishAvailability();
    subscribeTopic(uniqueId(), AHATOFSTR(HACommandTopic));
}

void HAButton::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    (void)payload;
    (void)length;

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
        if (
            nonEmptyString(_payloadPress)
            && (
                length != strlen(_payloadPress)
                || memcmp(payload, _payloadPress, length) != 0
            )
        ) {
            return;
        }

        if (_commandCallback) {
            _commandCallback(this);
        }
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        if (_commandStdCallback) {
            _commandStdCallback(this);
        }
#endif
    }
}

#endif
