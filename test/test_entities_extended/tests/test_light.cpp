#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastStateCallbackCall.reset(); \
    lastBrightnessCallbackCall.reset(); \
    lastColorTempCallbackCall.reset(); \
    lastRGBColorCallbackCall.reset();

#define assertStateCallbackCalled(expectedState, callerPtr) \
    TEST_ASSERT_TRUE(lastStateCallbackCall.called); \
    TEST_ASSERT_EQUAL(static_cast<bool>(expectedState), lastStateCallbackCall.state); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastStateCallbackCall.caller);

#define assertStateCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastStateCallbackCall.called);

#define assertBrightnessCallbackCalled(expectedBrightness, callerPtr) \
    TEST_ASSERT_TRUE(lastBrightnessCallbackCall.called); \
    TEST_ASSERT_EQUAL((uint8_t)expectedBrightness, lastBrightnessCallbackCall.brightness); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastBrightnessCallbackCall.caller);

#define assertBrightnessCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastBrightnessCallbackCall.called);

#define assertColorTempCallbackCalled(expectedColorTemp, callerPtr) \
    TEST_ASSERT_TRUE(lastColorTempCallbackCall.called); \
    TEST_ASSERT_EQUAL((uint16_t)expectedColorTemp, lastColorTempCallbackCall.temperature); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastColorTempCallbackCall.caller);

#define assertColorTempCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastColorTempCallbackCall.called);

#define assertRGBColorCallbackCalled(expectedColor, callerPtr) \
    TEST_ASSERT_TRUE(lastRGBColorCallbackCall.called); \
    TEST_ASSERT_TRUE(expectedColor == lastRGBColorCallbackCall.color); \
    TEST_ASSERT_TRUE(lastRGBColorCallbackCall.color.isSet); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastRGBColorCallbackCall.caller);

#define assertRGBColorCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastRGBColorCallbackCall.called);

struct StateCallback {
    bool called = false;
    bool state = false;
    HALight* caller = nullptr;

    void reset() {
        called = false;
        state = false;
        caller = nullptr;
    }
};

struct BrightnessCallback {
    bool called = false;
    uint8_t brightness = 0;
    HALight* caller = nullptr;

    void reset() {
        called = false;
        brightness = 0;
        caller = nullptr;
    }
};

struct ColorTemperatureCallback {
    bool called = false;
    uint16_t temperature = 0;
    HALight* caller = nullptr;

    void reset() {
        called = false;
        temperature = 0;
        caller = nullptr;
    }
};

struct RGBCommandCallback {
    bool called = false;
    HALight::RGBColor color = HALight::RGBColor();
    HALight* caller = nullptr;

