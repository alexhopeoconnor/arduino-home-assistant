#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

void test_UtilsTest_ends_with_null_str(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith(nullptr, "test"));
}

void test_UtilsTest_ends_with_null_suffix(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith("test", nullptr));
}

void test_UtilsTest_ends_with_valid_1(void)
{
    TEST_ASSERT_TRUE(HAUtils::endsWith("test", "st"));
}

void test_UtilsTest_ends_with_valid_2(void)
{
    TEST_ASSERT_TRUE(HAUtils::endsWith("test/abc", "/abc"));
}

void test_UtilsTest_ends_with_valid_3(void)
{
    TEST_ASSERT_TRUE(HAUtils::endsWith("test123", "3"));
}

void test_UtilsTest_ends_with_invalid_1(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith("test", "ST"));
}

void test_UtilsTest_ends_with_invalid_2(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith("test", ""));
}

void test_UtilsTest_ends_with_invalid_3(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith("test123", "2"));
}

void test_UtilsTest_ends_with_invalid_4(void)
{
    TEST_ASSERT_FALSE(HAUtils::endsWith("test", "testtest"));
}
