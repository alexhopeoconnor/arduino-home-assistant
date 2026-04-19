#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastAuxStateCallbackCall.reset(); \
    lastPowerCallbackCall.reset(); \
    lastFanModeCallbackCall.reset(); \
    lastSwingModeCallbackCall.reset(); \
    lastModeCallbackCall.reset(); \
    lastTargetTempCallbackCall.reset();

#define assertAuxStateCallbackCalled(expectedState, callerPtr) \
    TEST_ASSERT_TRUE(lastAuxStateCallbackCall.called); \
    TEST_ASSERT_EQUAL(static_cast<bool>(expectedState), lastAuxStateCallbackCall.state); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastAuxStateCallbackCall.caller);

#define assertAuxStateCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastAuxStateCallbackCall.called);

#define assertPowerCallbackCalled(expectedState, callerPtr) \
    TEST_ASSERT_TRUE(lastPowerCallbackCall.called); \
    TEST_ASSERT_EQUAL(static_cast<bool>(expectedState), lastPowerCallbackCall.state); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastPowerCallbackCall.caller);

#define assertPowerCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastPowerCallbackCall.called);

#define assertFanModeCallbackCalled(expectedMode, callerPtr) \
    TEST_ASSERT_TRUE(lastFanModeCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedMode, lastFanModeCallbackCall.mode); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastFanModeCallbackCall.caller);

#define assertFanModeCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastFanModeCallbackCall.called);

#define assertSwingModeCallbackCalled(expectedMode, callerPtr) \
    TEST_ASSERT_TRUE(lastSwingModeCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedMode, lastSwingModeCallbackCall.mode); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastSwingModeCallbackCall.caller);

#define assertSwingModeCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastSwingModeCallbackCall.called);

#define assertModeCallbackCalled(expectedMode, callerPtr) \
    TEST_ASSERT_TRUE(lastModeCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedMode, lastModeCallbackCall.mode); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastModeCallbackCall.caller);

#define assertModeCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastModeCallbackCall.called);

#define assertTargetTempCallbackCalled(expectedTemperature, callerPtr) \
    TEST_ASSERT_TRUE(lastTargetTempCallbackCall.called); \
    TEST_ASSERT_TRUE(expectedTemperature == lastTargetTempCallbackCall.temperature); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastTargetTempCallbackCall.caller);

#define assertTargetTempCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastTargetTempCallbackCall.called);

struct AuxStateCallback {
    bool called = false;
    bool state = false;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        state = false;
        caller = nullptr;
    }
};

struct PowerCallback {
    bool called = false;
    bool state = false;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        state = false;
        caller = nullptr;
    }
};

struct FanModeCallback {
    bool called = false;
    HAHVAC::FanMode mode = HAHVAC::UnknownFanMode;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        mode = HAHVAC::UnknownFanMode;
        caller = nullptr;
    }
};

struct SwingModeCallback {
    bool called = false;
    HAHVAC::SwingMode mode = HAHVAC::UnknownSwingMode;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        mode = HAHVAC::UnknownSwingMode;
        caller = nullptr;
    }
};

struct ModeCallback {
    bool called = false;
    HAHVAC::Mode mode = HAHVAC::UnknownMode;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        mode = HAHVAC::UnknownMode;
        caller = nullptr;
    }
};

struct TargetTempCallback {
    bool called = false;
    HANumeric temperature;
    HAHVAC* caller = nullptr;