    void reset() {
        called = false;
        color = HALight::RGBColor();
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueLight";
static StateCallback lastStateCallbackCall;
static BrightnessCallback lastBrightnessCallbackCall;
static ColorTemperatureCallback lastColorTempCallbackCall;
static RGBCommandCallback lastRGBColorCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/light/testDevice/uniqueLight/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueLight/stat_t"};
const char BrightnessStateTopic[] PROGMEM = {"testData/testDevice/uniqueLight/bri_stat_t"};
const char ColorTemperatureStateTopic[] PROGMEM = {"testData/testDevice/uniqueLight/clr_temp_stat_t"};
const char StateCommandTopic[] PROGMEM = {"testData/testDevice/uniqueLight/cmd_t"};
const char BrightnessCommandTopic[] PROGMEM = {"testData/testDevice/uniqueLight/bri_cmd_t"};
const char ColorTemperatureCommandTopic[] PROGMEM = {"testData/testDevice/uniqueLight/clr_temp_cmd_t"};
const char RGBCommandTopic[] PROGMEM = {"testData/testDevice/uniqueLight/rgb_cmd_t"};
const char RGBStateTopic[] PROGMEM = {"testData/testDevice/uniqueLight/rgb_stat_t"};

void onStateCommandReceived(bool state, HALight* caller)
{
    lastStateCallbackCall.called = true;
    lastStateCallbackCall.state = state;
    lastStateCallbackCall.caller = caller;
}

void onBrightnessCommandReceived(uint8_t brightness, HALight* caller)
{
    lastBrightnessCallbackCall.called = true;
    lastBrightnessCallbackCall.brightness = brightness;
    lastBrightnessCallbackCall.caller = caller;
}

void onColorTemperatureCommandReceived(uint16_t temperature, HALight* caller)
{
    lastColorTempCallbackCall.called = true;
    lastColorTempCallbackCall.temperature = temperature;
    lastColorTempCallbackCall.caller = caller;
}

void onRGBColorCommand(HALight::RGBColor color, HALight* caller)
{
    lastRGBColorCallbackCall.called = true;
    lastRGBColorCallbackCall.color = color;
    lastRGBColorCallbackCall.caller = caller;
}

void test_LightTest_invalid_unique_id(void) {
    prepareTest

    HALight light(nullptr);
    light.buildSerializerTest();
    HASerializer* serializer = light.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_LightTest_default_params(void) {
    prepareTest

    HALight light(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_LightTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HALight light(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_LightTest_default_params_with_brightness(void) {
    prepareTest

    HALight light(testUniqueId, HALight::BrightnessFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"bri_stat_t\":\"testData/testDevice/uniqueLight/bri_stat_t\","
            "\"bri_cmd_t\":\"testData/testDevice/uniqueLight/bri_cmd_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb()); // config + default state + default brightness
}

void test_LightTest_default_params_with_color_temp(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"clr_temp_stat_t\":\"testData/testDevice/uniqueLight/clr_temp_stat_t\","
            "\"clr_temp_cmd_t\":\"testData/testDevice/uniqueLight/clr_temp_cmd_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );

    // config + default state + default color temp
    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
}

void test_LightTest_default_params_with_rgb(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"rgb_cmd_t\":\"testData/testDevice/uniqueLight/rgb_cmd_t\","
            "\"rgb_stat_t\":\"testData/testDevice/uniqueLight/rgb_stat_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb()); // config + default state
}

void test_LightTest_default_params_with_brightness_and_color_temp(void) {
    prepareTest

    HALight light(
        testUniqueId,
        HALight::BrightnessFeature | HALight::ColorTemperatureFeature
    );
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"bri_stat_t\":\"testData/testDevice/uniqueLight/bri_stat_t\","
            "\"bri_cmd_t\":\"testData/testDevice/uniqueLight/bri_cmd_t\","
            "\"clr_temp_stat_t\":\"testData/testDevice/uniqueLight/clr_temp_stat_t\","
            "\"clr_temp_cmd_t\":\"testData/testDevice/uniqueLight/clr_temp_cmd_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );

    // config + default state + default brightness + default color temp
    TEST_ASSERT_EQUAL(4, mock->getFlushedMessagesNb());
}

void test_LightTest_state_command_subscription(void) {
    prepareTest

    HALight light(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(StateCommandTopic),
        mock->getSubscriptions()[0]->topic);
}

void test_LightTest_brightness_command_subscription(void) {
    prepareTest

    HALight light(testUniqueId, HALight::BrightnessFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(BrightnessCommandTopic),
        mock->getSubscriptions()[1]->topic);
}

void test_LightTest_color_temperature_command_subscription(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(ColorTemperatureCommandTopic),
        mock->getSubscriptions()[1]->topic);
}

void test_LightTest_rgb_command_subscription(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(
        AHATOFSTR(RGBCommandTopic),
        mock->getSubscriptions()[1]->topic);
}

void test_LightTest_availability(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HALight
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueLight/avty_t"),
        "online",
        true
    );
}

void test_LightTest_publish_last_known_state(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setCurrentState(true);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "ON", true);
}

void test_LightTest_publish_last_known_brightness(void) {
    prepareTest

    HALight light(testUniqueId, HALight::BrightnessFeature);
    light.setCurrentBrightness(50);
    mqtt.loop();

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 2, AHATOFSTR(BrightnessStateTopic), "50", true);
}

void test_LightTest_publish_last_known_color_temperature(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.setCurrentColorTemperature(200);
    mqtt.loop();

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 2, AHATOFSTR(ColorTemperatureStateTopic), "200", true);
}

void test_LightTest_publish_last_known_rgb_color(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.setCurrentRGBColor(HALight::RGBColor(255,123,15));
    mqtt.loop();

    TEST_ASSERT_EQUAL(3, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 2, AHATOFSTR(RGBStateTopic), "255,123,15", true);
}

void test_LightTest_publish_nothing_if_retained(void) {
    prepareTest

    HALight light(testUniqueId, HALight::BrightnessFeature);
    light.setRetain(true);
    light.setCurrentState(true);
    light.setCurrentBrightness(50);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_LightTest_name_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_object_id_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_icon_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_retain_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_optimistic_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_brightness_scale_setter(void) {
    prepareTest

    HALight light(testUniqueId, HALight::BrightnessFeature);
    light.setBrightnessScale(10);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"bri_stat_t\":\"testData/testDevice/uniqueLight/bri_stat_t\","
            "\"bri_cmd_t\":\"testData/testDevice/uniqueLight/bri_cmd_t\","
            "\"bri_scl\":10,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_brightness_scale_feature_disabled(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setBrightnessScale(10);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_color_temperature_range_setter(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.setMinMireds(50);
    light.setMaxMireds(600);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"clr_temp_stat_t\":\"testData/testDevice/uniqueLight/clr_temp_stat_t\","
            "\"clr_temp_cmd_t\":\"testData/testDevice/uniqueLight/clr_temp_cmd_t\","
            "\"min_mirs\":50,"
            "\"max_mirs\":600,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_color_temperature_range_setter_feature_disabled(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setMinMireds(50);
    light.setMaxMireds(600);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        light,
        (
            "{"
            "\"uniq_id\":\"uniqueLight\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLight/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLight/cmd_t\""
            "}"
        )
    );
}

void test_LightTest_current_state_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setCurrentState(true);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_TRUE(light.getCurrentState());
}

void test_LightTest_current_brightness_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setCurrentBrightness(50);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL((uint8_t)50, light.getCurrentBrightness());
}

