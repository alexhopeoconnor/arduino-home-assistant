#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

static const char* testDeviceId = "testDevice";
static byte testDeviceByteId[] = {0x11, 0x22, 0x33, 0x44, 0xaa, 0xbb};
static const char* testDeviceByteIdChar = "11223344aabb";
static const char* dummyTopic = "dummyTopic";

const char AvailabilityTopic[] PROGMEM = {"testData/testDevice/avty_t"};

#define prepareMqttTest \
    initMqttTest(testDeviceId); \
    mock->connectDummy()

#define assertSerializerEntry(entry, eType, eSubtype, eProperty, eValue) \
    do { \
        TEST_ASSERT_EQUAL_INT((int)(eType), (int)(entry)->type); \
        TEST_ASSERT_EQUAL_UINT8((eSubtype), (entry)->subtype); \
        TEST_ASSERT_EQUAL_PTR((const void*)(eProperty), (const void*)(entry)->property); \
        TEST_ASSERT_EQUAL_STRING((const char*)(eValue), reinterpret_cast<const char*>((entry)->value)); \
    } while (0)

#define flushSerializer(mockPtr, serializerPtr) \
    (mockPtr)->connectDummy(); \
    (mockPtr)->beginPublish(dummyTopic, (serializerPtr)->calculateSize(), false); \
    (serializerPtr)->flush(); \
    (mockPtr)->endPublish()

#define assertSerializerMqttMessage(expectedJson) \
    AHA_ASSERT_SINGLE_MQTT_MESSAGE_RAM_TOPIC(mock, dummyTopic, expectedJson, false)

void test_DeviceTest_default_unique_id(void)
{
    HADevice device;
    TEST_ASSERT_NULL(device.getUniqueId());
}

void test_DeviceTest_unique_id_constructor_char(void)
{
    HADevice device(testDeviceId);
    TEST_ASSERT_EQUAL_STRING(testDeviceId, device.getUniqueId());
}

void test_DeviceTest_unique_id_constructor_byte_array(void)
{
    HADevice device(testDeviceByteId, sizeof(testDeviceByteId));
    TEST_ASSERT_EQUAL_STRING(testDeviceByteIdChar, device.getUniqueId());
}

void test_DeviceTest_unique_id_setter(void)
{
    HADevice device;
    TEST_ASSERT_NULL(device.getUniqueId());

    bool result = device.setUniqueId(testDeviceByteId, sizeof(testDeviceByteId));
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(testDeviceByteIdChar, device.getUniqueId());
}

void test_DeviceTest_unique_id_setter_runtime(void)
{
    HADevice device(testDeviceId);
    TEST_ASSERT_EQUAL_STRING(testDeviceId, device.getUniqueId());

    bool result = device.setUniqueId(testDeviceByteId, sizeof(testDeviceByteId));
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_STRING(testDeviceId, device.getUniqueId());
}

void test_DeviceTest_serializer_no_unique_id(void)
{
    HADevice device;
    const HASerializer* serializer = device.getSerializer();

    TEST_ASSERT_EQUAL_UINT8(0, serializer->getEntriesNb());
}

void test_DeviceTest_serializer_unique_id_constructor_char(void)
{
    HADevice device(testDeviceId);
    const HASerializer* serializer = device.getSerializer();

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceIdentifiersProperty,
        testDeviceId
    );
}

void test_DeviceTest_serializer_unique_id_constructor_byte_array(void)
{
    HADevice device(testDeviceId);
    const HASerializer* serializer = device.getSerializer();

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceIdentifiersProperty,
        device.getUniqueId()
    );
}

void test_DeviceTest_serializer_unique_id_setter(void)
{
    HADevice device;
    device.setUniqueId(testDeviceByteId, sizeof(testDeviceByteId));
    const HASerializer* serializer = device.getSerializer();

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceIdentifiersProperty,
        device.getUniqueId()
    );
}

void test_DeviceTest_serializer_manufacturer(void)
{
    const char* manufacturer = "testManufacturer";
    HADevice device;
    const HASerializer* serializer = device.getSerializer();

    device.setManufacturer(manufacturer);

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceManufacturerProperty,
        manufacturer
    );
}

void test_DeviceTest_serializer_model(void)
{
    const char* model = "testModel";
    HADevice device;
    const HASerializer* serializer = device.getSerializer();

    device.setModel(model);

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceModelProperty,
        model
    );
}

