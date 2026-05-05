#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueSensor";
const char ConfigTopic[] PROGMEM = {"homeassistant/sensor/testDevice/uniqueSensor/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueSensor/stat_t"};
const char JsonAttributesTopic[] PROGMEM = {"testData/testDevice/uniqueSensor/json_attr_t"};

void test_SensorTest_invalid_unique_id(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(nullptr);
    sensor.buildSerializerTest();
    HASerializer* serializer = sensor.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_SensorTest_default_params(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_extended_unique_id(void) {
    initMqttTest(testDeviceId)

    device.enableExtendedUniqueIds();
    HASensor sensor(testUniqueId);
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

void test_SensorTest_availability(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HASensor
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueSensor/avty_t"),
        "online",
        true
    );
}

void test_SensorTest_name_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_object_id_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_default_entity_id_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setDefaultEntityId("sensor.test_sensor");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"def_ent_id\":\"sensor.test_sensor\","
            "\"uniq_id\":\"uniqueSensor\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_SensorTest_device_class_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_state_class_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setStateClass("measurement");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"stat_cla\":\"measurement\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_SensorTest_entity_category_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_force_update_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setForceUpdate(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"frc_upd\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_SensorTest_icon_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_unit_of_measurement_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setUnitOfMeasurement("%");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"unit_of_meas\":\"%\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_SensorTest_empty_unit_of_measurement_is_ignored(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
    sensor.setUnitOfMeasurement("");

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

void test_SensorTest_expire_after_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_expire_after_zero_setter(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId);
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

void test_SensorTest_json_attributes_topic(void) {
    initMqttTest(testDeviceId)

    HASensor sensor(testUniqueId, HASensor::JsonAttributesFeature);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        sensor,
        (
            "{"
            "\"uniq_id\":\"uniqueSensor\","
            "\"json_attr_t\":\"testData/testDevice/uniqueSensor/json_attr_t\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueSensor/stat_t\""
            "}"
        )
    );
}

void test_SensorTest_publish_value(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensor sensor(testUniqueId);

    TEST_ASSERT_TRUE(sensor.setValue("test123"));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "test123", true);
}

void test_SensorTest_publish_null_value(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensor sensor(testUniqueId);

    TEST_ASSERT_TRUE(sensor.setValue(nullptr));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "None", true);
}

void test_SensorTest_publish_json_attributes(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensor sensor(testUniqueId, HASensor::JsonAttributesFeature);

    TEST_ASSERT_TRUE(sensor.setJsonAttributes("{\"dummy\": 1}"));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(JsonAttributesTopic), "{\"dummy\": 1}", true);
}

void test_SensorNumberTest_publish_value_on_connect(void) {
    initMqttTest(testDeviceId)

    HASensorNumber sensor(testUniqueId);
    sensor.setCurrentValue(520);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "520", true);
}

void test_SensorNumberTest_dont_publish_default_value_on_connect(void) {
    initMqttTest(testDeviceId)

    HASensorNumber sensor(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_FALSE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 1); // config only
}

void test_SensorNumberTest_publish_debounce(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    uint16_t value = 1555;
    sensor.setCurrentValue(value);

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toUInt16());
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_SensorNumberTest_publish_force(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    uint16_t value = 1555;
    sensor.setCurrentValue(value);

    TEST_ASSERT_TRUE(sensor.setValue(value, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toUInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "1555", true);
}

void test_SensorNumberTest_publish_int_zero(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    sensor.setCurrentValue(50);
    int8_t value = 0;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toInt8());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0", true);
}

void test_SensorNumberTest_publish_int8(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    int8_t value = 127;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toInt8());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "127", true);
}

void test_SensorNumberTest_publish_uint8(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    uint8_t value = 50;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toUInt8());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "50", true);
}

void test_SensorNumberTest_publish_int16(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    int16_t value = 32766;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "32766", true);
}

void test_SensorNumberTest_publish_uint16(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    uint16_t value = 65534;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toUInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "65534", true);
}

void test_SensorNumberTest_publish_int32(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    int32_t value = 2147483646;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toInt32());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "2147483646", true);
}

