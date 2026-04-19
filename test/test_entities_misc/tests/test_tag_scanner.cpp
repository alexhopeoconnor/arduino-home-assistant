#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueScanner";

const char ConfigTopic[] PROGMEM = {"homeassistant/tag/testDevice/uniqueScanner/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};

void test_TagScannerTest_invalid_unique_id(void) {
    initMqttTest(testDeviceId)

    HATagScanner scanner(nullptr);
    scanner.buildSerializerTest();
    HASerializer* serializer = scanner.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_TagScannerTest_default_params(void) {
    initMqttTest(testDeviceId)

    HATagScanner scanner(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scanner,
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"t\":\"testData/testDevice/uniqueScanner/t\""
            "}"
        )
    );
}

void test_TagScannerTest_device_discovery_payload(void) {
    initMqttTest(testDeviceId)

    mqtt.enableDeviceDiscovery();
    HATagScanner scanner(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueScanner\":{"
                    "\"p\":\"tag\","
                    "\"t\":\"testData/testDevice/uniqueScanner/t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_TagScannerTest_nullptr_tag_scanned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HATagScanner scanner(testUniqueId);

    TEST_ASSERT_FALSE(scanner.tagScanned(nullptr));
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

void test_TagScannerTest_empty_tag_scanned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HATagScanner scanner(testUniqueId);

    TEST_ASSERT_FALSE(scanner.tagScanned(""));
    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
}

void test_TagScannerTest_tag_scanned(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HATagScanner scanner(testUniqueId);

    TEST_ASSERT_TRUE(scanner.tagScanned("helloTag"));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        F("testData/testDevice/uniqueScanner/t"),
        "helloTag",
        false
    );
}

