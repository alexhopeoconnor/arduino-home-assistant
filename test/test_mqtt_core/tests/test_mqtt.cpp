#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueId";
static HAMqtt* activeMqtt = nullptr;
static PubSubClientMock* activeMock = nullptr;
static bool messageCallbackCalled = false;
static bool processingFlagSeenDuringCallback = false;

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

void onMessageDeferredPublish(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    messageCallbackCalled = true;
    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/echo", "1", false));
}

void onMessageQueueOrderedPublishes(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/a", "1", false));
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/b", "2", false));
}

void onMessageInspectProcessingState(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    processingFlagSeenDuringCallback = activeMqtt->isProcessingMessage();
}

void onMessageDeferredStreamingPublish(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_TRUE(activeMqtt->beginPublish("testData/testDevice/stream", 2, false));
    activeMqtt->writePayload("O", 1);
    activeMqtt->writePayload("K", 1);
    TEST_ASSERT_TRUE(activeMqtt->endPublish());
}

void onMessageDeferredPublishThenDisconnect(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_NOT_NULL(activeMock);
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/retry", "1", false));
    activeMock->disconnect();
}

void onMessageDeferredPublishesForFailedFlush(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/a", "1", false));
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/b", "2", false));
}

void onMessageMixedDeferredPublishesPreserveOrder(const char* topic, const uint8_t* payload, uint16_t length)
{
    (void)topic;
    (void)payload;
    (void)length;

    TEST_ASSERT_NOT_NULL(activeMqtt);
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/a", "1", false));
    TEST_ASSERT_TRUE(activeMqtt->beginPublish("testData/testDevice/b", 1, false));
    activeMqtt->writePayload("2", 1);
    TEST_ASSERT_TRUE(activeMqtt->endPublish());
    TEST_ASSERT_TRUE(activeMqtt->publish("testData/testDevice/c", "3", false));
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

void test_MqttTest_publish_from_message_callback_is_deferred_and_flushed(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    messageCallbackCalled = false;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageDeferredPublish);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_TRUE(messageCallbackCalled);
    TEST_ASSERT_EQUAL_UINT16(1, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(1, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/echo", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("1", mock->getFlushedMessages()[0]->buffer);

    activeMqtt = nullptr;
}

void test_MqttTest_deferred_publish_order_is_preserved(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageQueueOrderedPublishes);

    mock->fakeMessage("testData/testDevice/input", "x");

    TEST_ASSERT_EQUAL_UINT16(2, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(2, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/a", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("1", mock->getFlushedMessages()[0]->buffer);
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/b", mock->getFlushedMessages()[1]->topic);
    TEST_ASSERT_EQUAL_STRING("2", mock->getFlushedMessages()[1]->buffer);

    activeMqtt = nullptr;
}

void test_MqttTest_processing_message_flag_only_wraps_inbound_dispatch(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    processingFlagSeenDuringCallback = false;
    TEST_ASSERT_FALSE(mqtt.isProcessingMessage());
    mqtt.onMessage(onMessageInspectProcessingState);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_TRUE(processingFlagSeenDuringCallback);
    TEST_ASSERT_FALSE(mqtt.isProcessingMessage());

    activeMqtt = nullptr;
}

void test_MqttTest_streaming_publish_from_message_callback_is_deferred_and_flushed(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageDeferredStreamingPublish);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_EQUAL_UINT16(1, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(0, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_FALSE(mqtt.hasDeferredFlushFailureForTest());
    TEST_ASSERT_EQUAL_UINT8(1, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/stream", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("OK", mock->getFlushedMessages()[0]->buffer);

    activeMqtt = nullptr;
}

void test_MqttTest_deferred_publish_is_kept_across_disconnect_and_retried_from_loop(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    activeMock = mock;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageDeferredPublishThenDisconnect);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_EQUAL_UINT16(1, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_TRUE(mqtt.hasDeferredFlushFailureForTest());
    TEST_ASSERT_TRUE(mqtt.didDeferredFlushFailDueToDisconnectForTest());
    TEST_ASSERT_EQUAL_UINT8(0, mock->getFlushedMessagesNb());

    mock->connectDummy();
    mqtt.loop();

    TEST_ASSERT_EQUAL_UINT8(0, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_FALSE(mqtt.hasDeferredFlushFailureForTest());
    TEST_ASSERT_EQUAL_UINT8(1, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/retry", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("1", mock->getFlushedMessages()[0]->buffer);

    activeMock = nullptr;
    activeMqtt = nullptr;
}

void test_MqttTest_failed_deferred_flush_keeps_queue_and_retries_in_order(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageDeferredPublishesForFailedFlush);
    mock->failNextEndPublish();

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_EQUAL_UINT16(2, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(2, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_TRUE(mqtt.hasDeferredFlushFailureForTest());
    TEST_ASSERT_TRUE(mqtt.didDeferredFlushFailAtEndPublishForTest());
    TEST_ASSERT_EQUAL_UINT8(0, mock->getFlushedMessagesNb());

    mqtt.loop();

    TEST_ASSERT_EQUAL_UINT8(0, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_FALSE(mqtt.hasDeferredFlushFailureForTest());
    TEST_ASSERT_EQUAL_UINT8(2, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/a", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("1", mock->getFlushedMessages()[0]->buffer);
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/b", mock->getFlushedMessages()[1]->topic);
    TEST_ASSERT_EQUAL_STRING("2", mock->getFlushedMessages()[1]->buffer);

    activeMqtt = nullptr;
}

void test_MqttTest_mixed_deferred_publish_order_is_preserved(void)
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.setDataPrefix("testData");
    mqtt.begin("testHost", "testUser", "testPass");
    mock->connectDummy();

    activeMqtt = &mqtt;
    mqtt.resetDeferredPublishTestCounters();
    mqtt.onMessage(onMessageMixedDeferredPublishesPreserveOrder);

    mock->fakeMessage("testData/testDevice/input", "1");

    TEST_ASSERT_EQUAL_UINT16(3, mqtt.getDeferredPublishEnqueueCountForTest());
    TEST_ASSERT_EQUAL_UINT8(0, mqtt.getPendingDeferredPublishesForTest());
    TEST_ASSERT_EQUAL_UINT8(3, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/a", mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("1", mock->getFlushedMessages()[0]->buffer);
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/b", mock->getFlushedMessages()[1]->topic);
    TEST_ASSERT_EQUAL_STRING("2", mock->getFlushedMessages()[1]->buffer);
    TEST_ASSERT_EQUAL_STRING("testData/testDevice/c", mock->getFlushedMessages()[2]->topic);
    TEST_ASSERT_EQUAL_STRING("3", mock->getFlushedMessages()[2]->buffer);

    activeMqtt = nullptr;
}
