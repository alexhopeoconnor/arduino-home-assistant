#include <AUnit.h>
#include <ArduinoHA.h>
#include <string.h>

#define prepareTest \
    initMqttTest(testDeviceId)

using aunit::TestRunner;

static const char* testDeviceId = "testDevice";

AHA_TEST(StdFunctionCallbacksTest, button_command_callback) {
    prepareTest

    HAButton button("button");
    bool called = false;
    HAButton* caller = nullptr;

    button.onCommand([&](HAButton* sender) {
        called = true;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/button/cmd_t"), F("PRESS"));

    assertTrue(called);
    assertEqual(&button, caller);
}

AHA_TEST(StdFunctionCallbacksTest, switch_command_callback) {
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

    assertTrue(called);
    assertTrue(state);
    assertEqual(&testSwitch, caller);
}

AHA_TEST(StdFunctionCallbacksTest, number_command_callback) {
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

    assertTrue(called);
    assertTrue(HANumeric(1234, 0) == value);
    assertEqual(&number, caller);
}

AHA_TEST(StdFunctionCallbacksTest, text_command_callback) {
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

    assertTrue(called);
    assertTrue(valueMatched);
    assertEqual(&text, caller);
}

AHA_TEST(StdFunctionCallbacksTest, select_command_callback) {
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

    assertTrue(called);
    assertEqual((int8_t)1, selectedIndex);
    assertEqual(&select, caller);
}

AHA_TEST(StdFunctionCallbacksTest, scene_command_callback) {
    prepareTest

    HAScene scene("scene");
    bool called = false;
    HAScene* caller = nullptr;

    scene.onCommand([&](HAScene* sender) {
        called = true;
        caller = sender;
    });

    mock->fakeMessage(F("testData/testDevice/scene/cmd_t"), F("ON"));

    assertTrue(called);
    assertEqual(&scene, caller);
}

AHA_TEST(StdFunctionCallbacksTest, cover_command_callback) {
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

    assertTrue(called);
    assertEqual(HACover::CommandOpen, command);
    assertEqual(&cover, caller);
}

AHA_TEST(StdFunctionCallbacksTest, lock_command_callback) {
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

    assertTrue(called);
    assertEqual(HALock::CommandUnlock, command);
    assertEqual(&lock, caller);
}

AHA_TEST(StdFunctionCallbacksTest, fan_command_callbacks) {
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

    assertTrue(stateCalled);
    assertTrue(state);
    assertEqual(&fan, stateCaller);
    assertTrue(speedCalled);
    assertEqual((uint16_t)50, speed);
    assertEqual(&fan, speedCaller);
}

AHA_TEST(StdFunctionCallbacksTest, light_command_callbacks) {
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

    assertTrue(stateCalled);
    assertTrue(state);
    assertEqual(&light, stateCaller);
    assertTrue(brightnessCalled);
    assertEqual((uint8_t)42, brightness);
    assertEqual(&light, brightnessCaller);
    assertTrue(colorTempCalled);
    assertEqual((uint16_t)250, colorTemperature);
    assertEqual(&light, colorTempCaller);
    assertTrue(rgbCalled);
    assertTrue(rgbColor.isSet);
    assertTrue(rgbColor == HALight::RGBColor(1, 2, 3));
    assertEqual(&light, rgbCaller);
}

AHA_TEST(StdFunctionCallbacksTest, hvac_command_callbacks) {
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

    assertTrue(auxCalled);
    assertTrue(auxState);
    assertEqual(&hvac, auxCaller);
    assertTrue(powerCalled);
    assertFalse(powerState);
    assertEqual(&hvac, powerCaller);
    assertTrue(fanModeCalled);
    assertEqual(HAHVAC::HighFanMode, fanMode);
    assertEqual(&hvac, fanModeCaller);
    assertTrue(swingModeCalled);
    assertEqual(HAHVAC::OnSwingMode, swingMode);
    assertEqual(&hvac, swingModeCaller);
    assertTrue(modeCalled);
    assertEqual(HAHVAC::HeatMode, mode);
    assertEqual(&hvac, modeCaller);
    assertTrue(targetTempCalled);
    assertTrue(HANumeric(21.5f, 1) == targetTemp);
    assertEqual(&hvac, targetTempCaller);
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
