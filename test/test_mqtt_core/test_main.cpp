#include <unity.h>
#include <Arduino.h>
#include "test_main.h"

static TestCase tests[] = {
    TEST_ENTRY(test_MqttTest_maximum_number_of_device_types),
    TEST_ENTRY(test_MqttTest_reconnect_interval_default_value),
    TEST_ENTRY(test_MqttTest_reconnect_interval_setter),
    TEST_ENTRY(test_MqttTest_reconnect_interval_throttles_attempts),
    TEST_ENTRY(test_MqttTest_publish_attempt_from_message_callback_is_rejected),

    TEST_ENTRY(test_DeviceTest_default_unique_id),
    TEST_ENTRY(test_DeviceTest_unique_id_constructor_char),
    TEST_ENTRY(test_DeviceTest_unique_id_constructor_byte_array),
    TEST_ENTRY(test_DeviceTest_unique_id_setter),
    TEST_ENTRY(test_DeviceTest_unique_id_setter_runtime),
    TEST_ENTRY(test_DeviceTest_serializer_no_unique_id),
    TEST_ENTRY(test_DeviceTest_serializer_unique_id_constructor_char),
    TEST_ENTRY(test_DeviceTest_serializer_unique_id_constructor_byte_array),
    TEST_ENTRY(test_DeviceTest_serializer_unique_id_setter),
    TEST_ENTRY(test_DeviceTest_serializer_manufacturer),
    TEST_ENTRY(test_DeviceTest_serializer_model),
    TEST_ENTRY(test_DeviceTest_serializer_name),
    TEST_ENTRY(test_DeviceTest_serializer_software_version),
    TEST_ENTRY(test_DeviceTest_default_availability),
    TEST_ENTRY(test_DeviceTest_enable_availability),
    TEST_ENTRY(test_DeviceTest_enable_availability_no_unique_id),
    TEST_ENTRY(test_DeviceTest_availability_publish_offline),
    TEST_ENTRY(test_DeviceTest_availability_publish_online),
    TEST_ENTRY(test_DeviceTest_extended_unique_ids_disabled),
    TEST_ENTRY(test_DeviceTest_enable_extended_unique_ids),
    TEST_ENTRY(test_DeviceTest_lwt_disabled),
    TEST_ENTRY(test_DeviceTest_lwt_enabled),
    TEST_ENTRY(test_DeviceTest_full_serialization),

    TEST_ENTRY(test_BaseDeviceTypeTest_constructor_params),
    TEST_ENTRY(test_BaseDeviceTypeTest_register_mqtt_type),
    TEST_ENTRY(test_BaseDeviceTypeTest_default_name),
    TEST_ENTRY(test_BaseDeviceTypeTest_name_setter),
    TEST_ENTRY(test_BaseDeviceTypeTest_object_id_setter),
    TEST_ENTRY(test_BaseDeviceTypeTest_default_entity_id_setter),
    TEST_ENTRY(test_BaseDeviceTypeTest_default_availability),
    TEST_ENTRY(test_BaseDeviceTypeTest_publish_nothing_if_not_configured),
    TEST_ENTRY(test_BaseDeviceTypeTest_publish_availability_online_runtime),
    TEST_ENTRY(test_BaseDeviceTypeTest_publish_availability_offline_runtime),
    TEST_ENTRY(test_BaseDeviceTypeTest_publish_shared_availability_on_connect),
    TEST_ENTRY(test_BaseDeviceTypeTest_publish_shared_availability_runtime),
    TEST_ENTRY(test_BaseDeviceTypeTest_republish_discovery),
    TEST_ENTRY(test_BaseDeviceTypeTest_remove_from_discovery),
};

static const size_t TEST_COUNT = sizeof(tests) / sizeof(tests[0]);
static size_t next_index = 0;
static bool begun = false;

void setUp(void) { }

void tearDown(void) { }

void setup()
{
    Serial.begin(115200);
    delay(500);
    UNITY_BEGIN();
    begun = true;
}

void loop()
{
    if (begun && next_index < TEST_COUNT) {
        TestCase& t = tests[next_index];
        Serial.print("\n[TEST] ");
        Serial.println(t.name);
        UnityDefaultTestRun(t.fn, t.name, t.line);
        next_index++;
        return;
    }

    if (begun && next_index >= TEST_COUNT) {
        UNITY_END();
        begun = false;
    }
}
