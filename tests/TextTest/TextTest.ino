#include <AUnit.h>
#include <ArduinoHA.h>

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedValue, callerPtr) \
    assertTrue(lastCommandCallbackCall.called); \
    assertEqual(expectedValue, lastCommandCallbackCall.value); \
    assertEqual(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    assertFalse(lastCommandCallbackCall.called);

using aunit::TestRunner;

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

AHA_TEST(TextTest, invalid_unique_id) {
    prepareTest

    HAText text(nullptr);
    text.buildSerializerTest();
    HASerializer* serializer = text.getSerializer();

    assertTrue(serializer == nullptr);
}

AHA_TEST(TextTest, default_params) {
    prepareTest

    HAText text(testUniqueId);
    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, extended_unique_id) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAText text(testUniqueId);
    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, command_subscription) {
    prepareTest

    HAText text(testUniqueId);
    mqtt.loop();

    assertEqual(1, mock->getSubscriptionsNb());
    assertEqual(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

AHA_TEST(TextTest, availability) {
    prepareTest

    HAText text(testUniqueId);
    text.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAText
    assertMqttMessage(
        1,
        F("testData/testDevice/uniqueText/avty_t"),
        "online",
        true
    )
}

AHA_TEST(TextTest, publish_last_known_state) {
    prepareTest

    HAText text(testUniqueId);
    text.setCurrentState("initial");
    mqtt.loop();

    assertEqual(2, mock->getFlushedMessagesNb());
    assertMqttMessage(1, AHATOFSTR(StateTopic), "initial", true)
}

AHA_TEST(TextTest, publish_nothing_if_retained) {
    prepareTest

    HAText text(testUniqueId);
    text.setRetain(true);
    text.setCurrentState("initial");
    mqtt.loop();

    assertEqual(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

AHA_TEST(TextTest, name_setter) {
    prepareTest

    HAText text(testUniqueId);
    text.setName("testName");

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, object_id_setter) {
    prepareTest

    HAText text(testUniqueId);
    text.setObjectId("testId");

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, icon_setter) {
    prepareTest

    HAText text(testUniqueId);
    text.setIcon("testIcon");

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, retain_setter) {
    prepareTest

    HAText text(testUniqueId);
    text.setRetain(true);

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, optimistic_setter) {
    prepareTest

    HAText text(testUniqueId);
    text.setOptimistic(true);

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, mode_setter_password) {
    prepareTest

    HAText text(testUniqueId);
    text.setMode(HAText::ModePassword);

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, min_max_pattern_setters) {
    prepareTest

    HAText text(testUniqueId);
    text.setMin(2);
    text.setMax(40);
    text.setPattern("^[a-zA-Z0-9]+$");

    assertEntityConfig(
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
    )
}

AHA_TEST(TextTest, publish_state) {
    prepareTest

    mock->connectDummy();
    HAText text(testUniqueId);

    assertTrue(text.setState("new-value"));
    assertSingleMqttMessage(AHATOFSTR(StateTopic), "new-value", true)
}

AHA_TEST(TextTest, publish_state_debounce) {
    prepareTest

    mock->connectDummy();
    HAText text(testUniqueId);
    text.setCurrentState("new-value");

    assertTrue(text.setState("new-value"));
    assertEqual(0, mock->getFlushedMessagesNb());
}

AHA_TEST(TextTest, command_callback) {
    prepareTest

    HAText text(testUniqueId);
    text.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("hello"));

    assertCommandCallbackCalled("hello", &text)
}

AHA_TEST(TextTest, different_text_command) {
    prepareTest

    HAText text(testUniqueId);
    text.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueTextDifferent/cmd_t"),
        F("hello")
    );

    assertCommandCallbackNotCalled()
}

void setup()
{
    delay(1000);
    Serial.begin(115200);
    while (!Serial);
}

void loop()
{
    TestRunner::run();
    delay(1);
}