void test_SensorNumberTest_publish_int32_signed(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    int32_t value = -2147483646;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toInt32());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-2147483646", true);
}

void test_SensorNumberTest_publish_uint32(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId);
    uint32_t value = 4294967295;

    TEST_ASSERT_TRUE(sensor.setValue(value));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(value, sensor.getCurrentValue().toUInt32());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "4294967295", true);
}

void test_SensorNumberTest_publish_p0(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP0);

    TEST_ASSERT_TRUE(sensor.setValue(173.5426f));
    TEST_ASSERT_FALSE(sensor.getCurrentValue().isFloat());
    TEST_ASSERT_EQUAL((uint16_t)173, sensor.getCurrentValue().toUInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "173", true);
}

void test_SensorNumberTest_publish_p0_zero_unsigned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP0);

    TEST_ASSERT_TRUE(sensor.setValue(0.050f, true));
    TEST_ASSERT_FALSE(sensor.getCurrentValue().isFloat());
    TEST_ASSERT_EQUAL((uint16_t)0, sensor.getCurrentValue().toUInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0", true);
}

void test_SensorNumberTest_publish_p0_zero_signed(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP0);

    TEST_ASSERT_TRUE(sensor.setValue(-0.050f, true));
    TEST_ASSERT_FALSE(sensor.getCurrentValue().isFloat());
    TEST_ASSERT_EQUAL((uint16_t)0, sensor.getCurrentValue().toUInt16());
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0", true);
}

void test_SensorNumberTest_publish_p1(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP1);

    TEST_ASSERT_TRUE(sensor.setValue(173.5426f));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(173.5f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "173.5", true);
}

void test_SensorNumberTest_publish_p1_zero_unsigned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP1);

    TEST_ASSERT_TRUE(sensor.setValue(0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(0.1f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0.1", true);
}

void test_SensorNumberTest_publish_p1_zero_signed(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP1);

    TEST_ASSERT_TRUE(sensor.setValue(-0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(-0.1f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-0.1", true);
}

void test_SensorNumberTest_publish_p2(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP2);

    TEST_ASSERT_TRUE(sensor.setValue(173.1534f));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(173.15f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "173.15", true);
}

void test_SensorNumberTest_publish_p2_zero_unsigned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP2);

    TEST_ASSERT_TRUE(sensor.setValue(0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(0.12f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0.12", true);
}

void test_SensorNumberTest_publish_p2_zero_signed(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP2);

    TEST_ASSERT_TRUE(sensor.setValue(-0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(-0.12f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-0.12", true);
}

void test_SensorNumberTest_publish_p3(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP3);

    TEST_ASSERT_TRUE(sensor.setValue(173.333f));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(173.333f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "173.333", true);
}

void test_SensorNumberTest_publish_p3_zero_unsigned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP3);

    TEST_ASSERT_TRUE(sensor.setValue(0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(0.123f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "0.123", true);
}

void test_SensorNumberTest_publish_p3_zero_signed(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP3);

    TEST_ASSERT_TRUE(sensor.setValue(-0.123f, true));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(-0.123f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "-0.123", true);
}

void test_SensorNumberTest_publish_p3_smaller(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP3);

    TEST_ASSERT_TRUE(sensor.setValue(173.3f));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isFloat());
    AHA_ASSERT_NEAR_FLOAT(173.3f, sensor.getCurrentValue().toFloat(), 0.1);
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "173.300", true);
}

void test_SensorNumberTest_publish_precision_mismatch(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP3);

    TEST_ASSERT_FALSE(sensor.setValue(HANumeric(25.0f, 1)));
    TEST_ASSERT_FALSE(sensor.getCurrentValue().isSet());
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
}

void test_SensorNumberTest_disconnected_value_updates_local_shadow(void) {
    initMqttTest(testDeviceId)

    HASensorNumber sensor(testUniqueId, HASensorNumber::PrecisionP1);

    TEST_ASSERT_FALSE(sensor.setValue(27.5f));
    TEST_ASSERT_TRUE(sensor.getCurrentValue().isSet());
    AHA_ASSERT_NEAR_FLOAT(27.5f, sensor.getCurrentValue().toFloat(), 0.1f);
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