    void reset() {
        called = false;
        temperature.reset();
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueHVAC";
static AuxStateCallback lastAuxStateCallbackCall;
static PowerCallback lastPowerCallbackCall;
static FanModeCallback lastFanModeCallbackCall;
static SwingModeCallback lastSwingModeCallbackCall;
static ModeCallback lastModeCallbackCall;
static TargetTempCallback lastTargetTempCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/climate/testDevice/uniqueHVAC/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char CurrentTemperatureTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/curr_temp_t"};
const char ActionTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/act_t"};
const char AuxStateTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/aux_stat_t"};
const char AuxCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/aux_cmd_t"};
const char PowerCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/pow_cmd_t"};
const char FanModeStateTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/fan_mode_stat_t"};
const char FanModeCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/fan_mode_cmd_t"};
const char SwingModeStateTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/swing_mode_stat_t"};
const char SwingModeCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/swing_mode_cmd_t"};
const char ModeStateTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/mode_stat_t"};
const char ModeCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/mode_cmd_t"};
const char TemperatureStateTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/temp_stat_t"};
const char TemperatureCommandTopic[] PROGMEM = {"testData/testDevice/uniqueHVAC/temp_cmd_t"};

void onAuxStateCommandReceived(bool state, HAHVAC* caller)
{
    lastAuxStateCallbackCall.called = true;
    lastAuxStateCallbackCall.state = state;
    lastAuxStateCallbackCall.caller = caller;
}

void onPowerCommandReceived(bool state, HAHVAC* caller)
{
    lastPowerCallbackCall.called = true;
    lastPowerCallbackCall.state = state;
    lastPowerCallbackCall.caller = caller;
}

void onFanModeCommandReceived(HAHVAC::FanMode mode, HAHVAC* caller)
{
    lastFanModeCallbackCall.called = true;
    lastFanModeCallbackCall.mode = mode;
    lastFanModeCallbackCall.caller = caller;
}

void onSwingModeCommandReceived(HAHVAC::SwingMode mode, HAHVAC* caller)
{
    lastSwingModeCallbackCall.called = true;
    lastSwingModeCallbackCall.mode = mode;
    lastSwingModeCallbackCall.caller = caller;
}

void onModeCommandReceived(HAHVAC::Mode mode, HAHVAC* caller)
{
    lastModeCallbackCall.called = true;
    lastModeCallbackCall.mode = mode;
    lastModeCallbackCall.caller = caller;
}

void onTargetTemperatureCommandReceived(HANumeric temperature, HAHVAC* caller)
{
    lastTargetTempCallbackCall.called = true;
    lastTargetTempCallbackCall.temperature = temperature;
    lastTargetTempCallbackCall.caller = caller;
}

void test_HVACTest_invalid_unique_id(void) {
    prepareTest

    HAHVAC hvac(nullptr);
    hvac.buildSerializerTest();
    HASerializer* serializer = hvac.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_HVACTest_default_params(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAHVAC hvac(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueHVAC\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HAHVAC hvac(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueHVAC\":{"
                    "\"p\":\"climate\","
                    "\"uniq_id\":\"uniqueHVAC\","
                    "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_HVACTest_config_with_action(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"act_t\":\"testData/testDevice/uniqueHVAC/act_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_aux(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"aux_cmd_t\":\"testData/testDevice/uniqueHVAC/aux_cmd_t\","
            "\"aux_stat_t\":\"testData/testDevice/uniqueHVAC/aux_stat_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + aux state
}

void test_HVACTest_config_with_power(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::PowerFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"pow_cmd_t\":\"testData/testDevice/uniqueHVAC/pow_cmd_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_fan(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_swing(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"swing_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/swing_mode_cmd_t\","
            "\"swing_mode_stat_t\":\"testData/testDevice/uniqueHVAC/swing_mode_stat_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_modes(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_target_temperature_p1(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_cmd_t\":\"testData/testDevice/uniqueHVAC/temp_cmd_t\","
            "\"temp_stat_t\":\"testData/testDevice/uniqueHVAC/temp_stat_t\","
            "\"temp_cmd_tpl\":\"{{int(float(value)*10**1)}}\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_target_temperature_p2(void) {
    prepareTest

    HAHVAC hvac(
        testUniqueId,
        HAHVAC::TargetTemperatureFeature,
        HAHVAC::PrecisionP2
    );
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_cmd_t\":\"testData/testDevice/uniqueHVAC/temp_cmd_t\","
            "\"temp_stat_t\":\"testData/testDevice/uniqueHVAC/temp_stat_t\","
            "\"temp_cmd_tpl\":\"{{int(float(value)*10**2)}}\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_config_with_target_temperature_p3(void) {
    prepareTest

    HAHVAC hvac(
        testUniqueId,
        HAHVAC::TargetTemperatureFeature,
        HAHVAC::PrecisionP3
    );
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_cmd_t\":\"testData/testDevice/uniqueHVAC/temp_cmd_t\","
            "\"temp_stat_t\":\"testData/testDevice/uniqueHVAC/temp_stat_t\","
            "\"temp_cmd_tpl\":\"{{int(float(value)*10**3)}}\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_aux_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(AuxCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_power_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::PowerFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(PowerCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_fan_mode_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(FanModeCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_swing_mode_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(SwingModeCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_mode_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(ModeCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_target_temperature_command_subscription(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(TemperatureCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_HVACTest_availability(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAHVAC
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueHVAC/avty_t"),
        "online",
        true
    );
}

void test_HVACTest_name_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueHVAC\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_object_id_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueHVAC\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_default_entity_id_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setDefaultEntityId("climate.test_hvac");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"def_ent_id\":\"climate.test_hvac\","
            "\"uniq_id\":\"uniqueHVAC\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_entity_category_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"ent_cat\":\"diagnostic\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_icon_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"ic\":\"testIcon\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_retain_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"ret\":true,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_temperature_unit_c_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setTemperatureUnit(HAHVAC::CelsiusUnit);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_unit\":\"C\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_temperature_unit_f_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setTemperatureUnit(HAHVAC::FahrenheitUnit);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_unit\":\"F\","
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_min_temp_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setMinTemp(25.555);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"min_temp\":25.5,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_min_temp_setter_p2(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::DefaultFeatures, HAHVAC::PrecisionP2);
    hvac.setMinTemp(25.555);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"min_temp\":25.55,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_max_temp_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setMaxTemp(25.555);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"max_temp\":25.5,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_max_temp_setter_p2(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::DefaultFeatures, HAHVAC::PrecisionP2);
    hvac.setMaxTemp(25.555);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"max_temp\":25.55,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_temp_step_setter(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setTempStep(0.5);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_step\":0.5,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_temp_step_setter_p2(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::DefaultFeatures, HAHVAC::PrecisionP2);
    hvac.setTempStep(0.05);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"temp_step\":0.05,"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
}

void test_HVACTest_fan_modes_setter_auto_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setFanModes(HAHVAC::AutoFanMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"fan_modes\":[\"auto\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_fan_modes_setter_low_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setFanModes(HAHVAC::LowFanMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"fan_modes\":[\"low\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_fan_modes_setter_medium_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setFanModes(HAHVAC::MediumFanMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"fan_modes\":[\"medium\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_fan_modes_setter_high_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setFanModes(HAHVAC::HighFanMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"fan_modes\":[\"high\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_fan_modes_setter_mixed(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setFanModes(HAHVAC::AutoFanMode | HAHVAC::MediumFanMode | HAHVAC::HighFanMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"fan_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/fan_mode_cmd_t\","
            "\"fan_mode_stat_t\":\"testData/testDevice/uniqueHVAC/fan_mode_stat_t\","
            "\"fan_modes\":[\"auto\",\"medium\",\"high\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_swing_modes_setter_on_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.setSwingModes(HAHVAC::OnSwingMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"swing_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/swing_mode_cmd_t\","
            "\"swing_mode_stat_t\":\"testData/testDevice/uniqueHVAC/swing_mode_stat_t\","
            "\"swing_modes\":[\"on\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_swing_modes_setter_off_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.setSwingModes(HAHVAC::OffSwingMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"swing_mode_cmd_t\":\"testData/testDevice/uniqueHVAC/swing_mode_cmd_t\","
            "\"swing_mode_stat_t\":\"testData/testDevice/uniqueHVAC/swing_mode_stat_t\","
            "\"swing_modes\":[\"off\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_auto_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::AutoMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"auto\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_off_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::OffMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"off\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_cool_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::CoolMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"cool\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_heat_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::HeatMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"heat\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_dry_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::DryMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"dry\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_fan_only_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::FanOnlyMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"fan_only\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_modes_setter_mixed(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setModes(HAHVAC::AutoMode | HAHVAC::HeatMode | HAHVAC::DryMode);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        hvac,
        (
            "{"
            "\"uniq_id\":\"uniqueHVAC\","
            "\"mode_cmd_t\":\"testData/testDevice/uniqueHVAC/mode_cmd_t\","
            "\"mode_stat_t\":\"testData/testDevice/uniqueHVAC/mode_stat_t\","
            "\"modes\":[\"auto\",\"heat\",\"dry\"],"
            "\"curr_temp_t\":\"testData/testDevice/uniqueHVAC/curr_temp_t\","
            "\"dev\":{\"ids\":\"testDevice\"}"
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // config
}

void test_HVACTest_publish_nothing_if_retained(void) {
    prepareTest

    HAHVAC hvac(
        testUniqueId,
        HAHVAC::ActionFeature |
            HAHVAC::AuxHeatingFeature |
            HAHVAC::PowerFeature |
            HAHVAC::FanFeature |
            HAHVAC::SwingFeature |
            HAHVAC::ModesFeature
    );
    hvac.setRetain(true);
    hvac.setCurrentTemperature(21.5f);
    hvac.setCurrentAction(HAHVAC::CoolingAction);
    hvac.setCurrentFanMode(HAHVAC::AutoFanMode);
    hvac.setCurrentSwingMode(HAHVAC::OnSwingMode);
    hvac.setCurrentMode(HAHVAC::HeatMode);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_HVACTest_publish_current_temperature_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.setCurrentCurrentTemperature(21.5f);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(CurrentTemperatureTopic), "21.5", true);
}

void test_HVACTest_publish_current_temperature(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId);

    TEST_ASSERT_TRUE(hvac.setCurrentTemperature(21.5f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(CurrentTemperatureTopic), "21.5", true);
}

void test_HVACTest_publish_current_temperature_p2(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::DefaultFeatures, HAHVAC::PrecisionP2);

    TEST_ASSERT_TRUE(hvac.setCurrentTemperature(21.5f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(CurrentTemperatureTopic), "21.50", true);
}

void test_HVACTest_publish_current_temperature_p3(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::DefaultFeatures, HAHVAC::PrecisionP3);

    TEST_ASSERT_TRUE(hvac.setCurrentTemperature(21.555f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(CurrentTemperatureTopic), "21.555", true);
}

void test_HVACTest_publish_current_temperature_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId);
    hvac.setCurrentCurrentTemperature(21.5f);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(hvac.setCurrentTemperature(21.5f));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_current_temperature_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId);
    hvac.setCurrentCurrentTemperature(21.5f);

