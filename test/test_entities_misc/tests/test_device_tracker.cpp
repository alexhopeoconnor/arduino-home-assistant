#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueTracker";

const char ConfigTopic[] PROGMEM = {
    "homeassistant/device_tracker/testDevice/uniqueTracker/config"
};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueTracker/stat_t"};

void test_DeviceTrackerTest_invalid_unique_id(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(nullptr);
    tracker.buildSerializerTest();
    HASerializer* serializer = tracker.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_DeviceTrackerTest_default_params(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_extended_unique_id(void) {
    initMqttTest(testDeviceId)

    device.enableExtendedUniqueIds();
    HADeviceTracker tracker(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueTracker\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_device_discovery_payload(void) {
    initMqttTest(testDeviceId)

    mqtt.enableDeviceDiscovery();
    HADeviceTracker tracker(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueTracker\":{"
                    "\"p\":\"device_tracker\","
                    "\"uniq_id\":\"uniqueTracker\","
                    "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_DeviceTrackerTest_source_type_gps(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setSourceType(HADeviceTracker::SourceTypeGPS);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"src_type\":\"gps\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_source_type_router(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setSourceType(HADeviceTracker::SourceTypeRouter);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"src_type\":\"router\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_source_type_bluetooth(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setSourceType(HADeviceTracker::SourceTypeBluetooth);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"src_type\":\"bluetooth\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_source_type_bluetooth_le(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setSourceType(HADeviceTracker::SourceTypeBluetoothLE);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"src_type\":\"bluetooth_le\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_availability(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HADeviceTracker
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueTracker/avty_t"),
        "online",
        true
    );
}

void test_DeviceTrackerTest_publish_initial_state(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setCurrentState(HADeviceTracker::StateHome);
    mqtt.loop();

    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "home", true);
}

void test_DeviceTrackerTest_name_setter(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueTracker\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_object_id_setter(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueTracker\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_default_entity_id_setter(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setDefaultEntityId("device_tracker.test_tracker");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"def_ent_id\":\"device_tracker.test_tracker\","
            "\"uniq_id\":\"uniqueTracker\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_entity_category_setter(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_icon_setter(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        tracker,
        (
            "{"
            "\"uniq_id\":\"uniqueTracker\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueTracker/stat_t\""
            "}"
        )
    );
}

void test_DeviceTrackerTest_default_state_unknown(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    TEST_ASSERT_EQUAL(HADeviceTracker::StateUnknown, tracker.getState());
}

void test_DeviceTrackerTest_default_state(void) {
    initMqttTest(testDeviceId)

    HADeviceTracker tracker(testUniqueId);
    tracker.setCurrentState(HADeviceTracker::StateNotAvailable);
    TEST_ASSERT_EQUAL(HADeviceTracker::StateNotAvailable, tracker.getState());
}

void test_DeviceTrackerTest_publish_state_home(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTracker tracker(testUniqueId);
    bool result = tracker.setState(HADeviceTracker::StateHome);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "home", true);
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTrackerTest_publish_state_not_home(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTracker tracker(testUniqueId);
    bool result = tracker.setState(HADeviceTracker::StateNotHome);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "not_home", true);
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTrackerTest_publish_state_not_available(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTracker tracker(testUniqueId);
    bool result = tracker.setState(HADeviceTracker::StateNotAvailable);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "offline", true);
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTrackerTest_publish_state_debounce(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTracker tracker(testUniqueId);
    tracker.setCurrentState(HADeviceTracker::StateHome);
    bool result = tracker.setState(HADeviceTracker::StateHome);

    // it shouldn't publish data if state doesn't change
    TEST_ASSERT_EQUAL(mock->getFlushedMessagesNb(), 0);
    TEST_ASSERT_TRUE(result);
}

void test_DeviceTrackerTest_publish_state_debounce_skip(void) {
    initMqttTest(testDeviceId)

    mock->connectDummy();
    HADeviceTracker tracker(testUniqueId);
    tracker.setCurrentState(HADeviceTracker::StateHome);
    bool result = tracker.setState(HADeviceTracker::StateHome, true);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "home", true);
    TEST_ASSERT_TRUE(result);
}