void test_DeviceTest_serializer_name(void)
{
    const char* name = "testName";
    HADevice device;
    const HASerializer* serializer = device.getSerializer();

    device.setName(name);

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HANameProperty,
        name
    );
}

void test_DeviceTest_serializer_software_version(void)
{
    const char* softwareVersion = "softwareVersion";
    HADevice device;
    const HASerializer* serializer = device.getSerializer();

    device.setSoftwareVersion(softwareVersion);

    TEST_ASSERT_EQUAL_UINT8(1, serializer->getEntriesNb());
    TEST_ASSERT_TRUE(serializer->getEntries() != nullptr);

    HASerializer::SerializerEntry* entries = serializer->getEntries();
    HASerializer::SerializerEntry* entry = &entries[0];

    assertSerializerEntry(
        entry,
        HASerializer::PropertyEntryType,
        HASerializer::ConstCharPropertyValue,
        HADeviceSoftwareVersionProperty,
        softwareVersion
    );
}

void test_DeviceTest_default_availability(void)
{
    HADevice device(testDeviceId);

    TEST_ASSERT_TRUE(device.isAvailable());
    TEST_ASSERT_FALSE(device.isSharedAvailabilityEnabled());
    TEST_ASSERT_NULL(device.getAvailabilityTopic());
}

void test_DeviceTest_enable_availability(void)
{
    prepareMqttTest;

    TEST_ASSERT_TRUE(device.enableSharedAvailability());
    TEST_ASSERT_TRUE(device.isSharedAvailabilityEnabled());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(AvailabilityTopic), device.getAvailabilityTopic());
    AHA_ASSERT_NO_MQTT_MESSAGE(mock);
}

void test_DeviceTest_enable_availability_no_unique_id(void)
{
    HADevice device;
    bool result = device.enableSharedAvailability();

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(device.isSharedAvailabilityEnabled());
    TEST_ASSERT_NULL(device.getAvailabilityTopic());
}

void test_DeviceTest_availability_publish_offline(void)
{
    prepareMqttTest;

    device.enableSharedAvailability();
    device.setAvailability(false);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AvailabilityTopic), "offline", true);
}

void test_DeviceTest_availability_publish_online(void)
{
    prepareMqttTest;

    device.enableSharedAvailability();
    device.setAvailability(true);

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(AvailabilityTopic), "online", true);
}

void test_DeviceTest_extended_unique_ids_disabled(void)
{
    prepareMqttTest;

    TEST_ASSERT_FALSE(device.isExtendedUniqueIdsEnabled());
}

void test_DeviceTest_enable_extended_unique_ids(void)
{
    prepareMqttTest;

    device.enableExtendedUniqueIds();

    TEST_ASSERT_TRUE(device.isExtendedUniqueIdsEnabled());
    AHA_ASSERT_NO_MQTT_MESSAGE(mock);
}

void test_DeviceTest_lwt_disabled(void)
{
    prepareMqttTest;

    TEST_ASSERT_NULL(mock->getLastWill().topic);
    TEST_ASSERT_NULL(mock->getLastWill().message);
}

void test_DeviceTest_lwt_enabled(void)
{
    initMqttTest(testDeviceId);

    device.enableSharedAvailability();
    device.enableLastWill();
    mqtt.loop();

    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(AvailabilityTopic), mock->getLastWill().topic);
    TEST_ASSERT_EQUAL_STRING("offline", mock->getLastWill().message);
    TEST_ASSERT_TRUE(mock->getLastWill().retain);
}

void test_DeviceTest_full_serialization(void)
{
    initMqttTest("myDeviceId");

    device.setManufacturer("myManufacturer");
    device.setModel("myModel");
    device.setName("myName");
    device.setSoftwareVersion("myVersion");
    device.setConfigurationUrl("http://1.1.1.1:1234");

    const HASerializer* serializer = device.getSerializer();
    flushSerializer(mock, serializer);
    assertSerializerMqttMessage(
        "{"
        "\"ids\":\"myDeviceId\","
        "\"mf\":\"myManufacturer\","
        "\"mdl\":\"myModel\","
        "\"name\":\"myName\","
        "\"sw\":\"myVersion\","
        "\"cu\":\"http://1.1.1.1:1234\""
        "}"
    );
}