    TEST_ASSERT_TRUE(hvac.setCurrentTemperature(21.5f, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(CurrentTemperatureTopic), "21.5", true);
}

void test_HVACTest_publish_action_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);
    hvac.setCurrentAction(HAHVAC::OffAction);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(ActionTopic), "off", true);
}

void test_HVACTest_publish_action_off(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::OffAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "off", true);
}

void test_HVACTest_publish_action_heating(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::HeatingAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "heating", true);
}

void test_HVACTest_publish_action_cooling(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::CoolingAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "cooling", true);
}

void test_HVACTest_publish_action_drying(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::DryingAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "drying", true);
}

void test_HVACTest_publish_action_idle(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::IdleAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "idle", true);
}

void test_HVACTest_publish_action_fan(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::FanAction));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "fan", true);
}

void test_HVACTest_publish_action_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);
    hvac.setCurrentAction(HAHVAC::FanAction);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::FanAction));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_action_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ActionFeature);
    hvac.setCurrentAction(HAHVAC::FanAction);

    TEST_ASSERT_TRUE(hvac.setAction(HAHVAC::FanAction, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ActionTopic), "fan", true);
}

void test_HVACTest_publish_aux_state_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    hvac.setCurrentAuxState(true);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(AuxStateTopic), "ON", true);
}

