#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* triggerType = "myType";
static const char* triggerSubtype = "mySubtype";

const char ConfigTopic[] PROGMEM = {
    "homeassistant/device_automation/testDevice/myType_mySubtype/config"
};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};

void test_DeviceTriggerTest_invalid_type(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(nullptr, triggerSubtype);
    trigger.buildSerializerTest();
    HASerializer* serializer = trigger.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_DeviceTriggerTest_invalid_type_progmem(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(
        static_cast<HADeviceTrigger::TriggerType>(50),
        triggerSubtype
    );
    trigger.buildSerializerTest();
    HASerializer* serializer = trigger.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
    TEST_ASSERT_TRUE(trigger.isProgmemType());
}

void test_DeviceTriggerTest_invalid_subtype(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(triggerType, nullptr);
    trigger.buildSerializerTest();
    HASerializer* serializer = trigger.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_DeviceTriggerTest_invalid_subtype_progmem(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(
        triggerType,
        static_cast<HADeviceTrigger::TriggerSubtype>(50)
    );
    trigger.buildSerializerTest();
    HASerializer* serializer = trigger.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
}

void test_DeviceTriggerTest_unique_id_generator(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(triggerType, triggerSubtype);
    TEST_ASSERT_EQUAL_STRING("myType_mySubtype", trigger.uniqueId());
}

void test_DeviceTriggerTest_string_type_string_subtype(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(triggerType, triggerSubtype);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        trigger,
        (
            "{"
            "\"atype\":\"trigger\","
            "\"type\":\"myType\","
            "\"stype\":\"mySubtype\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/myType_mySubtype/t\""
            "}"
        )
    );
}

void test_DeviceTriggerTest_device_discovery_payload(void) {
    initMqttTest(testDeviceId)

    mqtt.enableDeviceDiscovery();
    HADeviceTrigger trigger(triggerType, triggerSubtype);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"myType_mySubtype\":{"
                    "\"p\":\"device_automation\","
                    "\"atype\":\"trigger\","
                    "\"type\":\"myType\","
                    "\"stype\":\"mySubtype\","
                    "\"t\":\"testData/testDevice/myType_mySubtype/t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_DeviceTriggerTest_progmem_type_string_subtype(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(HADeviceTrigger::ButtonShortPressType, triggerSubtype);
    AHA_ASSERT_ENTITY_CONFIG_ON_TOPIC(
        mock,
        trigger,
        F("homeassistant/device_automation/testDevice/button_short_press_mySubtype/config"),
        (
            "{"
            "\"atype\":\"trigger\","
            "\"type\":\"button_short_press\","
            "\"stype\":\"mySubtype\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/button_short_press_mySubtype/t\""
            "}"
        )
    );
}

void test_DeviceTriggerTest_string_type_progmem_subtype(void) {
    initMqttTest(testDeviceId)

    HADeviceTrigger trigger(triggerType, HADeviceTrigger::Button1Subtype);
    AHA_ASSERT_ENTITY_CONFIG_ON_TOPIC(
        mock,
        trigger,
        F("homeassistant/device_automation/testDevice/myType_button_1/config"),
        (
            "{"
            "\"atype\":\"trigger\","
            "\"type\":\"myType\","
            "\"stype\":\"button_1\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/myType_button_1/t\""
            "}"
        )
    );
}

void test_DeviceTriggerTest_trigger(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTrigger trigger(triggerType, triggerSubtype);
    bool result = trigger.trigger();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        F("testData/testDevice/myType_mySubtype/t"),
        "",
        false
    );
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTriggerTest_trigger_progmem_type(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTrigger trigger(HADeviceTrigger::ButtonShortPressType, triggerSubtype);
    bool result = trigger.trigger();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        F("testData/testDevice/button_short_press_mySubtype/t"),
        "",
        false
    );
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTriggerTest_trigger_progmem_subtype(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTrigger trigger(triggerType, HADeviceTrigger::TurnOnSubtype);
    bool result = trigger.trigger();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        F("testData/testDevice/myType_turn_on/t"),
        "",
        false
    );
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTriggerTest_trigger_progmem_type_subtype(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonShortPressType,
        HADeviceTrigger::TurnOnSubtype
    );
    bool result = trigger.trigger();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        F("testData/testDevice/button_short_press_turn_on/t"),
        "",
        false
    );
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTriggerTest_type_progmem_button_short_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonShortPressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonShortPressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_short_release(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonShortReleaseType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonShortReleaseType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_long_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonLongPressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonLongPressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_long_release(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonLongReleaseType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonLongReleaseType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_double_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonDoublePressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonDoublePressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_triple_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonTriplePressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonTriplePressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_quadruple_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonQuadruplePressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonQuadruplePressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_type_progmem_button_quintuple_press(void) {
    HADeviceTrigger trigger(
        HADeviceTrigger::ButtonQuintuplePressType,
        nullptr
    );

    TEST_ASSERT_EQUAL(HAButtonQuintuplePressType, trigger.getType());
    TEST_ASSERT_TRUE(trigger.isProgmemType());
    TEST_ASSERT_TRUE(trigger.getSubtype() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_turn_on(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::TurnOnSubtype
    );

    TEST_ASSERT_EQUAL(HATurnOnSubtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_turn_off(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::TurnOffSubtype
    );

    TEST_ASSERT_EQUAL(HATurnOffSubtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_1(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button1Subtype
    );

    TEST_ASSERT_EQUAL(HAButton1Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_2(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button2Subtype
    );

    TEST_ASSERT_EQUAL(HAButton2Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_3(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button3Subtype
    );

    TEST_ASSERT_EQUAL(HAButton3Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_4(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button4Subtype
    );

    TEST_ASSERT_EQUAL(HAButton4Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_5(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button5Subtype
    );

    TEST_ASSERT_EQUAL(HAButton5Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

void test_DeviceTriggerTest_subtype_progmem_button_6(void) {
    HADeviceTrigger trigger(
        nullptr,
        HADeviceTrigger::Button6Subtype
    );

    TEST_ASSERT_EQUAL(HAButton6Subtype, trigger.getSubtype());
    TEST_ASSERT_TRUE(trigger.isProgmemSubtype());
    TEST_ASSERT_TRUE(trigger.getType() == nullptr);
}

