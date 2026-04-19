#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define assertNumberSize(value, precision, expectedSize) \
{ \
    HANumeric number(value, precision); \
    TEST_ASSERT_TRUE(number.isSet()); \
    TEST_ASSERT_EQUAL(expectedSize, number.calculateSize()); \
}

#define assertNumberToStr(value, precision, expectedStr) \
{ \
    memset(tmpBuffer, 0, sizeof(tmpBuffer)); \
    HANumeric number(value, precision); \
    const uint16_t expectedLength = expectedStr ? strlen(expectedStr) : 0; \
    const uint16_t writtenLength = number.toStr(tmpBuffer); \
    AHA_ASSERT_EQUAL_FLASH_TOPIC(F(expectedStr), tmpBuffer); \
    TEST_ASSERT_EQUAL(expectedLength, writtenLength); \
}

#define assertStrToNumber(expected, str) \
{ \
    HANumeric number = HANumeric::fromStr( \
        reinterpret_cast<const uint8_t*>(str), \
        str ? strlen(str) : 0 \
    ); \
    TEST_ASSERT_TRUE(number.isSet()); \
    TEST_ASSERT_EQUAL((int64_t)expected, number.getBaseValue()); \
}

#define assertStrToNumberInvalid(str) \
{ \
    HANumeric number = HANumeric::fromStr( \
        reinterpret_cast<const uint8_t*>(str), \
        str ? strlen(str) : 0 \
    ); \
    TEST_ASSERT_FALSE(number.isSet()); \
}

char tmpBuffer[32];

void test_NumericTest_calculate_number_zero(void) {
    assertNumberSize(0, 0, 1)
}

void test_NumericTest_calculate_number_signed_one_digit(void) {
    assertNumberSize(-8, 0, 2)
}

void test_NumericTest_calculate_number_unsigned_one_digit(void) {
    assertNumberSize(8, 0, 1)
}

void test_NumericTest_calculate_number_signed_two_digits(void) {
    assertNumberSize(-81, 0, 3)
}

void test_NumericTest_calculate_number_unsigned_two_digits(void) {
    assertNumberSize(86, 0, 2)
}

void test_NumericTest_calculate_number_signed_three_digits(void) {
    assertNumberSize(-100, 0, 4)
}

void test_NumericTest_calculate_number_unsigned_three_digits(void) {
    assertNumberSize(100, 0, 3)
}

void test_NumericTest_calculate_number_signed_four_digits(void) {
    assertNumberSize(-1234, 0, 5)
}

void test_NumericTest_calculate_number_unsigned_four_digits(void) {
    assertNumberSize(1234, 0, 4)
}

void test_NumericTest_calculate_float_p1_zero(void) {
    assertNumberSize(0.0f, 1, 1)
}

void test_NumericTest_calculate_float_p1_zero_signed(void) {
    assertNumberSize(-0.123f, 1, 4)
}

void test_NumericTest_calculate_float_p1_zero_unsigned(void) {
    assertNumberSize(0.123f, 1, 3)
}

void test_NumericTest_calculate_float_p1_small(void) {
    assertNumberSize(1.0f, 1, 3)
}

void test_NumericTest_calculate_float_p1_medium(void) {
    assertNumberSize(50.5f, 1, 4)
}

void test_NumericTest_calculate_float_p1_large(void) {
    assertNumberSize(5526.02f, 1, 6)
}

void test_NumericTest_calculate_float_p1_unsigned(void) {
    assertNumberSize(-5526.02f, 1, 7)
}

void test_NumericTest_calculate_float_p2_zero(void) {
    assertNumberSize(0.0f, 2, 1)
}

void test_NumericTest_calculate_float_p2_zero_signed(void) {
    assertNumberSize(-0.123f, 2, 5)
}

void test_NumericTest_calculate_float_p2_zero_with_padding(void) {
    assertNumberSize(0.012f, 2, 4)
}

