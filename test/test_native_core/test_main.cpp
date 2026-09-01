#include <Arduino.h>
#include <string.h>
#include <unity.h>

#include "HADevice.h"
#include "HAMqtt.h"
#include "device-types/HABaseDeviceType.h"
#include "device-types/HAText.h"
#include "mocks/PubSubClientMock.h"
#include "utils/HAAvailabilityConfig.h"
#include "utils/HAJson.h"
#include "utils/HASerializer.h"
#include "utils/HASerializerArray.h"

namespace {
class NativeSerializerEntity : public HABaseDeviceType
{
public:
    explicit NativeSerializerEntity(const char* uniqueId) :
        HABaseDeviceType(F("sensor"), uniqueId)
    {

    }

protected:
    void onMqttConnected() override { }
};
class NativeDiscoveryEntity : public HABaseDeviceType
{
public:
    explicit NativeDiscoveryEntity(const char* uniqueId) :
        HABaseDeviceType(F("sensor"), uniqueId)
    {

    }

protected:
    void buildSerializer() override
    {
        if (_serializer) {
            return;
        }

        _serializer = new HASerializer(this, 2);
        _serializer->set(F("name"), uniqueId());
        _serializer->set(HASerializer::WithUniqueId);
    }

    HASerializer* buildDeviceDiscoverySerializer() override
    {
        HASerializer* serializer = new HASerializer(this, 3);
        serializer->set(
            F("p"),
            componentName(),
            HASerializer::ProgmemPropertyValue
        );
        serializer->set(F("name"), uniqueId());
        serializer->set(HASerializer::WithUniqueId);
        return serializer;
    }

    bool supportsDeviceDiscovery() const override
    {
        return true;
    }

    void onMqttConnected() override { }
};

uint8_t disconnectedCallbackCalls = 0;
uint8_t stateCallbackCalls = 0;

void onNativeDisconnected()
{
    disconnectedCallbackCalls++;
}

void onNativeStateChanged(HAMqtt::ConnectionState)
{
    stateCallbackCalls++;
}


void test_json_helpers_escape_control_bytes_and_preserve_cursor_contract()
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
    TEST_ASSERT_EQUAL_PTR(output + strlen(output), cursor);
}

void test_availability_and_serializer_array_escape_json_values()
{
    HAAvailabilityConfig availability;
    TEST_ASSERT_TRUE(availability.add("availability/\"main\"", "{{ value_json.\\n }}", "on\nline", "off\\line"));

    const uint16_t availabilitySize = availability.calculateJsonSize();
    char availabilityJson[availabilitySize + 1];
    TEST_ASSERT_TRUE(availability.serialize(availabilityJson));
    TEST_ASSERT_EQUAL_STRING(
        "[{\"t\":\"availability/\\\"main\\\"\",\"val_tpl\":\"{{ value_json.\\\\n }}\",\"pl_avail\":\"on\\nline\",\"pl_not_avail\":\"off\\\\line\"}]",
        availabilityJson
    );
    TEST_ASSERT_EQUAL_UINT16(strlen(availabilityJson), availabilitySize);

    HASerializerArray values(2, false);
    TEST_ASSERT_TRUE(values.add("first\"item"));
    TEST_ASSERT_TRUE(values.add("second\nitem"));
    const uint16_t valuesSize = values.calculateSize();
    char valuesJson[valuesSize + 1];
    TEST_ASSERT_TRUE(values.serialize(valuesJson));
    TEST_ASSERT_EQUAL_STRING("[\"first\\\"item\",\"second\\nitem\"]", valuesJson);
    TEST_ASSERT_EQUAL_UINT16(strlen(valuesJson), valuesSize);
}

void test_discovery_topic_tokens_are_rejected_before_topic_generation()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);

    const __FlashStringHelper* component = F("sensor");
    const uint16_t topicSize = HASerializer::calculateConfigTopicLength(component, "valid_entity-1");
    char topic[topicSize];
    TEST_ASSERT_TRUE(HASerializer::generateConfigTopic(topic, component, "valid_entity-1"));
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/valid_entity-1/config", topic);

    TEST_ASSERT_EQUAL_UINT16(0, HASerializer::calculateConfigTopicLength(component, "invalid/entity"));
    TEST_ASSERT_FALSE(HASerializer::generateConfigTopic(topic, component, "invalid entity"));
}

