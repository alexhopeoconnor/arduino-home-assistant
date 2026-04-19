#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastStateCallbackCall.reset(); \
    lastSpeedCallbackCall.reset();

#define assertStateCallbackCalled(expectedState, callerPtr) \
    TEST_ASSERT_TRUE(lastStateCallbackCall.called); \
    TEST_ASSERT_EQUAL(static_cast<bool>(expectedState), lastStateCallbackCall.state); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastStateCallbackCall.caller);

#define assertStateCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastStateCallbackCall.called);

#define assertSpeedCallbackCalled(expectedSpeed, callerPtr) \
    TEST_ASSERT_TRUE(lastSpeedCallbackCall.called); \
    TEST_ASSERT_EQUAL((uint16_t)expectedSpeed, lastSpeedCallbackCall.speed); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastSpeedCallbackCall.caller);

#define assertSpeedCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastSpeedCallbackCall.called);

struct StateCallback {
    bool called = false;
    bool state = false;
    HAFan* caller = nullptr;

    void reset() {
        called = false;
        state = false;
        caller = nullptr;
    }
};

struct SpeedCallback {
    bool called = false;
    uint16_t speed = 0;
    HAFan* caller = nullptr;

    void reset() {
        called = false;
        speed = 0;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueFan";
static StateCallback lastStateCallbackCall;
static SpeedCallback lastSpeedCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/fan/testDevice/uniqueFan/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueFan/stat_t"};
const char SpeedPercentageTopic[] PROGMEM = {"testData/testDevice/uniqueFan/pct_stat_t"};
const char StateCommandTopic[] PROGMEM = {"testData/testDevice/uniqueFan/cmd_t"};
const char SpeedPercentageCommandTopic[] PROGMEM = {"testData/testDevice/uniqueFan/pct_cmd_t"};

void onStateCommandReceived(bool state, HAFan* caller)
{
    lastStateCallbackCall.called = true;
    lastStateCallbackCall.state = state;
    lastStateCallbackCall.caller = caller;
}

void onSpeedCommandReceived(uint16_t speed, HAFan* caller)
{
    lastSpeedCallbackCall.called = true;
    lastSpeedCallbackCall.speed = speed;
    lastSpeedCallbackCall.caller = caller;
}

void test_FanTest_invalid_unique_id(void) {
    prepareTest

    HAFan fan(nullptr);
    fan.buildSerializerTest();
    HASerializer* serializer = fan.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_FanTest_default_params(void) {
    prepareTest

    HAFan fan(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_FanTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAFan fan(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_FanTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HAFan fan(testUniqueId);
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
                "\"uniqueFan\":{"
                    "\"p\":\"fan\","
                    "\"uniq_id\":\"uniqueFan\","
                    "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
                    "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "OFF", true);
}

void test_FanTest_default_params_with_speed(void) {
    prepareTest

    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"pct_stat_t\":\"testData/testDevice/uniqueFan/pct_stat_t\","
            "\"pct_cmd_t\":\"testData/testDevice/uniqueFan/pct_cmd_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb()); // config + default state + default speed
}

void test_FanTest_state_command_subscription(void) {
    prepareTest

    HAFan fan(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(StateCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_FanTest_speed_command_subscription(void) {
    prepareTest

    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(SpeedPercentageCommandTopic),
        mock->getSubscriptions()[1]->topic);
}

void test_FanTest_availability(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAFan
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueFan/avty_t"),
        "online",
        true
    );
}

void test_FanTest_publish_last_known_state(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setCurrentState(true);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "ON", true);
}

void test_FanTest_publish_last_known_speed(void) {
    prepareTest

    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    fan.setCurrentSpeed(50);
    mqtt.loop();

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 2, AHATOFSTR(SpeedPercentageTopic), "50", true);
}

void test_FanTest_publish_nothing_if_retained(void) {
    prepareTest

    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    fan.setRetain(true);
    fan.setCurrentState(true);
    fan.setCurrentSpeed(50);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_FanTest_name_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_object_id_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_default_entity_id_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setDefaultEntityId("fan.test_fan");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"def_ent_id\":\"fan.test_fan\","
            "\"uniq_id\":\"uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_entity_category_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_icon_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_retain_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_optimistic_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_speed_range_setter(void) {
    prepareTest

    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    fan.setSpeedRangeMin(10);
    fan.setSpeedRangeMax(1000);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"pct_stat_t\":\"testData/testDevice/uniqueFan/pct_stat_t\","
            "\"pct_cmd_t\":\"testData/testDevice/uniqueFan/pct_cmd_t\","
            "\"spd_rng_max\":1000,"
            "\"spd_rng_min\":10,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_speed_range_setter_feature_disabled(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setSpeedRangeMin(10);
    fan.setSpeedRangeMax(1000);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        fan,
        (
            "{"
            "\"uniq_id\":\"uniqueFan\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueFan/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueFan/cmd_t\""
            "}"
        )
    );
}

void test_FanTest_current_state_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setCurrentState(true);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_TRUE(fan.getCurrentState());
}

void test_FanTest_current_speed_setter(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.setCurrentSpeed(50);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL((uint16_t)50, fan.getCurrentSpeed());
}

void test_FanTest_publish_state(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId);

    TEST_ASSERT_TRUE(fan.setState(true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
}

void test_FanTest_publish_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId);
    fan.setCurrentState(true);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(fan.setState(true));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_FanTest_publish_state_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId);
    fan.setCurrentState(true);