void test_NumericTest_calculate_float_p2_zero_unsigned(void) {
    assertNumberSize(0.123f, 2, 4)
}

void test_NumericTest_calculate_float_p2_small(void) {
    assertNumberSize(1.0f, 2, 4)
}

void test_NumericTest_calculate_float_p2_medium(void) {
    assertNumberSize(50.5f, 2, 5)
}

void test_NumericTest_calculate_float_p2_large(void) {
    assertNumberSize(5526.5f, 2, 7)
}

void test_NumericTest_calculate_float_p2_unsigned(void) {
    assertNumberSize(-5526.12f, 2, 8)
}

void test_NumericTest_calculate_float_p3_zero(void) {
    assertNumberSize(0.0f, 3, 1)
}

void test_NumericTest_calculate_float_p3_zero_signed(void) {
    assertNumberSize(-0.123f, 3, 6)
}

void test_NumericTest_calculate_float_p3_zero_with_padding(void) {
    assertNumberSize(0.012f, 3, 5)
}

void test_NumericTest_calculate_float_p3_zero_unsigned(void) {
    assertNumberSize(0.123f, 3, 5)
}

void test_NumericTest_calculate_float_p3_small(void) {
    assertNumberSize(1.0f, 3, 5)
}

void test_NumericTest_calculate_float_p3_medium(void) {
    assertNumberSize(50.5f, 3, 6)
}

void test_NumericTest_calculate_float_p3_large(void) {
    assertNumberSize(5526.5f, 3, 8)
}

void test_NumericTest_calculate_float_p3_unsigned(void) {
    assertNumberSize(-5526.12f, 3, 9)
}

void test_NumericTest_number_to_str_zero_p0(void) {
    assertNumberToStr(0, 0, "0");
}

void test_NumericTest_number_to_str_zero_p1(void) {
    assertNumberToStr(0, 1, "0");
}

void test_NumericTest_number_to_str_zero_p2(void) {
    assertNumberToStr(0, 2, "0");
}

void test_NumericTest_number_to_str_zero_p3(void) {
    assertNumberToStr(0, 3, "0");
}

void test_NumericTest_number_to_str_int8_p0(void) {
    int8_t value = -8;
    assertNumberToStr(value, 0, "-8");
}

void test_NumericTest_number_to_str_int8_p1(void) {
    int8_t value = -66;
    assertNumberToStr(value, 1, "-66.0");
}

void test_NumericTest_number_to_str_int8_p2(void) {
    int8_t value = 123;
    assertNumberToStr(value, 2, "123.00");
}

void test_NumericTest_number_to_str_int8_p3(void) {
    int8_t value = -123;
    assertNumberToStr(value, 3, "-123.000");
}

void test_NumericTest_number_to_str_int16_p0(void) {
    int16_t value = -12345;
    assertNumberToStr(value, 0, "-12345");
}

void test_NumericTest_number_to_str_int16_p1(void) {
    int16_t value = 543;
    assertNumberToStr(value, 1, "543.0");
}

void test_NumericTest_number_to_str_int16_p2(void) {
    int16_t value = 1234;
    assertNumberToStr(value, 2, "1234.00");
}

void test_NumericTest_number_to_str_int16_p3(void) {
    int16_t value = -80;
    assertNumberToStr(value, 3, "-80.000");
}

void test_NumericTest_number_to_str_int32_p0(void) {
    int32_t value = -123456;
    assertNumberToStr(value, 0, "-123456");
}

void test_NumericTest_number_to_str_int32_p1(void) {
    int32_t value = 54345;
    assertNumberToStr(value, 1, "54345.0");
}

void test_NumericTest_number_to_str_int32_p2(void) {
    int32_t value = 1234;
    assertNumberToStr(value, 2, "1234.00");
}

void test_NumericTest_number_to_str_int32_p3(void) {
    int32_t value = -801;
    assertNumberToStr(value, 3, "-801.000");
}