void test_streaming_serializer_writes_exact_escaped_payload_to_mqtt_mock()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);
    NativeSerializerEntity entity("native_entity");

    TEST_ASSERT_TRUE(mqtt.begin("native-host"));
    TEST_ASSERT_TRUE(mock->connectDummy());

    HASerializer serializer(&entity, 2);
    serializer.set(F("name"), "Name \"quoted\"\\line\nnext");
    serializer.set(F("unit_of_meas"), "C\tunit");

    const uint16_t payloadSize = serializer.calculateSize();
    TEST_ASSERT_NOT_EQUAL(0, payloadSize);
    TEST_ASSERT_TRUE(mqtt.beginPublish("native/serializer", payloadSize, true));
    TEST_ASSERT_TRUE(serializer.flush());
    TEST_ASSERT_TRUE(mqtt.endPublish());

    TEST_ASSERT_EQUAL_UINT8(1, mock->getFlushedMessagesNb());
    MqttMessage* message = mock->getFlushedMessages()[0];
    TEST_ASSERT_EQUAL_STRING("native/serializer", message->topic);
    TEST_ASSERT_EQUAL_STRING(
        "{\"name\":\"Name \\\"quoted\\\"\\\\line\\nnext\",\"unit_of_meas\":\"C\\tunit\"}",
        message->buffer
    );
    TEST_ASSERT_EQUAL_UINT16(strlen(message->buffer), message->writtenSize);
    TEST_ASSERT_EQUAL_UINT16(payloadSize, message->writtenSize);
}
void test_staged_migration_orders_markers_device_payload_and_legacy_cleanup()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);
    NativeDiscoveryEntity first("first");
    NativeDiscoveryEntity second("second");
    TEST_ASSERT_TRUE(mock->connectDummy());

    TEST_ASSERT_TRUE(mqtt.beginDeviceDiscoveryMigration());
    TEST_ASSERT_EQUAL_INT(HAMqtt::DeviceDiscoveryMigrationMarkersPending,
        mqtt.getDeviceDiscoveryMigrationState());
    TEST_ASSERT_FALSE(mqtt.publishDeviceDiscovery());
    TEST_ASSERT_EQUAL_UINT8(0, mock->getFlushedMessagesNb());

    TEST_ASSERT_TRUE(mqtt.publishDeviceDiscoveryMigrationMarkers());
    TEST_ASSERT_EQUAL_UINT8(2, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/first/config",
        mock->getFlushedMessages()[0]->topic);
    TEST_ASSERT_EQUAL_STRING("{\"migrate_discovery\":true}",
        mock->getFlushedMessages()[0]->buffer);
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/second/config",
        mock->getFlushedMessages()[1]->topic);

    TEST_ASSERT_TRUE(mqtt.publishDeviceDiscoveryMigrationConfig());
    TEST_ASSERT_EQUAL_UINT8(3, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("homeassistant/device/native_device/config",
        mock->getFlushedMessages()[2]->topic);
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[2]->buffer, "\"cmps\""));
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[2]->buffer, "\"first\""));
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[2]->buffer, "\"second\""));

    TEST_ASSERT_TRUE(mqtt.completeDeviceDiscoveryMigration());
    TEST_ASSERT_EQUAL_UINT8(5, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/first/config",
        mock->getFlushedMessages()[3]->topic);
    TEST_ASSERT_EQUAL_STRING("", mock->getFlushedMessages()[3]->buffer);
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/second/config",
        mock->getFlushedMessages()[4]->topic);
    TEST_ASSERT_EQUAL_STRING("", mock->getFlushedMessages()[4]->buffer);
    TEST_ASSERT_EQUAL_INT(HAMqtt::DeviceDiscoveryMigrationCompleted,
        mqtt.getDeviceDiscoveryMigrationState());

    TEST_ASSERT_TRUE(mqtt.rollbackDeviceDiscoveryMigration());
    TEST_ASSERT_EQUAL_UINT8(9, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL_STRING("homeassistant/device/native_device/config",
        mock->getFlushedMessages()[5]->topic);
    TEST_ASSERT_EQUAL_STRING("{\"migrate_discovery\":true}",
        mock->getFlushedMessages()[5]->buffer);
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/first/config",
        mock->getFlushedMessages()[6]->topic);
    TEST_ASSERT_EQUAL_STRING("homeassistant/sensor/native_device/second/config",
        mock->getFlushedMessages()[7]->topic);
    TEST_ASSERT_EQUAL_STRING("homeassistant/device/native_device/config",
        mock->getFlushedMessages()[8]->topic);
    TEST_ASSERT_EQUAL_STRING("", mock->getFlushedMessages()[8]->buffer);
    TEST_ASSERT_EQUAL_INT(HAMqtt::DeviceDiscoveryMigrationIdle,
        mqtt.getDeviceDiscoveryMigrationState());
    TEST_ASSERT_FALSE(mqtt.isDeviceDiscoveryEnabled());
}

void test_failed_rollback_stays_staged_until_it_can_complete()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);
    NativeDiscoveryEntity entity("entity");
    TEST_ASSERT_TRUE(mock->connectDummy());

    TEST_ASSERT_TRUE(mqtt.beginDeviceDiscoveryMigration());
    TEST_ASSERT_TRUE(mqtt.publishDeviceDiscoveryMigrationMarkers());
    TEST_ASSERT_TRUE(mqtt.publishDeviceDiscoveryMigrationConfig());

    mock->failNextBeginPublish();
    TEST_ASSERT_FALSE(mqtt.rollbackDeviceDiscoveryMigration());
    TEST_ASSERT_EQUAL_INT(HAMqtt::DeviceDiscoveryMigrationRollbackPending,
        mqtt.getDeviceDiscoveryMigrationState());
    TEST_ASSERT_TRUE(mqtt.isDeviceDiscoveryMigrationInProgress());
    TEST_ASSERT_FALSE(mqtt.publishDeviceDiscovery());

    TEST_ASSERT_TRUE(mqtt.rollbackDeviceDiscoveryMigration());
    TEST_ASSERT_EQUAL_INT(HAMqtt::DeviceDiscoveryMigrationIdle,
        mqtt.getDeviceDiscoveryMigrationState());
    TEST_ASSERT_FALSE(mqtt.isDeviceDiscoveryEnabled());
}

