#ifndef AHA_TEST_MAIN_SHARD01_H
#define AHA_TEST_MAIN_SHARD01_H

#include <Arduino.h>

using TestFn = void (*)(void);

struct TestCase {
    const char* name;
    TestFn fn;
    uint16_t line;
};

#define TEST_ENTRY(fn) { #fn, fn, __LINE__ }

// test_utils.cpp
extern void test_UtilsTest_ends_with_null_str(void);
extern void test_UtilsTest_ends_with_null_suffix(void);
extern void test_UtilsTest_ends_with_valid_1(void);
extern void test_UtilsTest_ends_with_valid_2(void);
extern void test_UtilsTest_ends_with_valid_3(void);
extern void test_UtilsTest_ends_with_invalid_1(void);
extern void test_UtilsTest_ends_with_invalid_2(void);
extern void test_UtilsTest_ends_with_invalid_3(void);
extern void test_UtilsTest_ends_with_invalid_4(void);

// test_serializer.cpp
extern void test_SerializerTest_empty_json(void);
extern void test_SerializerTest_skip_null_fields(void);
extern void test_SerializerTest_char_field(void);
extern void test_SerializerTest_bool_false_field(void);
extern void test_SerializerTest_bool_true_field(void);
extern void test_SerializerTest_number_zero_field(void);
extern void test_SerializerTest_number_signed_field(void);
extern void test_SerializerTest_number_unsigned_field(void);
extern void test_SerializerTest_float_p1_field(void);
extern void test_SerializerTest_float_p2_field(void);
extern void test_SerializerTest_float_p3_field(void);
extern void test_SerializerTest_float_p3_zero_signed_field(void);
extern void test_SerializerTest_float_p3_zero_unsigned_field(void);
extern void test_SerializerTest_progmem_field(void);
extern void test_SerializerTest_topic_field(void);
extern void test_SerializerTest_topics_field(void);
extern void test_SerializerTest_device_serialization(void);
extern void test_SerializerTest_device_mixed_serialization(void);
extern void test_SerializerTest_device_type_availability(void);
extern void test_SerializerTest_device_type_availability_mixed(void);
extern void test_SerializerTest_shared_availability(void);
extern void test_SerializerTest_empty_array(void);
extern void test_SerializerTest_two_element_array(void);
extern void test_SerializerTest_mixed_elements(void);

// test_serializer_array.cpp
extern void test_SerializerArrayTest_empty_array(void);
extern void test_SerializerArrayTest_single_element_progmem(void);
extern void test_SerializerArrayTest_multiple_elements_progmem(void);
extern void test_SerializerArrayTest_size_overflow_progmem(void);
extern void test_SerializerArrayTest_single_element_ram(void);
extern void test_SerializerArrayTest_multiple_elements_ram(void);

// test_serializer_topics.cpp
extern void test_SerializerTopicsTest_calculate_config_no_mqtt(void);
extern void test_SerializerTopicsTest_calculate_config_invalid_component(void);
extern void test_SerializerTopicsTest_calculate_config_invalid_object(void);
extern void test_SerializerTopicsTest_calculate_config_invalid_prefix(void);
extern void test_SerializerTopicsTest_calculate_config(void);
extern void test_SerializerTopicsTest_generate_config_no_mqtt(void);
extern void test_SerializerTopicsTest_generate_config_invalid_component(void);
extern void test_SerializerTopicsTest_generate_config_invalid_object(void);
extern void test_SerializerTopicsTest_generate_config_invalid_prefix(void);
extern void test_SerializerTopicsTest_generate_config(void);
extern void test_SerializerTopicsTest_calculate_data_no_mqtt(void);
extern void test_SerializerTopicsTest_calculate_data_invalid_topic(void);
extern void test_SerializerTopicsTest_calculate_data_invalid_prefix(void);
extern void test_SerializerTopicsTest_calculate_data_partial(void);
extern void test_SerializerTopicsTest_generate_data_no_mqtt(void);
extern void test_SerializerTopicsTest_generate_data_invalid_topic(void);
extern void test_SerializerTopicsTest_generate_data_invalid_prefix(void);
extern void test_SerializerTopicsTest_generate_data_partial(void);
extern void test_SerializerTopicsTest_generate_data_full(void);
extern void test_SerializerTopicsTest_compare_invalid_topic(void);
extern void test_SerializerTopicsTest_compare_matching_topics(void);
extern void test_SerializerTopicsTest_compare_not_matching_topics(void);

#endif
