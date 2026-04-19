#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedCommand, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_EQUAL(static_cast<bool>(expectedCommand), lastCommandCallbackCall.state); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    bool state = false;
    HASwitch* caller = nullptr;

    void reset() {
        called = false;
        state = false;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueSwitch";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/switch/testDevice/uniqueSwitch/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueSwitch/stat_t"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueSwitch/cmd_t"};

void onCommandReceived(bool state, HASwitch* caller)
{
    lastCommandCallbackCall.called = true;
    lastCommandCallbackCall.state = state;
    lastCommandCallbackCall.caller = caller;
}

void test_SwitchTest_invalid_unique_id(void) {
    prepareTest

    HASwitch testSwitch(nullptr);
    testSwitch.buildSerializerTest();
    HASerializer* serializer = testSwitch.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_SwitchTest_default_params(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
}

void test_SwitchTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HASwitch testSwitch(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueSwitch\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
}

void test_SwitchTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HASwitch testSwitch(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        0,
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueSwitch\":{"
                    "\"p\":\"switch\","
                    "\"uniq_id\":\"uniqueSwitch\","
                    "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
                    "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "OFF", true);
}

void test_SwitchTest_command_subscription(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_SwitchTest_availability(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HASwitch
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueSwitch/avty_t"),
        "online",
        true
    );
}

void test_SwitchTest_publish_last_known_state(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setCurrentState(true);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "ON", true);
}

void test_SwitchTest_publish_nothing_if_retained(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setRetain(true);
    testSwitch.setCurrentState(true);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_SwitchTest_name_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueSwitch\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_object_id_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueSwitch\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_default_entity_id_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setDefaultEntityId("switch.test_switch");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"def_ent_id\":\"switch.test_switch\","
            "\"uniq_id\":\"uniqueSwitch\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_device_class(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setDeviceClass("testClass");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"dev_cla\":\"testClass\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_entity_category_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_icon_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_retain_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_optimistic_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        testSwitch,
        (
            "{"
            "\"uniq_id\":\"uniqueSwitch\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSwitch/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSwitch/cmd_t\""
            "}"
        )
    );
}

void test_SwitchTest_current_state_setter(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.setCurrentState(true);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_TRUE(testSwitch.getCurrentState());
}

void test_SwitchTest_publish_state_on(void) {
    prepareTest

    mock->connectDummy();
    HASwitch testSwitch(testUniqueId);

    TEST_ASSERT_TRUE(testSwitch.setState(true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
}

void test_SwitchTest_publish_state_off(void) {
    prepareTest

    mock->connectDummy();
    HASwitch testSwitch(testUniqueId);
    testSwitch.setCurrentState(true);

    TEST_ASSERT_TRUE(testSwitch.setState(false));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "OFF", true);
}

void test_SwitchTest_command_on(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("ON"));

    assertCommandCallbackCalled(true, &testSwitch)
}

void test_SwitchTest_command_off(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("OFF"));

    assertCommandCallbackCalled(false, &testSwitch)
}

void test_SwitchTest_different_switch_command(void) {
    prepareTest

    HASwitch testSwitch(testUniqueId);
    testSwitch.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueSwitchDifferent/cmd_t"),
        F("CLOSE")
    );

    assertCommandCallbackNotCalled()
}