void test_HVACTest_publish_aux_state_on(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);

    TEST_ASSERT_TRUE(hvac.setAuxState(true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AuxStateTopic), "ON", true);
}

void test_HVACTest_publish_aux_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    hvac.setCurrentAuxState(true);

    TEST_ASSERT_TRUE(hvac.setAuxState(true));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_aux_state_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    hvac.setCurrentAuxState(false);

    TEST_ASSERT_TRUE(hvac.setAuxState(false, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AuxStateTopic), "OFF", true);
}

void test_HVACTest_publish_fan_mode_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setCurrentFanMode(HAHVAC::HighFanMode);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(FanModeStateTopic), "high", true);
}

void test_HVACTest_publish_fan_mode_auto(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::AutoFanMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(FanModeStateTopic), "auto", true);
}

void test_HVACTest_publish_fan_mode_low(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::LowFanMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(FanModeStateTopic), "low", true);
}

void test_HVACTest_publish_fan_mode_medium(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::MediumFanMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(FanModeStateTopic), "medium", true);
}

void test_HVACTest_publish_fan_mode_high(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::HighFanMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(FanModeStateTopic), "high", true);
}

void test_HVACTest_publish_fan_mode_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setCurrentFanMode(HAHVAC::HighFanMode);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::HighFanMode));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_fan_mode_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.setCurrentFanMode(HAHVAC::HighFanMode);

    TEST_ASSERT_TRUE(hvac.setFanMode(HAHVAC::HighFanMode, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(FanModeStateTopic), "high", true);
}

