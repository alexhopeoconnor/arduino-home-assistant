#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedCommand, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedCommand, lastCommandCallbackCall.command); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    HACover::CoverCommand command = static_cast<HACover::CoverCommand>(0);
    HACover* caller = nullptr;

    void reset() {
        called = false;
        command = static_cast<HACover::CoverCommand>(0);
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueCover";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/cover/testDevice/uniqueCover/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueCover/stat_t"};
const char PositionTopic[] PROGMEM = {"testData/testDevice/uniqueCover/pos_t"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueCover/cmd_t"};

void onCommandReceived(HACover::CoverCommand command, HACover* caller)
{
    lastCommandCallbackCall.called = true;
    lastCommandCallbackCall.command = command;
    lastCommandCallbackCall.caller = caller;
}

void test_CoverTest_invalid_unique_id(void) {
    prepareTest

    HACover cover(nullptr);
    cover.buildSerializerTest();
    HASerializer* serializer = cover.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_CoverTest_default_params(void) {
    prepareTest

    HACover cover(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_CoverTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HACover cover(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_CoverTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HACover cover(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueCover\":{"
                    "\"p\":\"cover\","
                    "\"uniq_id\":\"uniqueCover\","
                    "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
                    "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_CoverTest_default_params_with_position(void) {
    prepareTest

    HACover cover(testUniqueId, HACover::PositionFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\","
            "\"pos_t\":\"testData/testDevice/uniqueCover/pos_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_CoverTest_command_subscription(void) {
    prepareTest

    HACover cover(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_CoverTest_availability(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HACover
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueCover/avty_t"),
        "online",
        true
    );
}

void test_CoverTest_publish_last_known_state(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setCurrentState(HACover::StateClosed);
    cover.setCurrentPosition(100);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "closed", true);
}

void test_CoverTest_publish_last_known_state_with_position(void) {
    prepareTest

    HACover cover(testUniqueId, HACover::PositionFeature);
    cover.setCurrentState(HACover::StateClosed);
    cover.setCurrentPosition(100);
    mqtt.loop();

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "closed", true);
    AHA_ASSERT_MQTT_MESSAGE(mock, 2, AHATOFSTR(PositionTopic), "100", true);
}

void test_CoverTest_publish_nothing_if_retained(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setRetain(true);
    cover.setCurrentState(HACover::StateClosed);
    cover.setCurrentPosition(100);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_CoverTest_name_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_object_id_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_default_entity_id_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setDefaultEntityId("cover.test_cover");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"def_ent_id\":\"cover.test_cover\","
            "\"uniq_id\":\"uniqueCover\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_device_class(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setDeviceClass("testClass");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"dev_cla\":\"testClass\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_entity_category_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_icon_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_retain_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_optimistic_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        cover,
        (
            "{"
            "\"uniq_id\":\"uniqueCover\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueCover/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueCover/cmd_t\""
            "}"
        )
    );
}

void test_CoverTest_current_state_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setCurrentState(HACover::StateStopped);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL(HACover::StateStopped, cover.getCurrentState());
}

void test_CoverTest_current_position_setter(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.setCurrentPosition(500);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL(500, cover.getCurrentPosition());
}

void test_CoverTest_publish_state_closed(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateClosed));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "closed", true);
}

void test_CoverTest_publish_state_closing(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateClosing));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "closing", true);
}

void test_CoverTest_publish_state_open(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateOpen));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "open", true);
}

void test_CoverTest_publish_state_opening(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateOpening));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "opening", true);
}

void test_CoverTest_publish_state_stopped(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateStopped));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "stopped", true);
}

void test_CoverTest_publish_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);
    cover.setCurrentState(HACover::StateStopped);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(cover.setState(HACover::StateStopped));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_CoverTest_publish_state_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId);
    cover.setCurrentState(HACover::StateStopped);

    TEST_ASSERT_TRUE(cover.setState(HACover::StateStopped, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "stopped", true);
}

void test_CoverTest_publish_position(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId, HACover::PositionFeature);

    TEST_ASSERT_TRUE(cover.setPosition(250));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(PositionTopic), "250", true);
}

void test_CoverTest_publish_position_max(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId, HACover::PositionFeature);

    TEST_ASSERT_TRUE(cover.setPosition(32767));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(PositionTopic), "32767", true);
}

void test_CoverTest_publish_position_debounce(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId, HACover::PositionFeature);
    cover.setCurrentPosition(250);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(cover.setPosition(250));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_CoverTest_publish_position_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HACover cover(testUniqueId, HACover::PositionFeature);
    cover.setCurrentPosition(250);

    TEST_ASSERT_TRUE(cover.setPosition(250, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(PositionTopic), "250", true);
}

void test_CoverTest_command_open(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("OPEN"));

    assertCommandCallbackCalled(HACover::CommandOpen, &cover)
}

void test_CoverTest_command_close(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("CLOSE"));

    assertCommandCallbackCalled(HACover::CommandClose, &cover)
}

void test_CoverTest_command_stop(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("STOP"));

    assertCommandCallbackCalled(HACover::CommandStop, &cover)
}

void test_CoverTest_command_invalid(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("NOT_SUPPORTED"));

    assertCommandCallbackNotCalled()
}

void test_CoverTest_different_cover_command(void) {
    prepareTest

    HACover cover(testUniqueId);
    cover.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueCoverDifferent/cmd_t"),
        F("CLOSE")
    );

    assertCommandCallbackNotCalled()
}

