#include <unity.h>
#include <Arduino.h>
#include "test_main.h"

static TestCase tests[] = {
    TEST_ENTRY(test_BinarySensorTest_availability),
    TEST_ENTRY(test_BinarySensorTest_default_params),
    TEST_ENTRY(test_BinarySensorTest_default_state_false),
    TEST_ENTRY(test_BinarySensorTest_default_state_true),
    TEST_ENTRY(test_BinarySensorTest_device_class),
    TEST_ENTRY(test_BinarySensorTest_entity_category_setter),
    TEST_ENTRY(test_BinarySensorTest_expire_after_setter),
    TEST_ENTRY(test_BinarySensorTest_expire_after_zero_setter),
    TEST_ENTRY(test_BinarySensorTest_extended_unique_id),
    TEST_ENTRY(test_BinarySensorTest_icon_setter),
    TEST_ENTRY(test_BinarySensorTest_invalid_unique_id),
    TEST_ENTRY(test_BinarySensorTest_name_setter),
    TEST_ENTRY(test_BinarySensorTest_object_id_setter),
    TEST_ENTRY(test_BinarySensorTest_publish_initial_state),
    TEST_ENTRY(test_BinarySensorTest_publish_state_debounce),
    TEST_ENTRY(test_BinarySensorTest_publish_state_debounce_skip),
    TEST_ENTRY(test_BinarySensorTest_disconnected_state_updates_local_shadow),
    TEST_ENTRY(test_BinarySensorTest_publish_state_off),
    TEST_ENTRY(test_BinarySensorTest_publish_state_on),
    TEST_ENTRY(test_ButtonTest_availability),
    TEST_ENTRY(test_ButtonTest_command_callback),
    TEST_ENTRY(test_ButtonTest_command_subscription),
    TEST_ENTRY(test_ButtonTest_default_params),
    TEST_ENTRY(test_ButtonTest_device_class),
    TEST_ENTRY(test_ButtonTest_different_button_command),
    TEST_ENTRY(test_ButtonTest_entity_category_setter),
    TEST_ENTRY(test_ButtonTest_extended_unique_id),
    TEST_ENTRY(test_ButtonTest_icon_setter),
    TEST_ENTRY(test_ButtonTest_invalid_unique_id),
    TEST_ENTRY(test_ButtonTest_name_setter),
    TEST_ENTRY(test_ButtonTest_no_command_callback),
    TEST_ENTRY(test_ButtonTest_object_id_setter),
    TEST_ENTRY(test_ButtonTest_retain_setter),
    TEST_ENTRY(test_SensorNumberTest_dont_publish_default_value_on_connect),
    TEST_ENTRY(test_SensorNumberTest_publish_debounce),
    TEST_ENTRY(test_SensorNumberTest_publish_force),
    TEST_ENTRY(test_SensorNumberTest_publish_int16),
    TEST_ENTRY(test_SensorNumberTest_publish_int32),
    TEST_ENTRY(test_SensorNumberTest_publish_int32_signed),
    TEST_ENTRY(test_SensorNumberTest_publish_int8),
    TEST_ENTRY(test_SensorNumberTest_publish_int_zero),
    TEST_ENTRY(test_SensorNumberTest_publish_p0),
    TEST_ENTRY(test_SensorNumberTest_publish_p0_zero_signed),
    TEST_ENTRY(test_SensorNumberTest_publish_p0_zero_unsigned),
    TEST_ENTRY(test_SensorNumberTest_publish_p1),
    TEST_ENTRY(test_SensorNumberTest_publish_p1_zero_signed),
    TEST_ENTRY(test_SensorNumberTest_publish_p1_zero_unsigned),
    TEST_ENTRY(test_SensorNumberTest_publish_p2),
    TEST_ENTRY(test_SensorNumberTest_publish_p2_zero_signed),
    TEST_ENTRY(test_SensorNumberTest_publish_p2_zero_unsigned),
    TEST_ENTRY(test_SensorNumberTest_publish_p3),
    TEST_ENTRY(test_SensorNumberTest_publish_p3_smaller),
    TEST_ENTRY(test_SensorNumberTest_publish_p3_zero_signed),
    TEST_ENTRY(test_SensorNumberTest_publish_p3_zero_unsigned),
    TEST_ENTRY(test_SensorNumberTest_publish_precision_mismatch),
    TEST_ENTRY(test_SensorNumberTest_disconnected_value_updates_local_shadow),
    TEST_ENTRY(test_SensorNumberTest_publish_uint16),
    TEST_ENTRY(test_SensorNumberTest_publish_uint32),
    TEST_ENTRY(test_SensorNumberTest_publish_uint8),
    TEST_ENTRY(test_SensorNumberTest_publish_value_on_connect),
    TEST_ENTRY(test_SensorTest_availability),
    TEST_ENTRY(test_SensorTest_default_entity_id_setter),
    TEST_ENTRY(test_SensorTest_default_params),
    TEST_ENTRY(test_SensorTest_device_class_setter),
    TEST_ENTRY(test_SensorTest_empty_unit_of_measurement_is_ignored),
    TEST_ENTRY(test_SensorTest_entity_category_setter),
    TEST_ENTRY(test_SensorTest_expire_after_setter),
    TEST_ENTRY(test_SensorTest_expire_after_zero_setter),
    TEST_ENTRY(test_SensorTest_extended_unique_id),
    TEST_ENTRY(test_SensorTest_force_update_setter),
    TEST_ENTRY(test_SensorTest_icon_setter),
    TEST_ENTRY(test_SensorTest_invalid_unique_id),
    TEST_ENTRY(test_SensorTest_json_attributes_topic),
    TEST_ENTRY(test_SensorTest_name_setter),
    TEST_ENTRY(test_SensorTest_object_id_setter),
    TEST_ENTRY(test_SensorTest_publish_json_attributes),
    TEST_ENTRY(test_SensorTest_publish_null_value),
    TEST_ENTRY(test_SensorTest_publish_value),
    TEST_ENTRY(test_SensorTest_state_class_setter),
    TEST_ENTRY(test_SensorTest_unit_of_measurement_setter),
    TEST_ENTRY(test_SwitchTest_availability),
    TEST_ENTRY(test_SwitchTest_command_off),
    TEST_ENTRY(test_SwitchTest_command_on),
    TEST_ENTRY(test_SwitchTest_command_subscription),
    TEST_ENTRY(test_SwitchTest_callback_publish_is_deferred_until_after_dispatch),
    TEST_ENTRY(test_SwitchTest_current_state_setter),
    TEST_ENTRY(test_SwitchTest_default_entity_id_setter),
    TEST_ENTRY(test_SwitchTest_default_params),
    TEST_ENTRY(test_SwitchTest_device_class),
    TEST_ENTRY(test_SwitchTest_device_discovery_payload),
    TEST_ENTRY(test_SwitchTest_different_switch_command),
    TEST_ENTRY(test_SwitchTest_entity_category_setter),
    TEST_ENTRY(test_SwitchTest_extended_unique_id),
    TEST_ENTRY(test_SwitchTest_icon_setter),
    TEST_ENTRY(test_SwitchTest_invalid_unique_id),
    TEST_ENTRY(test_SwitchTest_name_setter),
    TEST_ENTRY(test_SwitchTest_object_id_setter),
    TEST_ENTRY(test_SwitchTest_optimistic_setter),
    TEST_ENTRY(test_SwitchTest_publish_last_known_state),
    TEST_ENTRY(test_SwitchTest_publish_nothing_if_retained),
    TEST_ENTRY(test_SwitchTest_publish_state_off),
    TEST_ENTRY(test_SwitchTest_publish_state_on),
    TEST_ENTRY(test_SwitchTest_retain_setter),
    TEST_ENTRY(test_TextTest_availability),
    TEST_ENTRY(test_TextTest_command_callback),
    TEST_ENTRY(test_TextTest_command_subscription),
    TEST_ENTRY(test_TextTest_default_params),
    TEST_ENTRY(test_TextTest_different_text_command),
    TEST_ENTRY(test_TextTest_extended_unique_id),
    TEST_ENTRY(test_TextTest_icon_setter),
    TEST_ENTRY(test_TextTest_invalid_unique_id),
    TEST_ENTRY(test_TextTest_min_max_pattern_setters),
    TEST_ENTRY(test_TextTest_mode_setter_password),
    TEST_ENTRY(test_TextTest_name_setter),
    TEST_ENTRY(test_TextTest_object_id_setter),
    TEST_ENTRY(test_TextTest_optimistic_setter),
    TEST_ENTRY(test_TextTest_publish_last_known_state),
    TEST_ENTRY(test_TextTest_publish_nothing_if_retained),
    TEST_ENTRY(test_TextTest_publish_state),
    TEST_ENTRY(test_TextTest_publish_state_debounce),
    TEST_ENTRY(test_TextTest_callback_publish_is_deferred_until_after_dispatch),
    TEST_ENTRY(test_TextTest_retain_setter),
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