void test_LightTest_current_color_temperature_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setCurrentColorTemperature(50);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL((uint16_t)50, light.getCurrentColorTemperature());
}

void test_LightTest_current_rgb_color_setter(void) {
    prepareTest

    HALight light(testUniqueId);
    light.setCurrentRGBColor(HALight::RGBColor(255,123,111));

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_TRUE(HALight::RGBColor(255,123,111) == light.getCurrentRGBColor());
}

void test_LightTest_publish_state(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId);

    TEST_ASSERT_TRUE(light.setState(true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
}

void test_LightTest_publish_state_debounce(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId);
    light.setCurrentState(true);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_TRUE(light.setState(true));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_state_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId);
    light.setCurrentState(true);

    TEST_ASSERT_TRUE(light.setState(true, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
}

void test_LightTest_publish_nothing_if_brightness_feature_is_disabled(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId);

    TEST_ASSERT_FALSE(light.setBrightness(50));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_brightness(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::BrightnessFeature);

    TEST_ASSERT_TRUE(light.setBrightness(50));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(BrightnessStateTopic), "50", true);
}

void test_LightTest_publish_brightness_debounce(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::BrightnessFeature);
    light.setCurrentBrightness(50);

    // it shouldn't publish data if value doesn't change
    TEST_ASSERT_TRUE(light.setBrightness(50));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_brightness_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::BrightnessFeature);
    light.setCurrentBrightness(50);

    TEST_ASSERT_TRUE(light.setBrightness(50, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(BrightnessStateTopic), "50", true);
}

void test_LightTest_publish_nothing_if_color_temperature_feature_is_disabled(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId);

    TEST_ASSERT_FALSE(light.setColorTemperature(200));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_color_temperature(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::ColorTemperatureFeature);

    TEST_ASSERT_TRUE(light.setColorTemperature(200));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ColorTemperatureStateTopic), "200", true);
}

void test_LightTest_publish_color_temperature_debounce(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.setCurrentColorTemperature(200);

    // it shouldn't publish data if value doesn't change
    TEST_ASSERT_TRUE(light.setColorTemperature(200));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_color_temperature_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.setCurrentColorTemperature(200);

    TEST_ASSERT_TRUE(light.setColorTemperature(200, true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(ColorTemperatureStateTopic), "200", true);
}

void test_LightTest_publish_rgb_color(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::RGBFeature);

    TEST_ASSERT_TRUE(light.setRGBColor(HALight::RGBColor(255,123,111)));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(RGBStateTopic), "255,123,111", true);
}

void test_LightTest_publish_rgb_color_debounce(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::RGBFeature);
    light.setCurrentRGBColor(HALight::RGBColor(255,123,111));

    // it shouldn't publish data if value doesn't change
    TEST_ASSERT_TRUE(light.setRGBColor(HALight::RGBColor(255,123,111)));
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_LightTest_publish_rgb_color_debounce_skip(void) {
    prepareTest

    mock->connectDummy();
    HALight light(testUniqueId, HALight::RGBFeature);
    light.setCurrentRGBColor(HALight::RGBColor(255,123,111));

    TEST_ASSERT_TRUE(light.setRGBColor(HALight::RGBColor(255,123,111), true));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(RGBStateTopic), "255,123,111", true);
}

