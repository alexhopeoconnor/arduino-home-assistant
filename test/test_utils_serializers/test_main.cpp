#include <unity.h>
#include <Arduino.h>
#include "test_main.h"

static TestCase tests[] = {
    TEST_ENTRY(test_UtilsTest_ends_with_null_str),
    TEST_ENTRY(test_UtilsTest_ends_with_null_suffix),
    TEST_ENTRY(test_UtilsTest_ends_with_valid_1),
    TEST_ENTRY(test_UtilsTest_ends_with_valid_2),
    TEST_ENTRY(test_UtilsTest_ends_with_valid_3),
    TEST_ENTRY(test_UtilsTest_ends_with_invalid_1),
    TEST_ENTRY(test_UtilsTest_ends_with_invalid_2),
    TEST_ENTRY(test_UtilsTest_ends_with_invalid_3),
    TEST_ENTRY(test_UtilsTest_ends_with_invalid_4),

    TEST_ENTRY(test_SerializerTest_empty_json),
    TEST_ENTRY(test_SerializerTest_skip_null_fields),
    TEST_ENTRY(test_SerializerTest_char_field),
    TEST_ENTRY(test_SerializerTest_bool_false_field),
    TEST_ENTRY(test_SerializerTest_bool_true_field),
    TEST_ENTRY(test_SerializerTest_number_zero_field),
    TEST_ENTRY(test_SerializerTest_number_signed_field),
    TEST_ENTRY(test_SerializerTest_number_unsigned_field),
    TEST_ENTRY(test_SerializerTest_float_p1_field),
    TEST_ENTRY(test_SerializerTest_float_p2_field),
    TEST_ENTRY(test_SerializerTest_float_p3_field),
    TEST_ENTRY(test_SerializerTest_float_p3_zero_signed_field),
    TEST_ENTRY(test_SerializerTest_float_p3_zero_unsigned_field),
    TEST_ENTRY(test_SerializerTest_progmem_field),
    TEST_ENTRY(test_SerializerTest_topic_field),
    TEST_ENTRY(test_SerializerTest_topics_field),
    TEST_ENTRY(test_SerializerTest_device_serialization),
    TEST_ENTRY(test_SerializerTest_device_mixed_serialization),
    TEST_ENTRY(test_SerializerTest_device_type_availability),
    TEST_ENTRY(test_SerializerTest_device_type_availability_mixed),
    TEST_ENTRY(test_SerializerTest_shared_availability),
    TEST_ENTRY(test_SerializerTest_empty_array),
    TEST_ENTRY(test_SerializerTest_two_element_array),
    TEST_ENTRY(test_SerializerTest_mixed_elements),

    TEST_ENTRY(test_SerializerArrayTest_empty_array),
    TEST_ENTRY(test_SerializerArrayTest_single_element_progmem),
    TEST_ENTRY(test_SerializerArrayTest_multiple_elements_progmem),
    TEST_ENTRY(test_SerializerArrayTest_size_overflow_progmem),
    TEST_ENTRY(test_SerializerArrayTest_single_element_ram),
    TEST_ENTRY(test_SerializerArrayTest_multiple_elements_ram),

    TEST_ENTRY(test_SerializerTopicsTest_calculate_config_no_mqtt),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_config_invalid_component),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_config_invalid_object),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_config_invalid_prefix),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_config),
    TEST_ENTRY(test_SerializerTopicsTest_generate_config_no_mqtt),
    TEST_ENTRY(test_SerializerTopicsTest_generate_config_invalid_component),
    TEST_ENTRY(test_SerializerTopicsTest_generate_config_invalid_object),
    TEST_ENTRY(test_SerializerTopicsTest_generate_config_invalid_prefix),
    TEST_ENTRY(test_SerializerTopicsTest_generate_config),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_data_no_mqtt),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_data_invalid_topic),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_data_invalid_prefix),
    TEST_ENTRY(test_SerializerTopicsTest_calculate_data_partial),
    TEST_ENTRY(test_SerializerTopicsTest_generate_data_no_mqtt),
    TEST_ENTRY(test_SerializerTopicsTest_generate_data_invalid_topic),
    TEST_ENTRY(test_SerializerTopicsTest_generate_data_invalid_prefix),
    TEST_ENTRY(test_SerializerTopicsTest_generate_data_partial),
    TEST_ENTRY(test_SerializerTopicsTest_generate_data_full),
    TEST_ENTRY(test_SerializerTopicsTest_compare_invalid_topic),
    TEST_ENTRY(test_SerializerTopicsTest_compare_matching_topics),
    TEST_ENTRY(test_SerializerTopicsTest_compare_not_matching_topics),
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