void test_HVACTest_publish_swing_mode_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.setCurrentSwingMode(HAHVAC::OnSwingMode);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(SwingModeStateTopic), "on", true);
}

void test_HVACTest_publish_swing_mode_on(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);

    TEST_ASSERT_TRUE(hvac.setSwingMode(HAHVAC::OnSwingMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SwingModeStateTopic), "on", true);
}

void test_HVACTest_publish_swing_mode_off(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);

    TEST_ASSERT_TRUE(hvac.setSwingMode(HAHVAC::OffSwingMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SwingModeStateTopic), "off", true);
}

void test_HVACTest_publish_swing_mode_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.setCurrentSwingMode(HAHVAC::OnSwingMode);

    TEST_ASSERT_TRUE(hvac.setSwingMode(HAHVAC::OnSwingMode));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_swing_mode_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.setCurrentSwingMode(HAHVAC::OnSwingMode);

    TEST_ASSERT_TRUE(hvac.setSwingMode(HAHVAC::OnSwingMode, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(SwingModeStateTopic), "on", true);
}

void test_HVACTest_publish_mode_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.setCurrentMode(HAHVAC::HeatMode);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(ModeStateTopic), "heat", true);
}

void test_HVACTest_publish_mode_auto(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::AutoMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "auto", true);
}

void test_HVACTest_publish_mode_off(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::OffMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "off", true);
}

void test_HVACTest_publish_mode_cool(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::CoolMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "cool", true);
}

void test_HVACTest_publish_mode_heat(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::HeatMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "heat", true);
}

void test_HVACTest_publish_mode_dry(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::DryMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "dry", true);
}

void test_HVACTest_publish_mode_fan_only(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);

    TEST_ASSERT_TRUE(hvac.setMode(HAHVAC::FanOnlyMode));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ModeStateTopic), "fan_only", true);
}