void test_device_component_removal_uses_marker_then_omission_and_readd()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);
    NativeDiscoveryEntity first("first");
    NativeDiscoveryEntity second("second");
    TEST_ASSERT_TRUE(mock->connectDummy());
    mqtt.enableDeviceDiscovery();

    TEST_ASSERT_TRUE(first.removeFromDiscovery());
    TEST_ASSERT_TRUE(first.isRemovedFromDeviceDiscovery());
    TEST_ASSERT_EQUAL_UINT8(2, mock->getFlushedMessagesNb());
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[0]->buffer,
        "\"first\":{\"p\":\"sensor\"}"));
    TEST_ASSERT_NULL(strstr(mock->getFlushedMessages()[1]->buffer, "\"first\""));
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[1]->buffer, "\"second\""));

    TEST_ASSERT_TRUE(first.republishDiscovery());
    TEST_ASSERT_FALSE(first.isRemovedFromDeviceDiscovery());
    TEST_ASSERT_EQUAL_UINT8(3, mock->getFlushedMessagesNb());
    TEST_ASSERT_NOT_NULL(strstr(mock->getFlushedMessages()[2]->buffer, "\"first\""));
}

void test_entities_before_or_after_mqtt_have_safe_registration_lifetimes()
{
    NativeDiscoveryEntity before("before");
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device, 2);
    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getRegisteredDeviceTypeCount());

    NativeDiscoveryEntity* after = new NativeDiscoveryEntity("after");
    TEST_ASSERT_EQUAL_UINT8(2, mqtt.getRegisteredDeviceTypeCount());
    delete after;
    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getRegisteredDeviceTypeCount());
}

void test_text_state_is_bounded_and_retains_the_last_valid_value()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device);
    HAText text("text");
    char oversizedState[HAText::MaxCommandLength + 2];
    memset(oversizedState, 'x', sizeof(oversizedState) - 1);
    oversizedState[sizeof(oversizedState) - 1] = 0;

    text.setCurrentState("initial");
    text.setCurrentState(oversizedState);
    TEST_ASSERT_EQUAL_STRING("initial", text.getCurrentState());
    TEST_ASSERT_FALSE(text.setState(oversizedState));
    TEST_ASSERT_EQUAL_STRING("initial", text.getCurrentState());
}

void test_registration_cap_and_explicit_disconnect_are_reported()
{
    HADevice device("native_device");
    PubSubClientMock* mock = new PubSubClientMock();
    HAMqtt mqtt(mock, device, 1);
    NativeDiscoveryEntity first("first");
    NativeDiscoveryEntity second("second");
    TEST_ASSERT_EQUAL_UINT8(1, mqtt.getRegisteredDeviceTypeCount());
    TEST_ASSERT_EQUAL_UINT16(1, mqtt.getDeviceTypeRegistrationFailures());

    disconnectedCallbackCalls = 0;
    stateCallbackCalls = 0;
    mqtt.onDisconnected(onNativeDisconnected);
    mqtt.onStateChanged(onNativeStateChanged);
    TEST_ASSERT_TRUE(mqtt.begin("native-host"));
    TEST_ASSERT_TRUE(mock->connectDummy());
    mock->setState(HAMqtt::StateConnected);
    mqtt.loop();
    TEST_ASSERT_TRUE(mqtt.disconnect());
    TEST_ASSERT_EQUAL_UINT8(1, disconnectedCallbackCalls);
    TEST_ASSERT_TRUE(stateCallbackCalls >= 2);
}

} // namespace

void setUp(void) { }
void tearDown(void) { }

int main(int, char**)
{
    UNITY_BEGIN();
    RUN_TEST(test_json_helpers_escape_control_bytes_and_preserve_cursor_contract);
    RUN_TEST(test_availability_and_serializer_array_escape_json_values);
    RUN_TEST(test_discovery_topic_tokens_are_rejected_before_topic_generation);
    RUN_TEST(test_streaming_serializer_writes_exact_escaped_payload_to_mqtt_mock);
    RUN_TEST(test_staged_migration_orders_markers_device_payload_and_legacy_cleanup);
    RUN_TEST(test_failed_rollback_stays_staged_until_it_can_complete);
    RUN_TEST(test_device_component_removal_uses_marker_then_omission_and_readd);
    RUN_TEST(test_entities_before_or_after_mqtt_have_safe_registration_lifetimes);
    RUN_TEST(test_text_state_is_bounded_and_retains_the_last_valid_value);
    RUN_TEST(test_registration_cap_and_explicit_disconnect_are_reported);
    return UNITY_END();
}
