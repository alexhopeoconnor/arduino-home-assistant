#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedNumber, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_TRUE(expectedNumber == lastCommandCallbackCall.number); \
    TEST_ASSERT_EQUAL(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    HANumeric number;
    HANumber* caller = nullptr;

    void reset() {
        called = false;
        number.reset();
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueNumber";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/number/testDevice/uniqueNumber/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueNumber/cmd_t"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueNumber/stat_t"};

void onCommandReceived(HANumeric number, HANumber* caller)
{
    lastCommandCallbackCall.called = true;
    lastCommandCallbackCall.number = number;
    lastCommandCallbackCall.caller = caller;
}

void test_NumberTest_invalid_unique_id(void) {
    prepareTest

    HANumber number(nullptr);
    number.buildSerializerTest();
    HASerializer* serializer = number.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_NumberTest_default_params(void) {
    prepareTest

    HANumber number(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_NumberTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HANumber number(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_NumberTest_command_subscription(void) {
    prepareTest

    HANumber number(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_NumberTest_availability(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HANumber
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueNumber/avty_t"),
        "online",
        true
    );
}

void test_NumberTest_publish_nothing_if_retained(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setRetain(true);
    number.setCurrentState(50);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_NumberTest_publish_state_none(void) {
    prepareTest

    HANumber number(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "None", true);
}

void test_NumberTest_set_state_int8(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    int8_t value = -123;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-123", true);
}

void test_NumberTest_set_state_int16(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    int16_t value = -12345;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-12345", true);
}

void test_NumberTest_set_state_int32(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    int32_t value = -1234567;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-1234567", true);
}

void test_NumberTest_set_state_uint8(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    uint8_t value = 254;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "254", true);
}

void test_NumberTest_set_state_uint16(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    uint16_t value = 65200;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "65200", true);
}

void test_NumberTest_set_state_uint32(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId);
    uint32_t value = 105200;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "105200", true);
}

void test_NumberTest_set_state_float_p1(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId, HANumber::PrecisionP1);
    float value = 25.1f;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "25.1", true);
}

void test_NumberTest_set_state_float_p2(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId, HANumber::PrecisionP2);
    float value = -25.1f;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-25.10", true);
}

void test_NumberTest_set_state_float_p3(void) {
    prepareTest

    mock->connectDummy();
    HANumber number(testUniqueId, HANumber::PrecisionP3);
    float value = -0.333333f;

    TEST_ASSERT_TRUE(number.setState(value));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-0.333", true);
}

void test_NumberTest_current_state_setter_getter_int8(void) {
    prepareTest

    HANumber number(testUniqueId);
    int8_t value = -123;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isInt8());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toInt8());
}

void test_NumberTest_current_state_setter_getter_int16(void) {
    prepareTest

    HANumber number(testUniqueId);
    int16_t value = -1234;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isInt16());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toInt16());
}

void test_NumberTest_current_state_setter_getter_int32(void) {
    prepareTest

    HANumber number(testUniqueId);
    int32_t value = -12345;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isInt32());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toInt32());
}

void test_NumberTest_current_state_setter_getter_uint8(void) {
    prepareTest

    HANumber number(testUniqueId);
    uint8_t value = 254;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isUInt8());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toUInt8());
}

void test_NumberTest_current_state_setter_getter_uint16(void) {
    prepareTest

    HANumber number(testUniqueId);
    uint16_t value = 12345;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isUInt16());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toUInt16());
}

void test_NumberTest_current_state_setter_getter_uint32(void) {
    prepareTest

    HANumber number(testUniqueId);
    uint32_t value = 1234567;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isUInt32());
    TEST_ASSERT_EQUAL(value, number.getCurrentState().toUInt32());
}

void test_NumberTest_current_state_setter_getter_float(void) {
    prepareTest

    HANumber number(testUniqueId, HANumber::PrecisionP1);
    float value = 25.0f;

    number.setCurrentState(value);
    TEST_ASSERT_TRUE(number.getCurrentState().isFloat());
    AHA_ASSERT_NEAR_FLOAT(value, number.getCurrentState().toFloat(), 0.1);
}

