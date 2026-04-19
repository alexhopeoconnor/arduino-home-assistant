#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueSensor";

const char ConfigTopic[] PROGMEM = {"homeassistant/binary_sensor/testDevice/uniqueSensor/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueSensor/stat_t"};

void test_BinarySensorTest_invalid_unique_id(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(nullptr);
    sensor.buildSerializerTest();
    HASerializer* serializer = sensor.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_BinarySensorTest_default_params(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_extended_unique_id(void) {
    initMqttTest(testDeviceId)

    device.enableExtendedUniqueIds();
    HABinarySensor sensor(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_availability(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HABinarySensor
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueSensor/avty_t"),
        "online",
        true
    );
}

void test_BinarySensorTest_publish_initial_state(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setCurrentState(true);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "ON", true);
}

void test_BinarySensorTest_name_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_object_id_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_device_class(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setDeviceClass("testClass");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev_cla\":\"testClass\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_entity_category_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_icon_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_expire_after_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setExpireAfter(60);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"exp_aft\":60,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_expire_after_zero_setter(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setExpireAfter(0);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_BinarySensorTest_default_state_false(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    TEST_ASSERT_FALSE(sensor.getCurrentState());
}

void test_BinarySensorTest_default_state_true(void) {
    initMqttTest(testDeviceId)

    HABinarySensor sensor(testUniqueId);
    sensor.setCurrentState(true);
    TEST_ASSERT_TRUE(sensor.getCurrentState());
}

void test_BinarySensorTest_publish_state_on(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HABinarySensor sensor(testUniqueId);
    bool result = sensor.setState(!sensor.getCurrentState());

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
    TEST_ASSERT_TRUE(result);
}

void test_BinarySensorTest_publish_state_off(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HABinarySensor sensor(testUniqueId);
    sensor.setCurrentState(true);
    bool result = sensor.setState(!sensor.getCurrentState());

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "OFF", true);
    TEST_ASSERT_TRUE(result);
}

void test_BinarySensorTest_publish_state_debounce(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HABinarySensor sensor(testUniqueId);
    sensor.setCurrentState(true);
    bool result = sensor.setState(true);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
    TEST_ASSERT_TRUE(result);
}

void test_BinarySensorTest_publish_state_debounce_skip(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HABinarySensor sensor(testUniqueId);
    sensor.setCurrentState(true);
    bool result = sensor.setState(true, true);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "ON", true);
    TEST_ASSERT_TRUE(result);
}

