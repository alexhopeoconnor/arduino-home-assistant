#ifndef AHA_TEST_MAIN_SHARD02_H
#define AHA_TEST_MAIN_SHARD02_H

#include <Arduino.h>

using TestFn = void (*)(void);

struct TestCase {
    const char* name;
    TestFn fn;
    uint16_t line;
};

#define TEST_ENTRY(fn) { #fn, fn, __LINE__ }

extern void test_MqttTest_maximum_number_of_device_types(void);
extern void test_MqttTest_reconnect_interval_default_value(void);
extern void test_MqttTest_reconnect_interval_setter(void);
extern void test_MqttTest_reconnect_interval_throttles_attempts(void);
extern void test_MqttTest_publish_from_message_callback_is_deferred_and_flushed(void);
extern void test_MqttTest_deferred_publish_order_is_preserved(void);
extern void test_MqttTest_processing_message_flag_only_wraps_inbound_dispatch(void);
extern void test_MqttTest_streaming_publish_from_message_callback_is_deferred_and_flushed(void);
extern void test_MqttTest_deferred_publish_is_kept_across_disconnect_and_retried_from_loop(void);
extern void test_MqttTest_failed_deferred_flush_keeps_queue_and_retries_in_order(void);
extern void test_MqttTest_mixed_deferred_publish_order_is_preserved(void);

extern void test_DeviceTest_default_unique_id(void);
extern void test_DeviceTest_unique_id_constructor_char(void);
extern void test_DeviceTest_unique_id_constructor_byte_array(void);
extern void test_DeviceTest_unique_id_setter(void);
extern void test_DeviceTest_unique_id_setter_runtime(void);
extern void test_DeviceTest_serializer_no_unique_id(void);
extern void test_DeviceTest_serializer_unique_id_constructor_char(void);
extern void test_DeviceTest_serializer_unique_id_constructor_byte_array(void);
extern void test_DeviceTest_serializer_unique_id_setter(void);
extern void test_DeviceTest_serializer_manufacturer(void);
extern void test_DeviceTest_serializer_model(void);
extern void test_DeviceTest_serializer_name(void);
extern void test_DeviceTest_serializer_software_version(void);
extern void test_DeviceTest_default_availability(void);
extern void test_DeviceTest_enable_availability(void);
extern void test_DeviceTest_enable_availability_no_unique_id(void);
extern void test_DeviceTest_availability_publish_offline(void);
extern void test_DeviceTest_availability_publish_online(void);
extern void test_DeviceTest_extended_unique_ids_disabled(void);
extern void test_DeviceTest_enable_extended_unique_ids(void);
extern void test_DeviceTest_lwt_disabled(void);
extern void test_DeviceTest_lwt_enabled(void);
extern void test_DeviceTest_full_serialization(void);

extern void test_BaseDeviceTypeTest_constructor_params(void);
extern void test_BaseDeviceTypeTest_register_mqtt_type(void);
extern void test_BaseDeviceTypeTest_default_name(void);
extern void test_BaseDeviceTypeTest_name_setter(void);
extern void test_BaseDeviceTypeTest_object_id_setter(void);
extern void test_BaseDeviceTypeTest_default_entity_id_setter(void);
extern void test_BaseDeviceTypeTest_default_availability(void);
extern void test_BaseDeviceTypeTest_publish_nothing_if_not_configured(void);
extern void test_BaseDeviceTypeTest_publish_availability_online_runtime(void);
extern void test_BaseDeviceTypeTest_publish_availability_offline_runtime(void);
extern void test_BaseDeviceTypeTest_publish_shared_availability_on_connect(void);
extern void test_BaseDeviceTypeTest_publish_shared_availability_runtime(void);
extern void test_BaseDeviceTypeTest_republish_discovery(void);
extern void test_BaseDeviceTypeTest_remove_from_discovery(void);

#endif
