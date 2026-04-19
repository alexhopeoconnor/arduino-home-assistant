/**
 * Unity helpers for ArduinoHA tests (replaces legacy AUnit macros).
 * Include after <Arduino.h>, then <ArduinoHA.h>, before test bodies.
 */
#ifndef AHA_UNITY_HELPERS_H
#define AHA_UNITY_HELPERS_H

#include <Arduino.h>
#include <unity.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include "mocks/PubSubClientMock.h"

#ifdef __cplusplus

static inline void ahaAssertRamEqualsFlash(const char* ram, const __FlashStringHelper* flash)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(ram, "actual string");
    TEST_ASSERT_NOT_NULL_MESSAGE(flash, "expected flash");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        0,
        strcmp_P(ram, reinterpret_cast<const char*>(flash)),
        "strcmp_P RAM vs flash"
    );
}

static inline void ahaAssertEqualPtr(const void* a, const void* b)
{
    TEST_ASSERT_EQUAL_PTR(a, b);
}

/** Compare subscription topic (RAM) to expected PROGMEM topic. */
static inline void ahaAssertFlashTopicEq(const __FlashStringHelper* expectedFlash, const char* actualRam)
{
    ahaAssertRamEqualsFlash(actualRam, expectedFlash);
}

#endif /* __cplusplus */

#define AHA_ASSERT_EQUAL_FLASH_TOPIC(flashExpected, ramActual) \
    ahaAssertFlashTopicEq((const __FlashStringHelper*)(flashExpected), (ramActual))

/** (expected, actual, tolerance) — matches common AUnit assertNear ordering. */
#define AHA_ASSERT_NEAR_FLOAT(expected, actual, tolerance) \
    TEST_ASSERT_FLOAT_WITHIN((tolerance), (expected), (actual))

/**
 * initMqttTest(testDeviceId)
 * Declares: PubSubClientMock* mock; HADevice device; HAMqtt mqtt;
 */
#define initMqttTest(testDeviceId) \
    PubSubClientMock* mock = new PubSubClientMock(); \
    HADevice device(testDeviceId); \
    HAMqtt mqtt(mock, device); \
    mqtt.setDataPrefix("testData"); \
    mqtt.begin("testHost", "testUser", "testPass");

#define AHA_ASSERT_NO_MQTT_MESSAGE(mockPtr) \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, (mockPtr)->getFlushedMessagesNb(), "expected no MQTT messages")

/** Topic expected as PROGMEM (__FlashStringHelper*). */
#define AHA_ASSERT_MQTT_MESSAGE(mockPtr, index, eTopicFlash, eMessageLiteral, eRetained) \
    do { \
        const __FlashStringHelper* messageP = F(eMessageLiteral); \
        const size_t messageLen = strlen_P(reinterpret_cast<const char*>(messageP)); \
        TEST_ASSERT_TRUE_MESSAGE((mockPtr)->getFlushedMessagesNb() > 0, "flushed count"); \
        TEST_ASSERT_TRUE_MESSAGE((mockPtr)->getFlushedMessagesNb() > (index), "index in range"); \
        MqttMessage* publishedMessage = (mockPtr)->getFlushedMessages()[(index)]; \
        ahaAssertRamEqualsFlash(publishedMessage->topic, (const __FlashStringHelper*)(eTopicFlash)); \
        ahaAssertRamEqualsFlash(publishedMessage->buffer, messageP); \
        TEST_ASSERT_EQUAL_UINT32_MESSAGE( \
            static_cast<uint32_t>(messageLen), \
            static_cast<uint32_t>(publishedMessage->bufferSize - 1), \
            "payload length"); \
        TEST_ASSERT_EQUAL_INT_MESSAGE( \
            (eRetained) ? 1 : 0, \
            publishedMessage->retained ? 1 : 0, \
            "retained flag"); \
    } while (0)

/** Topic expected as plain RAM C string (e.g. serializer tests). */
#define AHA_ASSERT_MQTT_MESSAGE_RAM_TOPIC(mockPtr, index, topicRam, eMessageLiteral, eRetained) \
    do { \
        const __FlashStringHelper* messageP = F(eMessageLiteral); \
        const size_t messageLen = strlen_P(reinterpret_cast<const char*>(messageP)); \
        TEST_ASSERT_TRUE_MESSAGE((mockPtr)->getFlushedMessagesNb() > 0, "flushed count"); \
        TEST_ASSERT_TRUE_MESSAGE((mockPtr)->getFlushedMessagesNb() > (index), "index in range"); \
        MqttMessage* publishedMessage = (mockPtr)->getFlushedMessages()[(index)]; \
        TEST_ASSERT_EQUAL_STRING_MESSAGE((topicRam), publishedMessage->topic, "topic"); \
        ahaAssertRamEqualsFlash(publishedMessage->buffer, messageP); \
        TEST_ASSERT_EQUAL_UINT32_MESSAGE( \
            static_cast<uint32_t>(messageLen), \
            static_cast<uint32_t>(publishedMessage->bufferSize - 1), \
            "payload length"); \
        TEST_ASSERT_EQUAL_INT_MESSAGE( \
            (eRetained) ? 1 : 0, \
            publishedMessage->retained ? 1 : 0, \
            "retained flag"); \
    } while (0)

#define AHA_ASSERT_SINGLE_MQTT_MESSAGE(mockPtr, eTopicFlash, eMessageLiteral, eRetained) \
    do { \
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, (mockPtr)->getFlushedMessagesNb(), "single message"); \
        AHA_ASSERT_MQTT_MESSAGE((mockPtr), 0, (eTopicFlash), (eMessageLiteral), (eRetained)); \
    } while (0)

#define AHA_ASSERT_SINGLE_MQTT_MESSAGE_RAM_TOPIC(mockPtr, topicRam, eMessageLiteral, eRetained) \
    do { \
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, (mockPtr)->getFlushedMessagesNb(), "single message"); \
        AHA_ASSERT_MQTT_MESSAGE_RAM_TOPIC((mockPtr), 0, (topicRam), (eMessageLiteral), (eRetained)); \
    } while (0)

/**
 * Requires: HAMqtt mqtt; entity variable named as second argument; mock from initMqttTest.
 * ConfigTopic must be visible (include ArduinoHA.h first).
 */
#define AHA_ASSERT_ENTITY_CONFIG(mockPtr, entityVar, expectedJsonLiteral) \
    do { \
        (void)(mockPtr); \
        mqtt.loop(); \
        AHA_ASSERT_MQTT_MESSAGE((mockPtr), 0, AHATOFSTR(ConfigTopic), (expectedJsonLiteral), true); \
        TEST_ASSERT_NULL_MESSAGE((entityVar).getSerializer(), "serializer cleared"); \
    } while (0)

#define AHA_ASSERT_ENTITY_CONFIG_ON_TOPIC(mockPtr, entityVar, topicFlash, expectedJsonLiteral) \
    do { \
        (void)(mockPtr); \
        mqtt.loop(); \
        AHA_ASSERT_MQTT_MESSAGE((mockPtr), 0, (topicFlash), (expectedJsonLiteral), true); \
        TEST_ASSERT_NULL_MESSAGE((entityVar).getSerializer(), "serializer cleared"); \
    } while (0)

#endif /* AHA_UNITY_HELPERS_H */
