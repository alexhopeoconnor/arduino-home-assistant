#include <Arduino.h>
#include <unity.h>

#include <ArduinoHA.h>
#include "mocks/PubSubClientMock.h"

using TestFn = void (*)(void);

struct TestCase {
    const char* name;
    TestFn fn;
    uint16_t line;
};

#define TEST_ENTRY(fn) { #fn, fn, __LINE__ }

static void flushDeviceSerializer(
    PubSubClientMock* mock,
    const HADevice& device
)
{
    mock->connectDummy();
    const HASerializer* serializer = device.getSerializer();
    TEST_ASSERT_NOT_NULL(serializer);
    TEST_ASSERT_TRUE(mock->beginPublish("test/device/config", serializer->calculateSize(), true));
    TEST_ASSERT_TRUE(serializer->flush());
    TEST_ASSERT_TRUE(mock->endPublish());
}

void test_DeviceMetadata_add_connection_escapes_json()
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device("testDevice");
    HAMqtt mqtt(mock, device);

    TEST_ASSERT_TRUE(device.addConnection("mac", "aa\"\\bb\n"));
    flushDeviceSerializer(mock, device);

    TEST_ASSERT_EQUAL_UINT8(1, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING(
        "{\"ids\":\"testDevice\",\"cns\":[[\"mac\",\"aa\\\"\\\\bb\\n\"]]}",
        mock->getFlushedMessages()[0]->buffer
    );
}

void test_DeviceMetadata_raw_connections_reject_malformed_input()
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device("testDevice");
    HAMqtt mqtt(mock, device);

    TEST_ASSERT_FALSE(device.setConnectionsJson("[[\"mac\",invalid]]"));
    flushDeviceSerializer(mock, device);

    TEST_ASSERT_EQUAL_STRING(
        "{\"ids\":\"testDevice\"}",
        mock->getFlushedMessages()[0]->buffer
    );
}

void test_DeviceMetadata_raw_connections_accept_complete_tuple_array()
{
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device("testDevice");
    HAMqtt mqtt(mock, device);

    TEST_ASSERT_TRUE(device.setConnectionsJson("[[\"mac\",\"aa:bb\"],[\"serial\",\"42\"]]"));
    flushDeviceSerializer(mock, device);

    TEST_ASSERT_EQUAL_STRING(
        "{\"ids\":\"testDevice\",\"cns\":[[\"mac\",\"aa:bb\"],[\"serial\",\"42\"]]}",
        mock->getFlushedMessages()[0]->buffer
    );
}

static TestCase tests[] = {
    TEST_ENTRY(test_DeviceMetadata_add_connection_escapes_json),
    TEST_ENTRY(test_DeviceMetadata_raw_connections_reject_malformed_input),
    TEST_ENTRY(test_DeviceMetadata_raw_connections_accept_complete_tuple_array),
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