void test_NumericTest_number_to_str_uint8_p0(void) {
    uint8_t value = 8;
    assertNumberToStr(value, 0, "8");
}

void test_NumericTest_number_to_str_uint8_p1(void) {
    uint8_t value = 66;
    assertNumberToStr(value, 1, "66.0");
}

void test_NumericTest_number_to_str_uint8_p2(void) {
    uint8_t value = 123;
    assertNumberToStr(value, 2, "123.00");
}

void test_NumericTest_number_to_str_uint8_p3(void) {
    uint8_t value = 255;
    assertNumberToStr(value, 3, "255.000");
}

void test_NumericTest_number_to_str_uint16_p0(void) {
    uint16_t value = 12345;
    assertNumberToStr(value, 0, "12345");
}

void test_NumericTest_number_to_str_uint16_p1(void) {
    uint16_t value = 543;
    assertNumberToStr(value, 1, "543.0");
}

void test_NumericTest_number_to_str_uint16_p2(void) {
    uint16_t value = 1234;
    assertNumberToStr(value, 2, "1234.00");
}

void test_NumericTest_number_to_str_uint16_p3(void) {
    uint16_t value = 80;
    assertNumberToStr(value, 3, "80.000");
}

void test_NumericTest_number_to_str_uint32_p0(void) {
    uint32_t value = 123456;
    assertNumberToStr(value, 0, "123456");
}

void test_NumericTest_number_to_str_uint32_p1(void) {
    uint32_t value = 54345;
    assertNumberToStr(value, 1, "54345.0");
}

void test_NumericTest_number_to_str_uint32_p2(void) {
    uint32_t value = 1234;
    assertNumberToStr(value, 2, "1234.00");
}

void test_NumericTest_number_to_str_uint32_p3(void) {
    uint32_t value = 801;
    assertNumberToStr(value, 3, "801.000");
}

void test_NumericTest_number_to_str_float_p0_zero(void) {
    assertNumberToStr(0.0f, 0, "0");
}

void test_NumericTest_number_to_str_float_p0_small(void) {
    assertNumberToStr(1.0f, 0, "1");
}

void test_NumericTest_number_to_str_float_p0_large(void) {
    assertNumberToStr(5526.5f, 0, "5526");
}

void test_NumericTest_number_to_str_float_p0_signed(void) {
    assertNumberToStr(-5526.12f, 0, "-5526");
}

void test_NumericTest_number_to_str_float_p1_zero(void) {
    assertNumberToStr(0.0f, 1, "0");
}

void test_NumericTest_number_to_str_float_p1_zero_decimal(void) {
    assertNumberToStr(0.123f, 1, "0.1");
}

void test_NumericTest_number_to_str_float_p1_zero_decimal_signed(void) {
    assertNumberToStr(-0.123f, 1, "-0.1");
}

void test_NumericTest_number_to_str_float_p1_small(void) {
    assertNumberToStr(1.0f, 1, "1.0");
}

void test_NumericTest_number_to_str_float_p1_medium(void) {
    assertNumberToStr(50.5f, 1, "50.5");
}

void test_NumericTest_number_to_str_float_p1_large(void) {
    assertNumberToStr(5526.5f, 1, "5526.5");
}

void test_NumericTest_number_to_str_float_p1_signed(void) {
    assertNumberToStr(-5526.12f, 1, "-5526.1");
}

void test_NumericTest_number_to_str_float_p2_zero(void) {
    assertNumberToStr(0.0f, 2, "0");
}

void test_NumericTest_number_to_str_float_p2_zero_decimal(void) {
    assertNumberToStr(0.123f, 2, "0.12");
}

void test_NumericTest_number_to_str_float_p2_zero_with_padding(void) {
    assertNumberToStr(0.01f, 2, "0.01");
}

void test_NumericTest_number_to_str_float_p2_zero_decimal_signed(void) {
    assertNumberToStr(-0.123f, 2, "-0.12");
}