void test_HVACTest_publish_target_temperature_on_connect(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.setCurrentTargetTemperature(21.5f);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(TemperatureStateTopic), "21.5", true);
}

void test_HVACTest_publish_target_temperature(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);

    TEST_ASSERT_TRUE(hvac.setTargetTemperature(21.5f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(TemperatureStateTopic), "21.5", true);
}

void test_HVACTest_publish_target_temperature_p2(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature, HAHVAC::PrecisionP2);

    TEST_ASSERT_TRUE(hvac.setTargetTemperature(21.5f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(TemperatureStateTopic), "21.50", true);
}

void test_HVACTest_publish_target_temperature_p3(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature, HAHVAC::PrecisionP3);

    TEST_ASSERT_TRUE(hvac.setTargetTemperature(21.555f));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(TemperatureStateTopic), "21.555", true);
}

void test_HVACTest_publish_target_temperature_debounce(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.setCurrentTargetTemperature(21.5f);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(hvac.setTargetTemperature(21.5f));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_HVACTest_publish_target_temperature_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.setCurrentTargetTemperature(21.5f);

    TEST_ASSERT_TRUE(hvac.setTargetTemperature(21.5f, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(TemperatureStateTopic), "21.5", true);
}

void test_HVACTest_aux_state_command_on(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    hvac.onAuxStateCommand(onAuxStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(AuxCommandTopic), F("ON"));

    assertAuxStateCallbackCalled(true, &hvac)
}

void test_HVACTest_aux_state_command_off(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::AuxHeatingFeature);
    hvac.onAuxStateCommand(onAuxStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(AuxCommandTopic), F("OFF"));

    assertAuxStateCallbackCalled(false, &hvac)
}

void test_HVACTest_aux_command_different_fan(void) {
    prepareTest

    HAHVAC hvac(testUniqueId);
    hvac.onAuxStateCommand(onAuxStateCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueHVACDifferent/act"),
        F("ON")
    );

    assertAuxStateCallbackNotCalled()
}

void test_HVACTest_power_command_on(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::PowerFeature);
    hvac.onPowerCommand(onPowerCommandReceived);
    mock->fakeMessage(AHATOFSTR(PowerCommandTopic), F("ON"));

    assertPowerCallbackCalled(true, &hvac)
}

void test_HVACTest_power_command_off(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::PowerFeature);
    hvac.onPowerCommand(onPowerCommandReceived);
    mock->fakeMessage(AHATOFSTR(PowerCommandTopic), F("OFF"));

    assertPowerCallbackCalled(false, &hvac)
}

void test_HVACTest_power_command_different(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::PowerFeature);
    hvac.onPowerCommand(onPowerCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueHVACDifferent/pow_cmd_t"),
        F("ON")
    );

    assertPowerCallbackNotCalled()
}

void test_HVACTest_fan_mode_command_auto(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(FanModeCommandTopic), F("auto"));

    assertFanModeCallbackCalled(HAHVAC::AutoFanMode, &hvac)
}

void test_HVACTest_fan_mode_command_low(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(FanModeCommandTopic), F("low"));

    assertFanModeCallbackCalled(HAHVAC::LowFanMode, &hvac)
}

void test_HVACTest_fan_mode_command_medium(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(FanModeCommandTopic), F("medium"));

    assertFanModeCallbackCalled(HAHVAC::MediumFanMode, &hvac)
}

void test_HVACTest_fan_mode_command_high(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(FanModeCommandTopic), F("high"));

    assertFanModeCallbackCalled(HAHVAC::HighFanMode, &hvac)
}

void test_HVACTest_fan_mode_command_invalid(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(FanModeCommandTopic), F("INVALID"));

    assertFanModeCallbackNotCalled()
}

