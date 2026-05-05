#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedValue, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_EQUAL_STRING(expectedValue, lastCommandCallbackCall.value); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    char value[64] = {0};
    HAText* caller = nullptr;

    void reset() {
        called = false;
        value[0] = 0;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueText";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/text/testDevice/uniqueText/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueText/stat_t"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueText/cmd_t"};

void onCommandReceived(const char* value, HAText* caller)
{
    lastCommandCallbackCall.called = true;
    strncpy(
        lastCommandCallbackCall.value,
        value,
        sizeof(lastCommandCallbackCall.value) - 1
    );
    lastCommandCallbackCall.value[sizeof(lastCommandCallbackCall.value) - 1] = 0;
    lastCommandCallbackCall.caller = caller;
}

void onCommandPublishAttempt(const char* value, HAText* caller)
{
    TEST_ASSERT_FALSE(caller->setState(value));
}

void test_TextTest_invalid_unique_id(void) {
    prepareTest

    HAText text(nullptr);
    text.buildSerializerTest();
    HASerializer* serializer = text.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_TextTest_default_params(void) {
    prepareTest

    HAText text(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAText text(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueText\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_command_subscription(void) {
    prepareTest

    HAText text(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_TextTest_availability(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAText
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueText/avty_t"),
        "online",
        true
    );
}

void test_TextTest_publish_last_known_state(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setCurrentState("initial");
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "initial", true);
}

void test_TextTest_publish_nothing_if_retained(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setRetain(true);
    text.setCurrentState("initial");
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_TextTest_name_setter(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueText\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_object_id_setter(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueText\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_icon_setter(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_retain_setter(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_optimistic_setter(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_mode_setter_password(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setMode(HAText::ModePassword);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"mode\":\"password\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_min_max_pattern_setters(void) {
    prepareTest

    HAText text(testUniqueId);
    text.setMin(2);
    text.setMax(40);
    text.setPattern("^[a-zA-Z0-9]+$");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        text,
        (
            "{"
            "\"uniq_id\":\"uniqueText\","
            "\"pattern\":\"^[a-zA-Z0-9]+$\","
            "\"min\":2,"
            "\"max\":40,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueText/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueText/cmd_t\""
            "}"
        )
    );
}

void test_TextTest_publish_state(void) {
    prepareTest

    mock->connectDummy();
    HAText text(testUniqueId);

    TEST_ASSERT_TRUE(text.setState("new-value"));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "new-value", true);
}

void test_TextTest_publish_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAText text(testUniqueId);
    text.setCurrentState("new-value");

    TEST_ASSERT_TRUE(text.setState("new-value"));
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

void test_TextTest_command_callback(void) {
    prepareTest

    HAText text(testUniqueId);
    text.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("hello"));

    assertCommandCallbackCalled("hello", &text)
}

void test_TextTest_callback_publish_attempt_is_rejected(void) {
    prepareTest

    mock->connectDummy();
    HAText text(testUniqueId);
    text.onCommand(onCommandPublishAttempt);

    mock->fakeMessage(AHATOFSTR(CommandTopic), F("hello"));

    TEST_ASSERT_EQUAL(1, mock->getPublishCallsFromCallbackNb());
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

void test_TextTest_different_text_command(void) {
    prepareTest

    HAText text(testUniqueId);
    text.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueTextDifferent/cmd_t"),
        F("hello")
    );

    assertCommandCallbackNotCalled()
}