void test_NumericTest_number_to_str_float_p2_small(void) {
    assertNumberToStr(1.0f, 2, "1.00");
}

void test_NumericTest_number_to_str_float_p2_medium(void) {
    assertNumberToStr(50.50f, 2, "50.50");
}

void test_NumericTest_number_to_str_float_p2_large(void) {
    assertNumberToStr(5526.5f, 2, "5526.50");
}

void test_NumericTest_number_to_str_float_p2_signed(void) {
    assertNumberToStr(-5526.12f, 2, "-5526.12");
}

void test_NumericTest_number_to_str_float_p3_zero(void) {
    assertNumberToStr(0.0f, 3, "0");
}

void test_NumericTest_number_to_str_float_p3_zero_decimal(void) {
    assertNumberToStr(0.123f, 3, "0.123");
}

void test_NumericTest_number_to_str_float_p3_zero_with_padding(void) {
    assertNumberToStr(0.001f, 3, "0.001");
}

void test_NumericTest_number_to_str_float_p3_zero_decimal_signed(void) {
    assertNumberToStr(-0.123f, 3, "-0.123");
}

void test_NumericTest_number_to_str_float_p3_small(void) {
    assertNumberToStr(1.123f, 3, "1.123");
}

void test_NumericTest_number_to_str_float_p3_medium(void) {
    assertNumberToStr(50.555f, 3, "50.555");
}

void test_NumericTest_number_to_str_float_p3_large(void) {
    assertNumberToStr(5526.5f, 3, "5526.500");
}

void test_NumericTest_number_to_str_float_p3_signed(void) {
    assertNumberToStr(-5526.12456456f, 3, "-5526.124");
}

void test_NumericTest_str_to_number_max(void) {
    assertStrToNumber(9223372036854775807, "9223372036854775807");
}

void test_NumericTest_str_to_number_min(void) {
    assertStrToNumber(-9223372036854775807, "-9223372036854775807");
}

void test_NumericTest_str_to_number_zero(void) {
    assertStrToNumber(0, "0");
}

void test_NumericTest_str_to_number_signed_0(void) {
    assertStrToNumber(-1, "-1");
}

void test_NumericTest_str_to_number_signed_1(void) {
    assertStrToNumber(-12, "-12");
}

void test_NumericTest_str_to_number_signed_2(void) {
    assertStrToNumber(-123, "-123");
}

void test_NumericTest_str_to_number_signed_3(void) {
    assertStrToNumber(-1234, "-1234");
}

void test_NumericTest_str_to_number_signed_4(void) {
    assertStrToNumber(-1234, "-1234");
}

void test_NumericTest_str_to_number_signed_5(void) {
    assertStrToNumber(-12345, "-12345");
}

void test_NumericTest_str_to_number_signed_6(void) {
    assertStrToNumber(-123456, "-123456");
}

void test_NumericTest_str_to_number_signed_7(void) {
    assertStrToNumber(-1234567, "-1234567");
}

void test_NumericTest_str_to_number_signed_8(void) {
    assertStrToNumber(-12345678, "-12345678");
}

void test_NumericTest_str_to_number_signed_9(void) {
    assertStrToNumber(-123456789, "-123456789");
}

void test_NumericTest_str_to_number_signed_10(void) {
    assertStrToNumber(-1234567890, "-1234567890");
}

void test_NumericTest_str_to_number_signed_11(void) {
    assertStrToNumber(-12345678901, "-12345678901");
}

void test_NumericTest_str_to_number_signed_12(void) {
    assertStrToNumber(-123456789012, "-123456789012");
}

void test_NumericTest_str_to_number_signed_13(void) {
    assertStrToNumber(-1234567890123, "-1234567890123");
}

void test_NumericTest_str_to_number_signed_14(void) {
    assertStrToNumber(-123456789012345, "-123456789012345");
}

