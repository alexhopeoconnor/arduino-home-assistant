#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#include <string.h>

#define prepareTest \
    initMqttTest(testDeviceId)

static const char* testDeviceId = "testDevice";

void test_StdFunctionCallbacksTest_button_command_callback(void) {
    prepareTest

    HAButton button("button");
    bool called = false;
    HAButton* caller = nullptr;

    button.onCommand([&](HAButton* sender) {
        called = true;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/button/cmd_t"), F("PRESS"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL_PTR(&button, caller);
}

void test_StdFunctionCallbacksTest_switch_command_callback(void) {
    prepareTest

    HASwitch testSwitch("switch");
    bool called = false;
    bool state = false;
    HASwitch* caller = nullptr;

    testSwitch.onCommand([&](bool cmdState, HASwitch* sender) {
        called = true;
        state = cmdState;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/switch/cmd_t"), F("ON"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_TRUE(state);
    TEST_ASSERT_EQUAL_PTR(&testSwitch, caller);
}

void test_StdFunctionCallbacksTest_number_command_callback(void) {
    prepareTest

    HANumber number("number");
    bool called = false;
    HANumeric value;
    HANumber* caller = nullptr;

    number.onCommand([&](HANumeric cmdValue, HANumber* sender) {
        called = true;
        value = cmdValue;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/number/cmd_t"), F("1234"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_TRUE(HANumeric(1234, 0) == value);
    TEST_ASSERT_EQUAL_PTR(&number, caller);
}

void test_StdFunctionCallbacksTest_text_command_callback(void) {
    prepareTest

    HAText text("text");
    bool called = false;
    bool valueMatched = false;
    HAText* caller = nullptr;

    text.onCommand([&](const char* value, HAText* sender) {
        called = true;
        valueMatched = strcmp(value, "hello") == 0;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/text/cmd_t"), F("hello"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_TRUE(valueMatched);
    TEST_ASSERT_EQUAL_PTR(&text, caller);
}

void test_StdFunctionCallbacksTest_select_command_callback(void) {
    prepareTest

    HASelect select("select");
    select.setOptions("Option A;Option B");
    bool called = false;
    int8_t selectedIndex = -1;
    HASelect* caller = nullptr;

    select.onCommand([&](int8_t index, HASelect* sender) {
        called = true;
        selectedIndex = index;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/select/cmd_t"), F("Option B"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL((int8_t)1, selectedIndex);
    TEST_ASSERT_EQUAL_PTR(&select, caller);
}

void test_StdFunctionCallbacksTest_scene_command_callback(void) {
    prepareTest

    HAScene scene("scene");
    bool called = false;
    HAScene* caller = nullptr;

    scene.onCommand([&](HAScene* sender) {
        called = true;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/scene/cmd_t"), F("ON"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL_PTR(&scene, caller);
}

void test_StdFunctionCallbacksTest_cover_command_callback(void) {
    prepareTest

    HACover cover("cover");
    bool called = false;
    HACover::CoverCommand command = static_cast<HACover::CoverCommand>(0);
    HACover* caller = nullptr;

    cover.onCommand([&](HACover::CoverCommand cmd, HACover* sender) {
        called = true;
        command = cmd;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/cover/cmd_t"), F("OPEN"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL(HACover::CommandOpen, command);
    TEST_ASSERT_EQUAL_PTR(&cover, caller);
}

void test_StdFunctionCallbacksTest_lock_command_callback(void) {
    prepareTest

    HALock lock("lock");
    bool called = false;
    HALock::LockCommand command = static_cast<HALock::LockCommand>(0);
    HALock* caller = nullptr;

    lock.onCommand([&](HALock::LockCommand cmd, HALock* sender) {
        called = true;
        command = cmd;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/lock/cmd_t"), F("UNLOCK"));

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL(HALock::CommandUnlock, command);
    TEST_ASSERT_EQUAL_PTR(&lock, caller);
}

void test_StdFunctionCallbacksTest_fan_command_callbacks(void) {
    prepareTest

    HAFan fan("fan", HAFan::SpeedsFeature);
    bool stateCalled = false;
    bool state = false;
    HAFan* stateCaller = nullptr;
    bool speedCalled = false;
    uint16_t speed = 0;
    HAFan* speedCaller = nullptr;

    fan.onStateCommand([&](bool cmdState, HAFan* sender) {
        stateCalled = true;
        state = cmdState;
        stateCaller = sender;
    });
    fan.onSpeedCommand([&](uint16_t cmdSpeed, HAFan* sender) {
        speedCalled = true;
        speed = cmdSpeed;
        speedCaller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/fan/cmd_t"), F("ON"));
    mock->fakeMessage(F("testData/testDevice/fan/pct_cmd_t"), F("50"));

    TEST_ASSERT_TRUE(stateCalled);
    TEST_ASSERT_TRUE(state);
    TEST_ASSERT_EQUAL_PTR(&fan, stateCaller);
    TEST_ASSERT_TRUE(speedCalled);
    TEST_ASSERT_EQUAL((uint16_t)50, speed);
    TEST_ASSERT_EQUAL_PTR(&fan, speedCaller);
}

void test_StdFunctionCallbacksTest_light_command_callbacks(void) {
    prepareTest

    HALight light(
        "light",
        HALight::BrightnessFeature | HALight::ColorTemperatureFeature | HALight::RGBFeature
    );
    bool stateCalled = false;
    bool state = false;
    HALight* stateCaller = nullptr;
    bool brightnessCalled = false;
    uint8_t brightness = 0;
    HALight* brightnessCaller = nullptr;
    bool colorTempCalled = false;
    uint16_t colorTemperature = 0;
    HALight* colorTempCaller = nullptr;
    bool rgbCalled = false;
    HALight::RGBColor rgbColor;
    HALight* rgbCaller = nullptr;

    light.onStateCommand([&](bool cmdState, HALight* sender) {
        stateCalled = true;
        state = cmdState;
        stateCaller = sender;
    });
    light.onBrightnessCommand([&](uint8_t cmdBrightness, HALight* sender) {
        brightnessCalled = true;
        brightness = cmdBrightness;
        brightnessCaller = sender;
    });
    light.onColorTemperatureCommand([&](uint16_t cmdTemperature, HALight* sender) {
        colorTempCalled = true;
        colorTemperature = cmdTemperature;
        colorTempCaller = sender;
    });
    light.onRGBColorCommand([&](HALight::RGBColor cmdColor, HALight* sender) {
        rgbCalled = true;
        rgbColor = cmdColor;
        rgbCaller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/light/cmd_t"), F("ON"));
    mock->fakeMessage(F("testData/testDevice/light/bri_cmd_t"), F("42"));
    mock->fakeMessage(F("testData/testDevice/light/clr_temp_cmd_t"), F("250"));
    mock->fakeMessage(F("testData/testDevice/light/rgb_cmd_t"), F("1,2,3"));

    TEST_ASSERT_TRUE(stateCalled);
    TEST_ASSERT_TRUE(state);
    TEST_ASSERT_EQUAL_PTR(&light, stateCaller);
    TEST_ASSERT_TRUE(brightnessCalled);
    TEST_ASSERT_EQUAL((uint8_t)42, brightness);
    TEST_ASSERT_EQUAL_PTR(&light, brightnessCaller);
    TEST_ASSERT_TRUE(colorTempCalled);
    TEST_ASSERT_EQUAL((uint16_t)250, colorTemperature);
    TEST_ASSERT_EQUAL_PTR(&light, colorTempCaller);
    TEST_ASSERT_TRUE(rgbCalled);
    TEST_ASSERT_TRUE(rgbColor.isSet);
    TEST_ASSERT_TRUE(rgbColor == HALight::RGBColor(1, 2, 3));
    TEST_ASSERT_EQUAL_PTR(&light, rgbCaller);
}

void test_StdFunctionCallbacksTest_hvac_command_callbacks(void) {
    prepareTest

    HAHVAC hvac(
        "hvac",
        HAHVAC::AuxHeatingFeature |
            HAHVAC::PowerFeature |
            HAHVAC::FanFeature |
            HAHVAC::SwingFeature |
            HAHVAC::ModesFeature |
            HAHVAC::TargetTemperatureFeature
    );
    bool auxCalled = false;
    bool auxState = false;
    HAHVAC* auxCaller = nullptr;
    bool powerCalled = false;
    bool powerState = true;
    HAHVAC* powerCaller = nullptr;
    bool fanModeCalled = false;
    HAHVAC::FanMode fanMode = HAHVAC::UnknownFanMode;
    HAHVAC* fanModeCaller = nullptr;
    bool swingModeCalled = false;
    HAHVAC::SwingMode swingMode = HAHVAC::UnknownSwingMode;
    HAHVAC* swingModeCaller = nullptr;
    bool modeCalled = false;
    HAHVAC::Mode mode = HAHVAC::UnknownMode;
    HAHVAC* modeCaller = nullptr;
    bool targetTempCalled = false;
    HANumeric targetTemp;
    HAHVAC* targetTempCaller = nullptr;

    hvac.onAuxStateCommand([&](bool state, HAHVAC* sender) {
        auxCalled = true;
        auxState = state;
        auxCaller = sender;
    });
    hvac.onPowerCommand([&](bool state, HAHVAC* sender) {
        powerCalled = true;
        powerState = state;
        powerCaller = sender;
    });
    hvac.onFanModeCommand([&](HAHVAC::FanMode cmdMode, HAHVAC* sender) {
        fanModeCalled = true;
        fanMode = cmdMode;
        fanModeCaller = sender;
    });
    hvac.onSwingModeCommand([&](HAHVAC::SwingMode cmdMode, HAHVAC* sender) {
        swingModeCalled = true;
        swingMode = cmdMode;
        swingModeCaller = sender;
    });
    hvac.onModeCommand([&](HAHVAC::Mode cmdMode, HAHVAC* sender) {
        modeCalled = true;
        mode = cmdMode;
        modeCaller = sender;
    });
    hvac.onTargetTemperatureCommand([&](HANumeric cmdTemperature, HAHVAC* sender) {
        targetTempCalled = true;
        targetTemp = cmdTemperature;
        targetTempCaller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/hvac/aux_cmd_t"), F("ON"));
    mock->fakeMessage(F("testData/testDevice/hvac/pow_cmd_t"), F("OFF"));
    mock->fakeMessage(F("testData/testDevice/hvac/fan_mode_cmd_t"), F("high"));
    mock->fakeMessage(F("testData/testDevice/hvac/swing_mode_cmd_t"), F("on"));
    mock->fakeMessage(F("testData/testDevice/hvac/mode_cmd_t"), F("heat"));
    mock->fakeMessage(F("testData/testDevice/hvac/temp_cmd_t"), F("215"));

    TEST_ASSERT_TRUE(auxCalled);
    TEST_ASSERT_TRUE(auxState);
    TEST_ASSERT_EQUAL_PTR(&hvac, auxCaller);
    TEST_ASSERT_TRUE(powerCalled);
    TEST_ASSERT_FALSE(powerState);
    TEST_ASSERT_EQUAL_PTR(&hvac, powerCaller);
    TEST_ASSERT_TRUE(fanModeCalled);
    TEST_ASSERT_EQUAL(HAHVAC::HighFanMode, fanMode);
    TEST_ASSERT_EQUAL_PTR(&hvac, fanModeCaller);
    TEST_ASSERT_TRUE(swingModeCalled);
    TEST_ASSERT_EQUAL(HAHVAC::OnSwingMode, swingMode);
    TEST_ASSERT_EQUAL_PTR(&hvac, swingModeCaller);
    TEST_ASSERT_TRUE(modeCalled);
    TEST_ASSERT_EQUAL(HAHVAC::HeatMode, mode);
    TEST_ASSERT_EQUAL_PTR(&hvac, modeCaller);
    TEST_ASSERT_TRUE(targetTempCalled);
    TEST_ASSERT_TRUE(HANumeric(21.5f, 1) == targetTemp);
    TEST_ASSERT_EQUAL_PTR(&hvac, targetTempCaller);
}

