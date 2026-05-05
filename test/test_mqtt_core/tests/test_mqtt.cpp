#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueId";
static HAMqtt* activeMqtt = nullptr;
static bool messageCallbackCalled = false;

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

void onMessagePublishAttempt(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    messageCallbackCalled = true;
    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_FALSE(activeMqtt->publish("testData/testDevice/echo", "1", false));
}

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

void test_MqttTest_publish_attempt_from_message_callback_is_rejected(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    messageCallbackCalled = false;
    mock->resetPublishCallsFromCallbackNb();
    mqtt.onMessage(onMessagePublishAttempt);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_TRUE(messageCallbackCalled);
    TEST_ASSERT_EQUAL_UINT16(1, mock->getPublishCallsFromCallbackNb());
    TEST_ASSERT_EQUAL_UINT8(0, mock->getFlushedMessagesNb());

    activeMqtt = nullptr;
}