void test_NumericTest_str_to_number_signed_15(void) {
    assertStrToNumber(-1234567890123456, "-1234567890123456");
}

void test_NumericTest_str_to_number_unsigned_0(void) {
    assertStrToNumber(1, "1");
}

void test_NumericTest_str_to_number_unsigned_1(void) {
    assertStrToNumber(12, "12");
}

void test_NumericTest_str_to_number_unsigned_2(void) {
    assertStrToNumber(123, "123");
}

void test_NumericTest_str_to_number_unsigned_3(void) {
    assertStrToNumber(1234, "1234");
}

void test_NumericTest_str_to_number_unsigned_4(void) {
    assertStrToNumber(1234, "1234");
}

void test_NumericTest_str_to_number_unsigned_5(void) {
    assertStrToNumber(12345, "12345");
}

void test_NumericTest_str_to_number_unsigned_6(void) {
    assertStrToNumber(123456, "123456");
}

void test_NumericTest_str_to_number_unsigned_7(void) {
    assertStrToNumber(1234567, "1234567");
}

void test_NumericTest_str_to_number_unsigned_8(void) {
    assertStrToNumber(12345678, "12345678");
}

void test_NumericTest_str_to_number_unsigned_9(void) {
    assertStrToNumber(123456789, "123456789");
}

void test_NumericTest_str_to_number_unsigned_10(void) {
    assertStrToNumber(1234567890, "1234567890");
}

void test_NumericTest_str_to_number_unsigned_11(void) {
    assertStrToNumber(12345678901, "12345678901");
}

void test_NumericTest_str_to_number_unsigned_12(void) {
    assertStrToNumber(123456789012, "123456789012");
}

void test_NumericTest_str_to_number_unsigned_13(void) {
    assertStrToNumber(1234567890123, "1234567890123");
}

void test_NumericTest_str_to_number_unsigned_14(void) {
    assertStrToNumber(123456789012345, "123456789012345");
}

void test_NumericTest_str_to_number_unsigned_15(void) {
    assertStrToNumber(1234567890123456, "1234567890123456");
}

void test_NumericTest_str_to_number_null(void) {
    const char* num = nullptr;
    assertStrToNumberInvalid(num);
}

void test_NumericTest_str_to_number_unsigned_overflow(void) {
    assertStrToNumberInvalid("92233720368547758078");
}

void test_NumericTest_str_to_number_signed_overflow(void) {
    assertStrToNumberInvalid("-92233720368547758078");
}

void test_NumericTest_str_to_number_invalid_0(void) {
    assertStrToNumberInvalid("--12");
}

void test_NumericTest_str_to_number_invalid_1(void) {
    assertStrToNumberInvalid("a1");
}

void test_NumericTest_str_to_number_invalid_2(void) {
    assertStrToNumberInvalid("567a32");
}

void test_NumericTest_str_to_number_invalid_3(void) {
    assertStrToNumberInvalid("15.334");
}

void test_NumericTest_number_to_float_1(void) {
    AHA_ASSERT_NEAR_FLOAT(HANumeric(500, 0).toFloat(), 500.0, 0.01);
}

void test_NumericTest_number_to_float_2(void) {
    AHA_ASSERT_NEAR_FLOAT(HANumeric(50, 1).toFloat(), 50.0, 0.01);
}

void test_NumericTest_number_to_float_3(void) {
    AHA_ASSERT_NEAR_FLOAT(HANumeric(5, 2).toFloat(), 5.0, 0.01);
}

void test_NumericTest_number_to_float_4(void) {
    AHA_ASSERT_NEAR_FLOAT(HANumeric(0.5f, 3).toFloat(), 0.5, 0.01);
}

void test_NumericTest_number_to_float_5(void) {
    AHA_ASSERT_NEAR_FLOAT(HANumeric(-265.544f, 3).toFloat(), -265.544, 0.0001);
}

