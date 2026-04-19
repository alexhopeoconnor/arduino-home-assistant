#include <Arduino.h>
#include <ArduinoHA.h>
#include <string.h>
#include "../../shared/aha_unity_helpers.h"

static char tmpBuffer[32];

#define assertJson(expectedJson, arrayVar) \
    do { \
        const __FlashStringHelper* expectedJsonP = F(expectedJson); \
        memset(tmpBuffer, 0, sizeof(tmpBuffer)); \
        TEST_ASSERT_TRUE((arrayVar).serialize(tmpBuffer)); \
        ahaAssertRamEqualsFlash(tmpBuffer, expectedJsonP); \
        TEST_ASSERT_EQUAL_UINT16( \
            (uint16_t)strlen_P(reinterpret_cast<const char*>(expectedJsonP)), \
            (arrayVar).calculateSize()); \
    } while (0)

void test_SerializerArrayTest_empty_array(void)
{
    HASerializerArray array(0);

    TEST_ASSERT_EQUAL(0, array.getItemsNb());
    assertJson("[]", array);
}

void test_SerializerArrayTest_single_element_progmem(void)
{
    HASerializerArray array(1);
    bool result = array.add(HANameProperty);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, array.getItemsNb());
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)HANameProperty,
        (uint32_t)(uintptr_t)&(*array.getItems()[0]));
    assertJson("[\"name\"]", array);
}

void test_SerializerArrayTest_multiple_elements_progmem(void)
{
    HASerializerArray array(3);

    TEST_ASSERT_TRUE(array.add(HANameProperty));
    TEST_ASSERT_TRUE(array.add(HADeviceManufacturerProperty));
    TEST_ASSERT_TRUE(array.add(HAUniqueIdProperty));
    TEST_ASSERT_EQUAL(3, array.getItemsNb());

    HASerializerArray::ItemType* items = array.getItems();
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)HANameProperty,
        (uint32_t)(uintptr_t)&(*items[0]));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)HADeviceManufacturerProperty,
        (uint32_t)(uintptr_t)&(*items[1]));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)HAUniqueIdProperty,
        (uint32_t)(uintptr_t)&(*items[2]));
    assertJson("[\"name\",\"mf\",\"uniq_id\"]", array);
}

void test_SerializerArrayTest_size_overflow_progmem(void)
{
    HASerializerArray array(1);

    TEST_ASSERT_TRUE(array.add(HANameProperty));
    TEST_ASSERT_FALSE(array.add(HAUniqueIdProperty));
    TEST_ASSERT_EQUAL(1, array.getItemsNb());

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)HANameProperty,
        (uint32_t)(uintptr_t)&(*array.getItems()[0]));
    assertJson("[\"name\"]", array);
}

void test_SerializerArrayTest_single_element_ram(void)
{
    const char* item = "test";
    HASerializerArray array(1, false);
    bool result = array.add(item);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, array.getItemsNb());

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item,
        (uint32_t)(uintptr_t)&(*array.getItem(0)));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item,
        (uint32_t)(uintptr_t)&(*array.getItems()[0]));
    assertJson("[\"test\"]", array);
}

void test_SerializerArrayTest_multiple_elements_ram(void)
{
    const char* item0 = "item0";
    const char* item1 = "item1";
    const char* item2 = "item2";
    HASerializerArray array(3, false);

    TEST_ASSERT_TRUE(array.add(item0));
    TEST_ASSERT_TRUE(array.add(item1));
    TEST_ASSERT_TRUE(array.add(item2));
    TEST_ASSERT_EQUAL(3, array.getItemsNb());

    HASerializerArray::ItemType* items = array.getItems();

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)nullptr,
        (uint32_t)(uintptr_t)&(*array.getItem(-1)));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)nullptr,
        (uint32_t)(uintptr_t)&(*array.getItem(3)));

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item0,
        (uint32_t)(uintptr_t)&(*array.getItem(0)));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item0,
        (uint32_t)(uintptr_t)&(*items[0]));

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item1,
        (uint32_t)(uintptr_t)&(*array.getItem(1)));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item1,
        (uint32_t)(uintptr_t)&(*items[1]));

    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item2,
        (uint32_t)(uintptr_t)&(*array.getItem(2)));
    TEST_ASSERT_EQUAL_UINT32(
        (uint32_t)(uintptr_t)item2,
        (uint32_t)(uintptr_t)&(*items[2]));
    assertJson("[\"item0\",\"item1\",\"item2\"]", array);
}
