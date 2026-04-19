#ifndef AHA_TEST_MAIN_ENTITIES_MISC_H
#define AHA_TEST_MAIN_ENTITIES_MISC_H

#include <Arduino.h>

using TestFn = void (*)(void);

struct TestCase {
    const char* name;
    TestFn fn;
    uint16_t line;
};

#define TEST_ENTRY(fn) { #fn, fn, __LINE__ }

extern void test_DeviceTrackerTest_availability(void);
extern void test_DeviceTrackerTest_default_entity_id_setter(void);
extern void test_DeviceTrackerTest_default_params(void);
extern void test_DeviceTrackerTest_default_state(void);
extern void test_DeviceTrackerTest_default_state_unknown(void);
extern void test_DeviceTrackerTest_device_discovery_payload(void);
extern void test_DeviceTrackerTest_entity_category_setter(void);
extern void test_DeviceTrackerTest_extended_unique_id(void);
extern void test_DeviceTrackerTest_icon_setter(void);
extern void test_DeviceTrackerTest_invalid_unique_id(void);
extern void test_DeviceTrackerTest_name_setter(void);
extern void test_DeviceTrackerTest_object_id_setter(void);
extern void test_DeviceTrackerTest_publish_initial_state(void);
extern void test_DeviceTrackerTest_publish_state_debounce(void);
extern void test_DeviceTrackerTest_publish_state_debounce_skip(void);
extern void test_DeviceTrackerTest_publish_state_home(void);
extern void test_DeviceTrackerTest_publish_state_not_available(void);
extern void test_DeviceTrackerTest_publish_state_not_home(void);
extern void test_DeviceTrackerTest_source_type_bluetooth(void);
extern void test_DeviceTrackerTest_source_type_bluetooth_le(void);
extern void test_DeviceTrackerTest_source_type_gps(void);
extern void test_DeviceTrackerTest_source_type_router(void);
extern void test_DeviceTriggerTest_device_discovery_payload(void);
extern void test_DeviceTriggerTest_invalid_subtype(void);
extern void test_DeviceTriggerTest_invalid_subtype_progmem(void);
extern void test_DeviceTriggerTest_invalid_type(void);
extern void test_DeviceTriggerTest_invalid_type_progmem(void);
extern void test_DeviceTriggerTest_progmem_type_string_subtype(void);
extern void test_DeviceTriggerTest_string_type_progmem_subtype(void);
extern void test_DeviceTriggerTest_string_type_string_subtype(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_1(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_2(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_3(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_4(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_5(void);
extern void test_DeviceTriggerTest_subtype_progmem_button_6(void);
extern void test_DeviceTriggerTest_subtype_progmem_turn_off(void);
extern void test_DeviceTriggerTest_subtype_progmem_turn_on(void);
extern void test_DeviceTriggerTest_trigger(void);
extern void test_DeviceTriggerTest_trigger_progmem_subtype(void);
extern void test_DeviceTriggerTest_trigger_progmem_type(void);
extern void test_DeviceTriggerTest_trigger_progmem_type_subtype(void);
extern void test_DeviceTriggerTest_type_progmem_button_double_press(void);
extern void test_DeviceTriggerTest_type_progmem_button_long_press(void);
extern void test_DeviceTriggerTest_type_progmem_button_long_release(void);
extern void test_DeviceTriggerTest_type_progmem_button_quadruple_press(void);
extern void test_DeviceTriggerTest_type_progmem_button_quintuple_press(void);
extern void test_DeviceTriggerTest_type_progmem_button_short_press(void);
extern void test_DeviceTriggerTest_type_progmem_button_short_release(void);
extern void test_DeviceTriggerTest_type_progmem_button_triple_press(void);
extern void test_DeviceTriggerTest_unique_id_generator(void);
extern void test_SceneTest_availability(void);
extern void test_SceneTest_command_callback(void);
extern void test_SceneTest_command_subscription(void);
extern void test_SceneTest_default_entity_id_setter(void);
extern void test_SceneTest_default_params(void);
extern void test_SceneTest_device_discovery_payload(void);
extern void test_SceneTest_different_scene_command(void);
extern void test_SceneTest_entity_category_setter(void);
extern void test_SceneTest_extended_unique_id(void);
extern void test_SceneTest_icon_setter(void);
extern void test_SceneTest_invalid_unique_id(void);
extern void test_SceneTest_name_setter(void);
extern void test_SceneTest_no_command_callback(void);
extern void test_SceneTest_object_id_setter(void);
extern void test_SceneTest_retain_setter(void);
extern void test_StdFunctionCallbacksTest_button_command_callback(void);
extern void test_StdFunctionCallbacksTest_cover_command_callback(void);
extern void test_StdFunctionCallbacksTest_fan_command_callbacks(void);
extern void test_StdFunctionCallbacksTest_hvac_command_callbacks(void);
extern void test_StdFunctionCallbacksTest_light_command_callbacks(void);
extern void test_StdFunctionCallbacksTest_lock_command_callback(void);
extern void test_StdFunctionCallbacksTest_number_command_callback(void);
extern void test_StdFunctionCallbacksTest_scene_command_callback(void);
extern void test_StdFunctionCallbacksTest_select_command_callback(void);
extern void test_StdFunctionCallbacksTest_switch_command_callback(void);
extern void test_StdFunctionCallbacksTest_text_command_callback(void);
extern void test_TagScannerTest_default_params(void);
extern void test_TagScannerTest_device_discovery_payload(void);
extern void test_TagScannerTest_empty_tag_scanned(void);
extern void test_TagScannerTest_invalid_unique_id(void);
extern void test_TagScannerTest_nullptr_tag_scanned(void);
extern void test_TagScannerTest_tag_scanned(void);

#endif
