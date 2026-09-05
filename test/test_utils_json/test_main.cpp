#include <Arduino.h>
#include <string.h>
#include <unity.h>

#include "utils/HAJson.h"

using TestFn = void (*)(void);

struct TestCase {
    const char* name;
    TestFn fn;
    uint16_t line;
};

#define TEST_ENTRY(fn) { #fn, fn, __LINE__ }

void test_HAJson_escapes_all_required_json_characters()
{
    const char value[] = "quote\" slash\\ newline\n tab\t control\x01";
    char output[96] = {};
    char* cursor = output;

    TEST_ASSERT_EQUAL_UINT16(
        strlen("\"quote\\\" slash\\\\ newline\\n tab\\t control\\u0001\""),
        HAJson::calculateEscapedStringSize(value)
    );
    TEST_ASSERT_TRUE(HAJson::appendEscapedString(cursor, output + sizeof(output) - 1, value));
    TEST_ASSERT_EQUAL_STRING(
        "\"quote\\\" slash\\\\ newline\\n tab\\t control\\u0001\"",
        output
    );
}

void test_HAJson_rejects_too_small_output_buffer_without_partial_output()
{
    char output[5] = "ok";
    char* cursor = output;

    TEST_ASSERT_FALSE(HAJson::appendEscapedString(cursor, output + sizeof(output) - 1, "toolong"));
    TEST_ASSERT_EQUAL_STRING("ok", output);
    TEST_ASSERT_EQUAL_PTR(output, cursor);
}

void test_HAJson_validates_home_assistant_discovery_topic_tokens()
{
    TEST_ASSERT_TRUE(HAJson::isValidDiscoveryTopicToken("device_01-A"));
    TEST_ASSERT_FALSE(HAJson::isValidDiscoveryTopicToken(""));
    TEST_ASSERT_FALSE(HAJson::isValidDiscoveryTopicToken("device/id"));
    TEST_ASSERT_FALSE(HAJson::isValidDiscoveryTopicToken("device id"));
    TEST_ASSERT_FALSE(HAJson::isValidDiscoveryTopicToken("device\n"));
}

static TestCase tests[] = {
    TEST_ENTRY(test_HAJson_escapes_all_required_json_characters),
    TEST_ENTRY(test_HAJson_rejects_too_small_output_buffer_without_partial_output),
    TEST_ENTRY(test_HAJson_validates_home_assistant_discovery_topic_tokens),
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);
static size_t nextTest = 0;
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
    if (begun && nextTest < testCount) {
        TestCase& test = tests[nextTest++];
        UnityDefaultTestRun(test.fn, test.name, test.line);
        return;
    }

    if (begun) {
        UNITY_END();
        begun = false;
    }
}
