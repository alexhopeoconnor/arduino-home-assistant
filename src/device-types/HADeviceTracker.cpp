#include "HADeviceTracker.h"
#ifndef EX_ARDUINOHA_DEVICE_TRACKER

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HADeviceTracker::HADeviceTracker(const char* uniqueId) :
    HABaseDeviceType(AHATOFSTR(HAComponentDeviceTracker), uniqueId),
    _icon(nullptr),
    _sourceType(SourceTypeUnknown),
    _currentState(StateUnknown)
{

}

bool HADeviceTracker::setState(const TrackerState state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    if (state == StateUnknown) {
        return false;
    }

    const bool published = publishState(state);
    _currentState = state;
    return published;
}

void HADeviceTracker::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 18);
    _serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(_serializer);
    _serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(_serializer);
    _serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    _serializer->set(AHATOFSTR(HAIconProperty), _icon);
    _serializer->set(
        AHATOFSTR(HASourceTypeProperty),
        getSourceTypeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(AHATOFSTR(HAStateTopic));
}

HASerializer* HADeviceTracker::buildDeviceDiscoverySerializer()
{
    if (!uniqueId()) {
        return nullptr;
    }

    HASerializer* serializer = new HASerializer(this, 18);
    serializer->set(
        AHATOFSTR(HAPlatformProperty),
        AHATOFSTR(HAComponentDeviceTracker),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(AHATOFSTR(HANameProperty), _name);
    setEntityIdProperty(serializer);
    serializer->set(HASerializer::WithUniqueId);
    applyCommonEntityProperties(serializer);
    serializer->set(AHATOFSTR(HAStateEntityCategory), nonEmptyString(_entityCategory));
    serializer->set(AHATOFSTR(HAIconProperty), _icon);
    serializer->set(
        AHATOFSTR(HASourceTypeProperty),
        getSourceTypeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    serializer->set(HASerializer::WithAvailability);
    serializer->topic(AHATOFSTR(HAStateTopic));
    return serializer;
}

void HADeviceTracker::onMqttConnected()
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

bool HADeviceTracker::publishState(const TrackerState state)
{
    const __FlashStringHelper *stateStr = nullptr;
    switch (state) {
    case StateHome:
        stateStr = AHATOFSTR(HAHome);
        break;

    case StateNotHome:
        stateStr = AHATOFSTR(HANotHome);
        break;

    case StateNotAvailable:
        stateStr = AHATOFSTR(HAOffline);
        break;

    default:
        return false;
    }

    return publishOnDataTopic(AHATOFSTR(HAStateTopic), stateStr, true);
}

const __FlashStringHelper* HADeviceTracker::getSourceTypeProperty() const
{
    switch (_sourceType) {
    case SourceTypeGPS:
        return AHATOFSTR(HAGPSType);

    case SourceTypeRouter:
        return AHATOFSTR(HARouterType);

    case SourceTypeBluetooth:
        return AHATOFSTR(HABluetoothType);

    case SourceTypeBluetoothLE:
        return AHATOFSTR(HABluetoothLEType);

    default:
        return nullptr;
    }
}

#endif
