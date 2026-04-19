#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueCamera";

const char ConfigTopic[] PROGMEM = {"homeassistant/camera/testDevice/uniqueCamera/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char DataTopic[] PROGMEM = {"testData/testDevice/uniqueCamera/t"};

void test_CameraTest_invalid_unique_id(void) {
    initMqttTest(testDeviceId)

    HACamera camera(nullptr);
    camera.buildSerializerTest();
    HASerializer* serializer = camera.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_CameraTest_default_params(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"uniq_id\":\"uniqueCamera\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_extended_unique_id(void) {
    initMqttTest(testDeviceId)

    device.enableExtendedUniqueIds();
    HACamera camera(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueCamera\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_device_discovery_payload(void) {
    initMqttTest(testDeviceId)

    mqtt.enableDeviceDiscovery();
    HACamera camera(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueCamera\":{"
                    "\"p\":\"camera\","
                    "\"uniq_id\":\"uniqueCamera\","
                    "\"t\":\"testData/testDevice/uniqueCamera/t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_CameraTest_availability(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HACamera
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueCamera/avty_t"),
        "online",
        true
    );
}

void test_CameraTest_name_setter(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueCamera\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_object_id_setter(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueCamera\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_default_entity_id_setter(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setDefaultEntityId("camera.test_camera");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"def_ent_id\":\"camera.test_camera\","
            "\"uniq_id\":\"uniqueCamera\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_entity_category_setter(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"uniq_id\":\"uniqueCamera\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}


void test_CameraTest_icon_setter(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"uniq_id\":\"uniqueCamera\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_encoding_base64(void) {
    initMqttTest(testDeviceId)

    HACamera camera(testUniqueId);
    camera.setEncoding(HACamera::EncodingBase64);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        camera,
        (
            "{"
            "\"uniq_id\":\"uniqueCamera\","
            "\"e\":\"b64\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueCamera/t\""
            "}"
        )
    );
}

void test_CameraTest_publish_nullptr(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HACamera camera(testUniqueId);

    bool result = camera.publishImage(nullptr, 0);

    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
    TEST_ASSERT_FALSE(result);
}

void test_CameraTest_publish_image(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HACamera camera(testUniqueId);

    const char* data = "IMAGE CONTENT";
    bool result = camera.publishImage((const uint8_t*)data, strlen(data));

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(DataTopic), "IMAGE CONTENT", true);
    TEST_ASSERT_TRUE(result);
}