    TEST_ASSERT_TRUE(fan.setState(true, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
}

void test_FanTest_publish_nothing_if_speed_feature_is_disabled(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId);

    TEST_ASSERT_FALSE(fan.setSpeed(50));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_FanTest_publish_speed(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId, HAFan::SpeedsFeature);

    TEST_ASSERT_TRUE(fan.setSpeed(50));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SpeedPercentageTopic), "50", true);
}

void test_FanTest_publish_speed_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    fan.setCurrentSpeed(50);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(fan.setSpeed(50));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_FanTest_publish_speed_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAFan fan(testUniqueId, HAFan::SpeedsFeature);
    fan.setCurrentSpeed(50);

    TEST_ASSERT_TRUE(fan.setSpeed(50, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SpeedPercentageTopic), "50", true);
}

void test_FanTest_state_command_on(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(StateCommandTopic), F("ON"));

    assertStateCallbackCalled(true, &fan)
}

void test_FanTest_state_command_off(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(StateCommandTopic), F("OFF"));

    assertStateCallbackCalled(false, &fan)
}

void test_FanTest_state_command_different_fan(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueFanDifferent/cmd_t"),
        F("ON")
    );

    assertStateCallbackNotCalled()
}

void test_FanTest_speed_command_half(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onSpeedCommand(onSpeedCommandReceived);
    mock->fakeMessage(AHATOFSTR(SpeedPercentageCommandTopic), F("50"));

    assertSpeedCallbackCalled(50, &fan)
}

void test_FanTest_speed_command_max(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onSpeedCommand(onSpeedCommandReceived);
    mock->fakeMessage(AHATOFSTR(SpeedPercentageCommandTopic), F("100"));

    assertSpeedCallbackCalled(100, &fan)
}

void test_FanTest_speed_command_in_range(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onSpeedCommand(onSpeedCommandReceived);
    fan.setSpeedRangeMin(1000);
    fan.setSpeedRangeMax(50000);
    mock->fakeMessage(AHATOFSTR(SpeedPercentageCommandTopic), F("49999"));

    assertSpeedCallbackCalled(49999, &fan)
}

void test_FanTest_speed_command_invalid(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onSpeedCommand(onSpeedCommandReceived);
    mock->fakeMessage(AHATOFSTR(SpeedPercentageCommandTopic), F("INVALID"));

    assertSpeedCallbackNotCalled()
}

void test_FanTest_speed_command_different_fan(void) {
    prepareTest

    HAFan fan(testUniqueId);
    fan.onSpeedCommand(onSpeedCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueFanDifferent/pct_cmd_t"),
        F("50")
    );

    assertSpeedCallbackNotCalled()
}

