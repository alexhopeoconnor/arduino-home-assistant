#include <Arduino.h>
#include <ArduinoHA.h>
#include <string.h>
#include "../../shared/aha_unity_helpers.h"

static const char* deviceId = "testDevice";
static const char* dataPrefix = "dataPrefix";
static const char* discoveryPrefix = "discoveryPrefix";

const char DummyProgmemStr[] PROGMEM = {"dummyProgmem"};
const char ComponentNameStr[] PROGMEM = {"componentName"};

static char tmpBuffer[64];

static void clearTmpBuffer()
{
    memset(tmpBuffer, 0, sizeof(tmpBuffer));
}

void test_SerializerTopicsTest_calculate_config_no_mqtt(void)
{
    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateConfigTopicLength(
            AHATOFSTR(ComponentNameStr),
            "objectId"
        ));
}

void test_SerializerTopicsTest_calculate_config_invalid_component(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateConfigTopicLength(
            nullptr,
            "objectId"
        ));
}

void test_SerializerTopicsTest_calculate_config_invalid_object(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateConfigTopicLength(
            AHATOFSTR(ComponentNameStr),
            nullptr
        ));
}

void test_SerializerTopicsTest_calculate_config_invalid_prefix(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDiscoveryPrefix(nullptr);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateConfigTopicLength(
            AHATOFSTR(ComponentNameStr),
            "objectId"
        ));
}

void test_SerializerTopicsTest_calculate_config(void)
{
    const char* objectId = "objectId";
    const char* expectedTopic = "discoveryPrefix/componentName/testDevice/objectId/config";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDiscoveryPrefix(discoveryPrefix);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)(strlen(expectedTopic) + 1),
        HASerializer::calculateConfigTopicLength(
            AHATOFSTR(ComponentNameStr),
            objectId
        ));
}

void test_SerializerTopicsTest_generate_config_no_mqtt(void)
{
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateConfigTopic(
        tmpBuffer,
        AHATOFSTR(ComponentNameStr),
        "objectId"
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_config_invalid_component(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateConfigTopic(
        tmpBuffer,
        nullptr,
        "objectId"
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_config_invalid_object(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateConfigTopic(
        tmpBuffer,
        AHATOFSTR(ComponentNameStr),
        nullptr
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_config_invalid_prefix(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDiscoveryPrefix(nullptr);
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateConfigTopic(
        tmpBuffer,
        AHATOFSTR(ComponentNameStr),
        "objectId"
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_config(void)
{
    const char* objectId = "objectId";
    const char* expectedTopic = "discoveryPrefix/componentName/testDevice/objectId/config";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDiscoveryPrefix(discoveryPrefix);
    clearTmpBuffer();

    TEST_ASSERT_TRUE(HASerializer::generateConfigTopic(
        tmpBuffer,
        AHATOFSTR(ComponentNameStr),
        objectId
    ));
    TEST_ASSERT_EQUAL_STRING(expectedTopic, tmpBuffer);
}

void test_SerializerTopicsTest_calculate_data_no_mqtt(void)
{
    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateDataTopicLength(
            "objectId",
            AHATOFSTR(DummyProgmemStr)
        ));
}

void test_SerializerTopicsTest_calculate_data_invalid_topic(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateDataTopicLength(
            "objectId",
            nullptr
        ));
}

void test_SerializerTopicsTest_calculate_data_invalid_prefix(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(nullptr);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)0,
        HASerializer::calculateDataTopicLength(
            "objectId",
            AHATOFSTR(DummyProgmemStr)
        ));
}

void test_SerializerTopicsTest_calculate_data_partial(void)
{
    const char* objectId = nullptr;
    const char* expectedTopic = "dataPrefix/testDevice/dummyProgmem";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);

    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)strlen(expectedTopic) + 1,
        HASerializer::calculateDataTopicLength(
            objectId,
            AHATOFSTR(DummyProgmemStr)
        ));
}

void test_SerializerTopicsTest_generate_data_no_mqtt(void)
{
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateDataTopic(
        tmpBuffer,
        "objectId",
        AHATOFSTR(DummyProgmemStr)
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_data_invalid_topic(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateDataTopic(
        tmpBuffer,
        "objectId",
        nullptr
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_data_invalid_prefix(void)
{
    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(nullptr);
    clearTmpBuffer();

    TEST_ASSERT_FALSE(HASerializer::generateDataTopic(
        tmpBuffer,
        "objectId",
        AHATOFSTR(DummyProgmemStr)
    ));
    TEST_ASSERT_TRUE(strlen(tmpBuffer) == 0);
}

void test_SerializerTopicsTest_generate_data_partial(void)
{
    const char* objectId = nullptr;
    const char* expectedTopic = "dataPrefix/testDevice/dummyProgmem";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);
    clearTmpBuffer();

    TEST_ASSERT_TRUE(HASerializer::generateDataTopic(
        tmpBuffer,
        objectId,
        AHATOFSTR(DummyProgmemStr)
    ));
    TEST_ASSERT_EQUAL_STRING(tmpBuffer, expectedTopic);
}

void test_SerializerTopicsTest_generate_data_full(void)
{
    const char* objectId = "objectId";
    const char* expectedTopic = "dataPrefix/testDevice/objectId/dummyProgmem";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);
    clearTmpBuffer();

    TEST_ASSERT_TRUE(HASerializer::generateDataTopic(
        tmpBuffer,
        objectId,
        AHATOFSTR(DummyProgmemStr)
    ));
    TEST_ASSERT_EQUAL_STRING(tmpBuffer, expectedTopic);
}

void test_SerializerTopicsTest_compare_invalid_topic(void)
{
    const char* topic = nullptr;
    const char* objectId = "objectId";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);

    TEST_ASSERT_FALSE(HASerializer::compareDataTopics(
        topic,
        objectId,
        AHATOFSTR(DummyProgmemStr)
    ));
}

void test_SerializerTopicsTest_compare_matching_topics(void)
{
    const char* topic = "dataPrefix/testDevice/objectId/dummyProgmem";
    const char* objectId = "objectId";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);

    TEST_ASSERT_TRUE(HASerializer::compareDataTopics(
        topic,
        objectId,
        AHATOFSTR(DummyProgmemStr)
    ));
}

void test_SerializerTopicsTest_compare_not_matching_topics(void)
{
    const char* topic = "dataPrefix/testDevice/objectId/Progmem";
    const char* objectId = "objectId";

    HADevice device(deviceId);
    HAMqtt mqtt(nullptr, device);
    mqtt.setDataPrefix(dataPrefix);

    TEST_ASSERT_FALSE(HASerializer::compareDataTopics(
        topic,
        objectId,
        AHATOFSTR(DummyProgmemStr)
    ));
}
