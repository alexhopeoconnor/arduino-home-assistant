#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastPressCallbackCall.reset();

#define assertPressCallbackCalled(callerPtr) \
    TEST_ASSERT_TRUE(lastPressCallbackCall.called); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastPressCallbackCall.caller);

#define assertPressCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastPressCallbackCall.called);

struct PressCallback {
    bool called = false;
    HAButton* caller = nullptr;

    void reset() {
        called = false;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueButton";
static PressCallback lastPressCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/button/testDevice/uniqueButton/config"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueButton/cmd_t"};
const char CommandMessage[] PROGMEM = {"PRESS"};

void onCommandReceived(HAButton* caller)
{
    lastPressCallbackCall.called = true;
    lastPressCallbackCall.caller = caller;
}

void test_ButtonTest_invalid_unique_id(void) {
    prepareTest

    HAButton button(nullptr);
    button.buildSerializerTest();
    HASerializer* serializer = button.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_ButtonTest_default_params(void) {
    prepareTest

    HAButton button(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"uniqueButton\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAButton button(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueButton\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_command_subscription(void) {
    prepareTest

    HAButton button(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_ButtonTest_availability(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAButton
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueButton/avty_t"),
        "online",
        true
    );
}

void test_ButtonTest_name_setter(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueButton\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_object_id_setter(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueButton\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_device_class(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setDeviceClass("testClass");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"uniqueButton\","
            "\"dev_cla\":\"testClass\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_entity_category_setter(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"uniqueButton\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_icon_setter(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"uniqueButton\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_retain_setter(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        button,
        (
            "{"
            "\"uniq_id\":\"uniqueButton\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"cmd_t\":\"testData/testDevice/uniqueButton/cmd_t\""
            "}"
        )
    );
}

void test_ButtonTest_command_callback(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), AHATOFSTR(CommandMessage));

    assertPressCallbackCalled(&button)
}

void test_ButtonTest_no_command_callback(void) {
    prepareTest

    HAButton button(testUniqueId);
    mock->fakeMessage(AHATOFSTR(CommandTopic), AHATOFSTR(CommandMessage));

    assertPressCallbackNotCalled()
}

void test_ButtonTest_different_button_command(void) {
    prepareTest

    HAButton button(testUniqueId);
    button.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueButtonDifferent/cmd_t"),
        AHATOFSTR(CommandMessage)
    );

    assertPressCallbackNotCalled()
}