void test_HVACTest_fan_mode_command_different(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::FanFeature);
    hvac.onFanModeCommand(onFanModeCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueHVACDifferent/fan_mode_cmd_t"),
        F("auto")
    );

    assertFanModeCallbackNotCalled()
}

void test_HVACTest_swing_mode_command_on(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.onSwingModeCommand(onSwingModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(SwingModeCommandTopic), F("on"));

    assertSwingModeCallbackCalled(HAHVAC::OnSwingMode, &hvac)
}

void test_HVACTest_swing_mode_command_off(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.onSwingModeCommand(onSwingModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(SwingModeCommandTopic), F("off"));

    assertSwingModeCallbackCalled(HAHVAC::OffSwingMode, &hvac)
}

void test_HVACTest_swing_mode_command_invalid(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.onSwingModeCommand(onSwingModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(SwingModeCommandTopic), F("INVALID"));

    assertSwingModeCallbackNotCalled()
}

void test_HVACTest_swing_mode_command_different(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::SwingFeature);
    hvac.onSwingModeCommand(onSwingModeCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueHVACDifferent/swing_mode_cmd_t"),
        F("on")
    );

    assertSwingModeCallbackNotCalled()
}

void test_HVACTest_mode_command_auto(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("auto"));

    assertModeCallbackCalled(HAHVAC::AutoMode, &hvac)
}

void test_HVACTest_mode_command_off(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("off"));

    assertModeCallbackCalled(HAHVAC::OffMode, &hvac)
}

void test_HVACTest_mode_command_cool(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("cool"));

    assertModeCallbackCalled(HAHVAC::CoolMode, &hvac)
}

void test_HVACTest_mode_command_heat(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("heat"));

    assertModeCallbackCalled(HAHVAC::HeatMode, &hvac)
}

void test_HVACTest_mode_command_dry(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("dry"));

    assertModeCallbackCalled(HAHVAC::DryMode, &hvac)
}

void test_HVACTest_mode_command_fan_only(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::ModesFeature);
    hvac.onModeCommand(onModeCommandReceived);
    mock->fakeMessage(AHATOFSTR(ModeCommandTopic), F("fan_only"));

    assertModeCallbackCalled(HAHVAC::FanOnlyMode, &hvac)
}

void test_HVACTest_target_temperature_command_p1(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.onTargetTemperatureCommand(onTargetTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(TemperatureCommandTopic), F("215"));

    assertTargetTempCallbackCalled(HANumeric(21.5f, 1), &hvac)
}

void test_HVACTest_target_temperature_command_p2(void) {
    prepareTest

    HAHVAC hvac(
        testUniqueId,
        HAHVAC::TargetTemperatureFeature,
        HAHVAC::PrecisionP2
    );
    hvac.onTargetTemperatureCommand(onTargetTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(TemperatureCommandTopic), F("215"));

    assertTargetTempCallbackCalled(HANumeric(2.15f, 2), &hvac)
}

void test_HVACTest_target_temperature_command_p3(void) {
    prepareTest

    HAHVAC hvac(
        testUniqueId,
        HAHVAC::TargetTemperatureFeature,
        HAHVAC::PrecisionP3
    );
    hvac.onTargetTemperatureCommand(onTargetTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(TemperatureCommandTopic), F("215"));

    assertTargetTempCallbackCalled(HANumeric(0.215f, 3), &hvac)
}

void test_HVACTest_target_temperature_command_invalid(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.onTargetTemperatureCommand(onTargetTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(TemperatureCommandTopic), F("21.5"));

    assertTargetTempCallbackNotCalled()
}

void test_HVACTest_target_temperature_command_different(void) {
    prepareTest

    HAHVAC hvac(testUniqueId, HAHVAC::TargetTemperatureFeature);
    hvac.onTargetTemperatureCommand(onTargetTemperatureCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueHVACDifferent/temp_cmd_t"),
        F("215")
    );

    assertTargetTempCallbackNotCalled()
}

