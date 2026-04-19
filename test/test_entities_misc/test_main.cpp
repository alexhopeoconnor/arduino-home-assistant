#include <unity.h>
#include <Arduino.h>
#include "test_main.h"

static TestCase tests[] = {
    TEST_ENTRY(test_DeviceTrackerTest_availability),
    TEST_ENTRY(test_DeviceTrackerTest_default_entity_id_setter),
    TEST_ENTRY(test_DeviceTrackerTest_default_params),
    TEST_ENTRY(test_DeviceTrackerTest_default_state),
    TEST_ENTRY(test_DeviceTrackerTest_default_state_unknown),
    TEST_ENTRY(test_DeviceTrackerTest_device_discovery_payload),
    TEST_ENTRY(test_DeviceTrackerTest_entity_category_setter),
    TEST_ENTRY(test_DeviceTrackerTest_extended_unique_id),
    TEST_ENTRY(test_DeviceTrackerTest_icon_setter),
    TEST_ENTRY(test_DeviceTrackerTest_invalid_unique_id),
    TEST_ENTRY(test_DeviceTrackerTest_name_setter),
    TEST_ENTRY(test_DeviceTrackerTest_object_id_setter),
    TEST_ENTRY(test_DeviceTrackerTest_publish_initial_state),
    TEST_ENTRY(test_DeviceTrackerTest_publish_state_debounce),
    TEST_ENTRY(test_DeviceTrackerTest_publish_state_debounce_skip),
    TEST_ENTRY(test_DeviceTrackerTest_publish_state_home),
    TEST_ENTRY(test_DeviceTrackerTest_publish_state_not_available),
    TEST_ENTRY(test_DeviceTrackerTest_publish_state_not_home),
    TEST_ENTRY(test_DeviceTrackerTest_source_type_bluetooth),
    TEST_ENTRY(test_DeviceTrackerTest_source_type_bluetooth_le),
    TEST_ENTRY(test_DeviceTrackerTest_source_type_gps),
    TEST_ENTRY(test_DeviceTrackerTest_source_type_router),
    TEST_ENTRY(test_DeviceTriggerTest_device_discovery_payload),
    TEST_ENTRY(test_DeviceTriggerTest_invalid_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_invalid_subtype_progmem),
    TEST_ENTRY(test_DeviceTriggerTest_invalid_type),
    TEST_ENTRY(test_DeviceTriggerTest_invalid_type_progmem),
    TEST_ENTRY(test_DeviceTriggerTest_progmem_type_string_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_string_type_progmem_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_string_type_string_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_1),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_2),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_3),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_4),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_5),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_button_6),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_turn_off),
    TEST_ENTRY(test_DeviceTriggerTest_subtype_progmem_turn_on),
    TEST_ENTRY(test_DeviceTriggerTest_trigger),
    TEST_ENTRY(test_DeviceTriggerTest_trigger_progmem_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_trigger_progmem_type),
    TEST_ENTRY(test_DeviceTriggerTest_trigger_progmem_type_subtype),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_double_press),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_long_press),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_long_release),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_quadruple_press),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_quintuple_press),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_short_press),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_short_release),
    TEST_ENTRY(test_DeviceTriggerTest_type_progmem_button_triple_press),
    TEST_ENTRY(test_DeviceTriggerTest_unique_id_generator),
    TEST_ENTRY(test_SceneTest_availability),
    TEST_ENTRY(test_SceneTest_command_callback),
    TEST_ENTRY(test_SceneTest_command_subscription),
    TEST_ENTRY(test_SceneTest_default_entity_id_setter),
    TEST_ENTRY(test_SceneTest_default_params),
    TEST_ENTRY(test_SceneTest_device_discovery_payload),
    TEST_ENTRY(test_SceneTest_different_scene_command),
    TEST_ENTRY(test_SceneTest_entity_category_setter),
    TEST_ENTRY(test_SceneTest_extended_unique_id),
    TEST_ENTRY(test_SceneTest_icon_setter),
    TEST_ENTRY(test_SceneTest_invalid_unique_id),
    TEST_ENTRY(test_SceneTest_name_setter),
    TEST_ENTRY(test_SceneTest_no_command_callback),
    TEST_ENTRY(test_SceneTest_object_id_setter),
    TEST_ENTRY(test_SceneTest_retain_setter),
    TEST_ENTRY(test_StdFunctionCallbacksTest_button_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_cover_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_fan_command_callbacks),
    TEST_ENTRY(test_StdFunctionCallbacksTest_hvac_command_callbacks),
    TEST_ENTRY(test_StdFunctionCallbacksTest_light_command_callbacks),
    TEST_ENTRY(test_StdFunctionCallbacksTest_lock_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_number_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_scene_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_select_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_switch_command_callback),
    TEST_ENTRY(test_StdFunctionCallbacksTest_text_command_callback),
    TEST_ENTRY(test_TagScannerTest_default_params),
    TEST_ENTRY(test_TagScannerTest_device_discovery_payload),
    TEST_ENTRY(test_TagScannerTest_empty_tag_scanned),
    TEST_ENTRY(test_TagScannerTest_invalid_unique_id),
    TEST_ENTRY(test_TagScannerTest_nullptr_tag_scanned),
    TEST_ENTRY(test_TagScannerTest_tag_scanned),
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
