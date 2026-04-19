#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueId";

const char ComponentNameStr[] PROGMEM = {"componentName"};

class DummyDeviceType : public HABaseDeviceType
{
public:
    DummyDeviceType(const __FlashStringHelper* componentName, const char* uniqueId) :
        HABaseDeviceType(componentName, uniqueId) { }

protected:
    virtual void onMqttConnected() override
    {
        publishAvailability();
    }
};

void test_MqttTest_maximum_number_of_device_types(void)
{
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device, 1);
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);

    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getDevicesTypesNb());
    TEST_ASSERT_EQUAL_PTR(&deviceType, mqtt.getDevicesTypes()[0]);
}

void test_MqttTest_reconnect_interval_default_value(void)
{
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device);

    TEST_ASSERT_EQUAL_UINT16(10000, mqtt.getReconnectInterval());
}

void test_MqttTest_reconnect_interval_setter(void)
{
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device);

    mqtt.setReconnectInterval(5000);
    TEST_ASSERT_EQUAL_UINT16(5000, mqtt.getReconnectInterval());

    mqtt.setReconnectInterval(0);
    TEST_ASSERT_EQUAL_UINT16(5000, mqtt.getReconnectInterval());
}

void test_MqttTest_reconnect_interval_throttles_attempts(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.begin("testHost", "testUser", "testPass");
    mqtt.setReconnectInterval(50);

    TEST_ASSERT_EQUAL_UINT16(0, mock->getConnectCallsNb());
    mqtt.loop();
    TEST_ASSERT_EQUAL_UINT16(1, mock->getConnectCallsNb());

    mock->disconnect();
    mqtt.loop();
    TEST_ASSERT_EQUAL_UINT16(1, mock->getConnectCallsNb());

    delay(60);
    mqtt.loop();
    TEST_ASSERT_EQUAL_UINT16(2, mock->getConnectCallsNb());
}