void test_NumberTest_name_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_object_id_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_default_entity_id_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setDefaultEntityId("number.test_number");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"def_ent_id\":\"number.test_number\","
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_device_class(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setDeviceClass("testClass");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev_cla\":\"testClass\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_entity_category_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_icon_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_retain_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_optimistic_setter(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_mode_setter_box(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setMode(HANumber::ModeBox);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"mode\":\"box\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_mode_setter_slider(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setMode(HANumber::ModeSlider);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"mode\":\"slider\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_unit_of_measurement_setter(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId);
    number.setUnitOfMeasurement("%");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"unit_of_meas\":\"%\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_empty_unit_of_measurement_is_ignored(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId);
    number.setUnitOfMeasurement("");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_min_setter_p0(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId);
    number.setMin(2);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"min\":2,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_min_setter_p1(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP1);
    number.setMin(2.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**1)}}\","
            "\"min\":2.5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_min_setter_p2(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP2);
    number.setMin(95467.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**2)}}\","
            "\"min\":95467.50,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_min_setter_p3(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP3);
    number.setMin(50.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**3)}}\","
            "\"min\":50.500,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_max_setter_p0(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId);
    number.setMax(2);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"max\":2,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_max_setter_p1(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP1);
    number.setMax(2.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**1)}}\","
            "\"max\":2.5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_max_setter_p2(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP2);
    number.setMax(95467.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**2)}}\","
            "\"max\":95467.50,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_max_setter_p3(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP3);
    number.setMax(50.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**3)}}\","
            "\"max\":50.500,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_step_setter_p0(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId);
    number.setStep(2);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"step\":2,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_step_setter_p1(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP1);
    number.setStep(2.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**1)}}\","
            "\"step\":2.5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_step_setter_p2(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP2);
    number.setStep(0.01);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**2)}}\","
            "\"step\":0.01,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_step_setter_p3(void) {
    initMqttTest(testDeviceId)

    HANumber number(testUniqueId, HANumber::PrecisionP3);
    number.setStep(0.001);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"cmd_tpl\":\"{{int(float(value)*10**3)}}\","
            "\"step\":0.001,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_update_min_max_step_before_connect(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.updateMinMaxStep(1, 99, 5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"min\":1,"
            "\"max\":99,"
            "\"step\":5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_min_can_equal_max(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.setMin(5);
    number.setMax(5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        number,
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"min\":5,"
            "\"max\":5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        )
    );
}

void test_NumberTest_update_min_max_step_republishes_config(void) {
    prepareTest

    HANumber number(testUniqueId);
    mqtt.loop();
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state

    number.updateMinMaxStep(1, 99, 5);

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        2,
        AHATOFSTR(ConfigTopic),
        (
            "{"
            "\"uniq_id\":\"uniqueNumber\","
            "\"min\":1,"
            "\"max\":99,"
            "\"step\":5,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
            "}"
        ),
        true
    );
}

void test_NumberTest_update_min_max_step_republishes_device_discovery_when_enabled(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HANumber number(testUniqueId);
    mqtt.loop();
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());

    number.updateMinMaxStep(1, 99, 5);

    TEST_ASSERT_EQUAL(4, mock->getFlushedMessagesNb());
    MqttMessage* clearedConfig = mock->getFlushedMessages()[2];
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(ConfigTopic), clearedConfig->topic);
    TEST_ASSERT_EQUAL((size_t)1, clearedConfig->bufferSize);
    TEST_ASSERT_TRUE(clearedConfig->retained);
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        3,
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueNumber\":{"
                    "\"p\":\"number\","
                    "\"uniq_id\":\"uniqueNumber\","
                    "\"min\":1,"
                    "\"max\":99,"
                    "\"step\":5,"
                    "\"stat_t\":\"testData/testDevice/uniqueNumber/stat_t\","
                    "\"cmd_t\":\"testData/testDevice/uniqueNumber/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_NumberTest_command_none(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("None"));

    assertCommandCallbackCalled(HANumeric(), &number)
}

void test_NumberTest_command_number_zero(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("0"));


    assertCommandCallbackCalled(HANumeric(0, 0), &number)
}

void test_NumberTest_command_number_unsigned(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("1234"));

    assertCommandCallbackCalled(HANumeric(1234, 0), &number)
}

void test_NumberTest_command_number_signed(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("-1234"));

    assertCommandCallbackCalled(HANumeric(-1234, 0), &number)
}

void test_NumberTest_command_number_float_p1(void) {
    prepareTest

    HANumber number(testUniqueId, HANumber::PrecisionP1);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("-1234"));

    assertCommandCallbackCalled(HANumeric(-123.4f, 1), &number)
}

void test_NumberTest_command_number_float_p2(void) {
    prepareTest

    HANumber number(testUniqueId, HANumber::PrecisionP2);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("-1234"));

    assertCommandCallbackCalled(HANumeric(-12.34f, 2), &number)
}

void test_NumberTest_command_number_float_p3(void) {
    prepareTest

    HANumber number(testUniqueId, HANumber::PrecisionP3);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("-1234"));

    assertCommandCallbackCalled(HANumeric(-1.234f, 3), &number)
}

void test_NumberTest_command_number_invalid(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("abc"));

    assertCommandCallbackNotCalled()
}

void test_NumberTest_different_number_command(void) {
    prepareTest

    HANumber number(testUniqueId);
    number.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueCoverDifferent/cmd_t"),
        F("123")
    );

    assertCommandCallbackNotCalled()
}