void test_LightTest_state_command_on(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(StateCommandTopic), F("ON"));

    assertStateCallbackCalled(true, &light)
}

void test_LightTest_state_command_off(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(AHATOFSTR(StateCommandTopic), F("OFF"));

    assertStateCallbackCalled(false, &light)
}

void test_LightTest_state_command_different_light(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onStateCommand(onStateCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueLightDifferent/cmd_t"),
        F("ON")
    );

    assertStateCallbackNotCalled()
}

void test_LightTest_brightness_command_min(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onBrightnessCommand(onBrightnessCommandReceived);
    mock->fakeMessage(AHATOFSTR(BrightnessCommandTopic), F("0"));

    assertBrightnessCallbackCalled(0, &light)
}

void test_LightTest_brightness_command_max(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onBrightnessCommand(onBrightnessCommandReceived);
    mock->fakeMessage(AHATOFSTR(BrightnessCommandTopic), F("255"));

    assertBrightnessCallbackCalled(255, &light)
}

void test_LightTest_brightness_command_overflow(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onBrightnessCommand(onBrightnessCommandReceived);
    mock->fakeMessage(AHATOFSTR(BrightnessCommandTopic), F("300"));

    assertBrightnessCallbackNotCalled()
}

void test_LightTest_brightness_command_invalid(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onBrightnessCommand(onBrightnessCommandReceived);
    mock->fakeMessage(AHATOFSTR(BrightnessCommandTopic), F("INVALID"));

    assertBrightnessCallbackNotCalled()
}

void test_LightTest_brightness_command_different_light(void) {
    prepareTest

    HALight light(testUniqueId);
    light.onBrightnessCommand(onBrightnessCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueLightDifferent/pct_cmd_t"),
        F("50")
    );

    assertBrightnessCallbackNotCalled()
}

void test_LightTest_color_temperature_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.onColorTemperatureCommand(onColorTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(ColorTemperatureCommandTopic), F("200"));

    assertColorTempCallbackCalled(200, &light)
}

void test_LightTest_color_temperature_command_min(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.onColorTemperatureCommand(onColorTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(ColorTemperatureCommandTopic), F("153"));

    assertColorTempCallbackCalled(153, &light)
}

void test_LightTest_color_temperature_command_max(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.onColorTemperatureCommand(onColorTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(ColorTemperatureCommandTopic), F("500"));

    assertColorTempCallbackCalled(500, &light)
}

void test_LightTest_color_temperature_command_invalid(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.onColorTemperatureCommand(onColorTemperatureCommandReceived);
    mock->fakeMessage(AHATOFSTR(ColorTemperatureCommandTopic), F("INVALID"));

    assertColorTempCallbackNotCalled()
}

void test_LightTest_color_temperature_command_different_light(void) {
    prepareTest

    HALight light(testUniqueId, HALight::ColorTemperatureFeature);
    light.onColorTemperatureCommand(onColorTemperatureCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueLightDifferent/clr_temp_cmd_t"),
        F("180")
    );

    assertColorTempCallbackNotCalled()
}

void test_LightTest_rgb_color_min_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("0,0,0"));

    assertRGBColorCallbackCalled(HALight::RGBColor(0,0,0), &light)
}

void test_LightTest_rgb_color_small_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("1,2,3"));

    assertRGBColorCallbackCalled(HALight::RGBColor(1,2,3), &light)
}

void test_LightTest_rgb_color_max_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("255,255,255"));

    assertRGBColorCallbackCalled(HALight::RGBColor(255,255,255), &light)
}

void test_LightTest_rgb_color_mix_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("255,12,1"));

    assertRGBColorCallbackCalled(HALight::RGBColor(255,12,1), &light)
}

void test_LightTest_rgb_color_invalid_1_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("255,12"));

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_invalid_2_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F(",,"));

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_invalid_3_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), "");

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_invalid_4_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("255,,"));

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_invalid_5_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("thisIsNotTheColor"));

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_invalid_6_command(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(AHATOFSTR(RGBCommandTopic), F("256,123,2"));

    assertRGBColorCallbackNotCalled()
}

void test_LightTest_rgb_color_command_different_light(void) {
    prepareTest

    HALight light(testUniqueId, HALight::RGBFeature);
    light.onRGBColorCommand(onRGBColorCommand);
    mock->fakeMessage(
        F("testData/testDevice/uniqueLightDifferent/rgb_cmd_t"),
        F("255,12,1")
    );

    assertRGBColorCallbackNotCalled()
}

