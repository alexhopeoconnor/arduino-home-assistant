#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueId";

const char AvailabilityTopic[] PROGMEM = {"testData/testDevice/uniqueId/avty_t"};
const char SharedAvailabilityTopic[] PROGMEM = {"testData/testDevice/avty_t"};
const char ConfigTopic[] PROGMEM = {"homeassistant/componentName/testDevice/uniqueId/config"};
const char ComponentNameStr[] PROGMEM = {"componentName"};

class DummyDeviceType : public HABaseDeviceType
{
public:
    DummyDeviceType(const __FlashStringHelper* componentName, const char* uniqueId) :
        HABaseDeviceType(componentName, uniqueId) { }

protected:
    virtual void buildSerializer() override
    {
        if (_serializer || !uniqueId()) {
            return;
        }

        _serializer = new HASerializer(this, 3);
        _serializer->set(AHATOFSTR(HANameProperty), "testName");
        setEntityIdProperty(_serializer);
        _serializer->set(HASerializer::WithUniqueId);
    }

    virtual void onMqttConnected() override
    {
        publishAvailability();
    }
};

#define prepareTest \
    initMqttTest(testDeviceId); \
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId)

void test_BaseDeviceTypeTest_constructor_params(void)
{
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    TEST_ASSERT_EQUAL_PTR(AHATOFSTR(ComponentNameStr), deviceType.componentName());
    TEST_ASSERT_EQUAL_STRING(testUniqueId, deviceType.uniqueId());
}

void test_BaseDeviceTypeTest_register_mqtt_type(void)
{
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device);
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);

    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getDevicesTypesNb());
    TEST_ASSERT_EQUAL_PTR(&deviceType, mqtt.getDevicesTypes()[0]);
}

void test_BaseDeviceTypeTest_default_name(void)
{
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    TEST_ASSERT_NULL(deviceType.getName());
}

void test_BaseDeviceTypeTest_name_setter(void)
{
    const char* name = "testName";
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    deviceType.setName(name);
    TEST_ASSERT_EQUAL_STRING(name, deviceType.getName());
}

void test_BaseDeviceTypeTest_object_id_setter(void)
{
    const char* objectId = "testId";
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    deviceType.setObjectId(objectId);
    TEST_ASSERT_EQUAL_STRING(objectId, deviceType.getObjectId());
}

void test_BaseDeviceTypeTest_default_entity_id_setter(void)
{
    const char* entityId = "sensor.test_id";
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    deviceType.setDefaultEntityId(entityId);
    TEST_ASSERT_EQUAL_STRING(entityId, deviceType.getDefaultEntityId());
}

void test_BaseDeviceTypeTest_default_availability(void)
{
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);
    TEST_ASSERT_FALSE(deviceType.isAvailabilityConfigured());
}

void test_BaseDeviceTypeTest_publish_nothing_if_not_configured(void)
{
    prepareTest;

    mqtt.loop();
    AHA_ASSERT_NO_MQTT_MESSAGE(mock);
}

void test_BaseDeviceTypeTest_publish_availability_online_runtime(void)
{
    prepareTest;

    mock->connectDummy();
    deviceType.setAvailability(true);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AvailabilityTopic), "online", true);
}

void test_BaseDeviceTypeTest_publish_availability_offline_runtime(void)
{
    prepareTest;

    mock->connectDummy();
    deviceType.setAvailability(false);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AvailabilityTopic), "offline", true);
}

void test_BaseDeviceTypeTest_publish_shared_availability_on_connect(void)
{
    prepareTest;

    device.enableSharedAvailability();
    mqtt.loop();
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SharedAvailabilityTopic), "online", true);
}

void test_BaseDeviceTypeTest_publish_shared_availability_runtime(void)
{
    prepareTest;

    device.enableSharedAvailability();
    mock->connectDummy();
    device.setAvailability(false);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SharedAvailabilityTopic), "offline", true);
}

void test_BaseDeviceTypeTest_republish_discovery(void)
{
    prepareTest;

    mock->connectDummy();
    TEST_ASSERT_TRUE(deviceType.republishDiscovery());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(
        mock,
        AHATOFSTR(ConfigTopic),
        "{\"name\":\"testName\",\"uniq_id\":\"uniqueId\"}",
        true
    );
}

void test_BaseDeviceTypeTest_remove_from_discovery(void)
{
    prepareTest;

    mock->connectDummy();
    TEST_ASSERT_TRUE(deviceType.removeFromDiscovery());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ConfigTopic), "", true);
}
