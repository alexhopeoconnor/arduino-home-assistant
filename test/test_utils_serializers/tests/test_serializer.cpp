#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static const char* testTopic = "testTopic";
const char TestComponentStr[] PROGMEM = {"dummyProgmem"};

class DummyDeviceType : public HABaseDeviceType
{
public:
    DummyDeviceType() : HABaseDeviceType(AHATOFSTR(TestComponentStr), "testId") { }

protected:
    virtual void onMqttConnected() override { }
};

#define prepareTest(maxEntriesNb) \
    initMqttTest(testDeviceId); \
    DummyDeviceType dummyDeviceType; \
    HASerializer serializer(&dummyDeviceType, maxEntriesNb)

#define flushSerializer(mockPtr, ser) \
    (mockPtr)->connectDummy(); \
    (mockPtr)->beginPublish(testTopic, (ser).calculateSize(), false); \
    (ser).flush(); \
    (mockPtr)->endPublish()

#define assertSerializerMqttMessage(expectedJson) \
    AHA_ASSERT_SINGLE_MQTT_MESSAGE_RAM_TOPIC(mock, testTopic, expectedJson, false)

void test_SerializerTest_empty_json(void)
{
    prepareTest(0);
    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{}");
}

void test_SerializerTest_skip_null_fields(void)
{
    prepareTest(2);

    serializer.set(AHATOFSTR(HADeviceClassProperty), "Class");
    serializer.set(AHATOFSTR(HAIconProperty), nullptr);

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"dev_cla\":\"Class\"}");
}

void test_SerializerTest_char_field(void)
{
    prepareTest(1);

    serializer.set(AHATOFSTR(HANameProperty), "XYZ");

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":\"XYZ\"}");
}

void test_SerializerTest_bool_false_field(void)
{
    prepareTest(1);

    bool value = false;
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::BoolPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":false}");
}

void test_SerializerTest_bool_true_field(void)
{
    prepareTest(1);

    bool value = true;
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::BoolPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":true}");
}

void test_SerializerTest_number_zero_field(void)
{
    prepareTest(1);

    HANumeric value(0, 0);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":0}");
}

void test_SerializerTest_number_signed_field(void)
{
    prepareTest(1);

    HANumeric value(-12346756, 0);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":-12346756}");
}

void test_SerializerTest_number_unsigned_field(void)
{
    prepareTest(1);

    HANumeric value(312346733, 0);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":312346733}");
}

void test_SerializerTest_float_p1_field(void)
{
    prepareTest(1);

    HANumeric value(250.5235f, 1);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":250.5}");
}

void test_SerializerTest_float_p2_field(void)
{
    prepareTest(1);

    HANumeric value(250.5235f, 2);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":250.52}");
}

void test_SerializerTest_float_p3_field(void)
{
    prepareTest(1);

    HANumeric value(250.5235f, 3);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":250.523}");
}

void test_SerializerTest_float_p3_zero_signed_field(void)
{
    prepareTest(1);

    HANumeric value(-0.243f, 3);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":-0.243}");
}

void test_SerializerTest_float_p3_zero_unsigned_field(void)
{
    prepareTest(1);

    HANumeric value(0.243f, 3);
    serializer.set(
        AHATOFSTR(HANameProperty),
        &value,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":0.243}");
}

void test_SerializerTest_progmem_field(void)
{
    prepareTest(1);

    serializer.set(
        AHATOFSTR(HANameProperty),
        HAOffline,
        HASerializer::ProgmemPropertyValue
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"name\":\"offline\"}");
}

void test_SerializerTest_topic_field(void)
{
    prepareTest(1);

    serializer.topic(AHATOFSTR(HAStateTopic));

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{\"stat_t\":\"testData/testDevice/testId/stat_t\"}"
    );
}

void test_SerializerTest_topics_field(void)
{
    prepareTest(2);

    serializer.topic(AHATOFSTR(HAStateTopic));
    serializer.topic(AHATOFSTR(HAAvailabilityTopic));

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{"
        "\"stat_t\":\"testData/testDevice/testId/stat_t\","
        "\"avty_t\":\"testData/testDevice/testId/avty_t\""
        "}"
    );
}

void test_SerializerTest_device_serialization(void)
{
    prepareTest(1);

    serializer.set(HASerializer::WithDevice);

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"dev\":{\"ids\":\"testDevice\"}}");
}

void test_SerializerTest_device_mixed_serialization(void)
{
    prepareTest(2);

    serializer.set(HASerializer::WithDevice);
    serializer.set(AHATOFSTR(HADeviceClassProperty), "Class1");

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{\"dev\":{\"ids\":\"testDevice\"},\"dev_cla\":\"Class1\"}"
    );
}

void test_SerializerTest_device_type_availability(void)
{
    prepareTest(1);

    dummyDeviceType.setAvailability(false);
    serializer.set(HASerializer::WithAvailability);

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{\"avty_t\":\"testData/testDevice/testId/avty_t\"}"
    );
}

void test_SerializerTest_device_type_availability_mixed(void)
{
    prepareTest(2);

    dummyDeviceType.setAvailability(false);
    serializer.set(HASerializer::WithAvailability);
    serializer.set(AHATOFSTR(HADeviceClassProperty), "Class1");

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{"
        "\"avty_t\":\"testData/testDevice/testId/avty_t\","
        "\"dev_cla\":\"Class1\""
        "}"
    );
}

void test_SerializerTest_shared_availability(void)
{
    prepareTest(1);

    device.enableSharedAvailability();
    serializer.set(HASerializer::WithAvailability);

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"avty_t\":\"testData/testDevice/avty_t\"}");
}

void test_SerializerTest_empty_array(void)
{
    prepareTest(1);

    HASerializerArray array(0);
    serializer.set(
        AHATOFSTR(HADeviceClassProperty),
        &array,
        HASerializer::ArrayPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"dev_cla\":[]}");
}

void test_SerializerTest_two_element_array(void)
{
    prepareTest(1);

    HASerializerArray array(2);
    array.add(HADeviceProperty);
    array.add(HAIconProperty);
    serializer.set(
        AHATOFSTR(HADeviceClassProperty),
        &array,
        HASerializer::ArrayPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage("{\"dev_cla\":[\"dev\",\"ic\"]}");
}

void test_SerializerTest_mixed_elements(void)
{
    prepareTest(6);

    HASerializerArray array(2);
    array.add(HADeviceProperty);
    array.add(HAIconProperty);

    dummyDeviceType.setAvailability(false);
    serializer.set(
        AHATOFSTR(HADeviceClassProperty),
        &array,
        HASerializer::ArrayPropertyType
    );
    serializer.set(HASerializer::WithAvailability);
    serializer.set(HASerializer::WithDevice);
    serializer.set(AHATOFSTR(HANameProperty), "TestName");
    serializer.topic(AHATOFSTR(HAStateTopic));

    HANumeric intValue(312346733, 0);
    serializer.set(
        AHATOFSTR(HAIconProperty),
        &intValue,
        HASerializer::NumberPropertyType
    );

    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{"
        "\"dev_cla\":[\"dev\",\"ic\"],"
        "\"avty_t\":\"testData/testDevice/testId/avty_t\","
        "\"dev\":{\"ids\":\"testDevice\"},"
        "\"name\":\"TestName\","
        "\"stat_t\":\"testData/testDevice/testId/stat_t\","
        "\"ic\":312346733"
        "}"
    );
}
