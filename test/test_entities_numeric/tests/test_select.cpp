#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedIndex, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedIndex, lastCommandCallbackCall.index); \
    TEST_ASSERT_EQUAL(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    int8_t index = -1;
    HASelect* caller = nullptr;

    void reset() {
        called = false;
        index = -1;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueSelect";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/select/testDevice/uniqueSelect/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueSelect/stat_t"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueSelect/cmd_t"};

void onCommandReceived(int8_t index, HASelect* caller)
{
    lastCommandCallbackCall.called = true;
    lastCommandCallbackCall.index = index;
    lastCommandCallbackCall.caller = caller;
}

void onCommandPublishAttempt(int8_t index, HASelect* caller)
{
    TEST_ASSERT_FALSE(caller->setState(index));
}

void test_SelectTest_invalid_unique_id(void) {
    prepareTest

    HASelect select(nullptr);
    select.buildSerializerTest();
    HASerializer* serializer = select.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_SelectTest_no_options(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.buildSerializerTest();
    HASerializer* serializer = select.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
    TEST_ASSERT_TRUE(select.getOptions() == nullptr);
}

void test_SelectTest_invalid_options_nullptr(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions(nullptr);
    select.buildSerializerTest();
    HASerializer* serializer = select.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
    TEST_ASSERT_TRUE(select.getOptions() == nullptr);
}

void test_SelectTest_invalid_options_empty(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("");
    select.buildSerializerTest();
    HASerializer* serializer = select.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
    TEST_ASSERT_TRUE(select.getOptions() == nullptr);
}

void test_SelectTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HASelect select(testUniqueId);
    select.setOptions("Option A");

    TEST_ASSERT_EQUAL(1, select.getOptions()->getItemsNb());
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueSelect\","
            "\"options\":[\"Option A\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
}

void test_SelectTest_single_option(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A");

    TEST_ASSERT_EQUAL(1, select.getOptions()->getItemsNb());
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
}

void test_SelectTest_multiple_options(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");

    TEST_ASSERT_EQUAL(3, select.getOptions()->getItemsNb());
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
}

void test_SelectTest_long_options_string_over_255_chars(void) {
    prepareTest

    char options[273];
    for (uint16_t i = 0; i < 270; i++) {
        options[i] = 'A';
    }

    options[270] = ';';
    options[271] = 'B';
    options[272] = 0;

    HASelect select(testUniqueId);
    select.setOptions(options);

    TEST_ASSERT_TRUE(select.getOptions() != nullptr);
    TEST_ASSERT_EQUAL((uint8_t)2, select.getOptions()->getItemsNb());
    TEST_ASSERT_EQUAL_STRING("B", select.getOptions()->getItem(1));
}

void test_SelectTest_command_subscription(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_SelectTest_availability(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HACover
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        "testData/testDevice/uniqueSelect/avty_t",
        "online",
        true
    );
}

void test_SelectTest_publish_last_known_state(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setCurrentState(1);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("B", select.getCurrentOption());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "B", true);
}

void test_SelectTest_publish_nothing_if_retained(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setCurrentState(1);
    select.setRetain(true);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_SelectTest_publish_state_none(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "None", true);
}

void test_SelectTest_name_setter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
}

void test_SelectTest_object_id_setter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
}

void test_SelectTest_icon_setter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"uniqueSelect\","
            "\"ic\":\"testIcon\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
}

void test_SelectTest_retain_setter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
}

void test_SelectTest_optimistic_setter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        select,
        (
            "{"
            "\"uniq_id\":\"uniqueSelect\","
            "\"options\":[\"Option A\",\"B\",\"C\"],"
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSelect/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueSelect/cmd_t\""
            "}"
        )
    );
}

void test_SelectTest_current_state_getter(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.setCurrentState(1);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL(1, select.getCurrentState());
    TEST_ASSERT_EQUAL_STRING("B", select.getCurrentOption());
}

void test_SelectTest_publish_state_first(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");

    TEST_ASSERT_TRUE(select.setState(0));
    TEST_ASSERT_TRUE(select.getOptions() != nullptr);
    TEST_ASSERT_EQUAL_STRING("Option A", select.getCurrentOption());
    TEST_ASSERT_EQUAL(3, select.getOptions()->getItemsNb());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "Option A", true);
}

void test_SelectTest_publish_state_last(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");

    TEST_ASSERT_TRUE(select.setState(2));
    TEST_ASSERT_TRUE(select.getOptions() != nullptr);
    TEST_ASSERT_EQUAL_STRING("C", select.getCurrentOption());
    TEST_ASSERT_EQUAL(3, select.getOptions()->getItemsNb());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "C", true);
}

void test_SelectTest_publish_state_only(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A");

    TEST_ASSERT_TRUE(select.setState(0));
    TEST_ASSERT_TRUE(select.getOptions() != nullptr);
    TEST_ASSERT_EQUAL(1, select.getOptions()->getItemsNb());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "Option A", true);
}

void test_SelectTest_publish_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A");
    select.setCurrentState(0);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(select.setState(0));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_SelectTest_publish_state_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A");
    select.setCurrentState(0);

    TEST_ASSERT_TRUE(select.setState(0, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "Option A", true);
}

void test_SelectTest_command_option_first(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("Option A"));

    assertCommandCallbackCalled(0, &select)
}

void test_SelectTest_command_option_middle(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("B"));

    assertCommandCallbackCalled(1, &select)
}

void test_SelectTest_command_option_last(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("C"));

    assertCommandCallbackCalled(2, &select)
}

void test_SelectTest_command_option_non_existing(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("DoesNotExist"));

    assertCommandCallbackNotCalled()
}

void test_SelectTest_callback_publish_attempt_is_rejected(void) {
    prepareTest

    mock->connectDummy();
    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandPublishAttempt);

    mock->fakeMessage(AHATOFSTR(CommandTopic), F("B"));

    TEST_ASSERT_EQUAL(1, mock->getPublishCallsFromCallbackNb());
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

void test_SelectTest_different_select_command(void) {
    prepareTest

    HASelect select(testUniqueId);
    select.setOptions("Option A;B;C");
    select.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueDifferentSelect/cmd_t"),
        F("Option A")
    );

    assertCommandCallbackNotCalled()
}


