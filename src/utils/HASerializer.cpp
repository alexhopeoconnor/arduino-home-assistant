
#include <Arduino.h>

#ifdef ARDUINO_ARCH_SAMD
#include <avr/dtostrf.h>
#endif

#include "HASerializer.h"
#include <new>
#include "../ArduinoHADefines.h"
#include "../HADevice.h"
#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HANumeric.h"
#include "../utils/HAJson.h"
#include "../utils/HAAvailabilityConfig.h"
#include "../device-types/HABaseDeviceType.h"

namespace {
bool writeJsonEscapedByte(HAMqtt* mqtt, const uint8_t value)
{
    char output[6];
    uint8_t length = 0;

    switch (value) {
    case '"':
        output[0] = '\\';
        output[1] = '"';
        length = 2;
        break;
    case '\\':
        output[0] = '\\';
        output[1] = '\\';
        length = 2;
        break;
    case '\b':
        output[0] = '\\';
        output[1] = 'b';
        length = 2;
        break;
    case '\f':
        output[0] = '\\';
        output[1] = 'f';
        length = 2;
        break;
    case '\n':
        output[0] = '\\';
        output[1] = 'n';
        length = 2;
        break;
    case '\r':
        output[0] = '\\';
        output[1] = 'r';
        length = 2;
        break;
    case '\t':
        output[0] = '\\';
        output[1] = 't';
        length = 2;
        break;
    default:
        if (value < 0x20) {
            static const char hex[] = "0123456789ABCDEF";
            output[0] = '\\';
            output[1] = 'u';
            output[2] = '0';
            output[3] = '0';
            output[4] = hex[value >> 4];
            output[5] = hex[value & 0x0F];
            length = 6;
        } else {
            output[0] = static_cast<char>(value);
            length = 1;
        }
        break;
    }

    return mqtt && mqtt->writePayload(output, length);
}

bool writeJsonString(HAMqtt* mqtt, const char* value, const bool progmem)
{
    if (!mqtt || !value) {
        return false;
    }

    const char quote = '"';
    if (!mqtt->writePayload(&quote, 1)) {
        return false;
    }
    for (size_t i = 0; ; i++) {
        const uint8_t byte = progmem
            ? pgm_read_byte(value + i)
            : static_cast<uint8_t>(value[i]);
        if (byte == 0) {
            break;
        }
        if (!writeJsonEscapedByte(mqtt, byte)) {
            return false;
        }
    }
    return mqtt->writePayload(&quote, 1);
}

bool writeJsonStringContents(HAMqtt* mqtt, const char* value)
{
    if (!mqtt || !value) {
        return false;
    }

    for (size_t i = 0; value[i] != '\0'; i++) {
        if (!writeJsonEscapedByte(mqtt, static_cast<uint8_t>(value[i]))) {
            return false;
        }
    }

    return true;
}
} // namespace

uint16_t HASerializer::calculateConfigTopicLength(
    const __FlashStringHelper* componentName,
    const char* objectId
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !componentName ||
        !objectId ||
        !mqtt ||
        !mqtt->getDiscoveryPrefix() ||
        !mqtt->getDevice() ||
        !mqtt->getDevice()->getUniqueId() ||
        !HAJson::isValidDiscoveryTopicToken(mqtt->getDevice()->getUniqueId()) ||
        !HAJson::isValidDiscoveryTopicToken(objectId)
    ) {
        return 0;
    }

    return
        strlen(mqtt->getDiscoveryPrefix()) + 1 + // prefix with slash
        strlen_P(AHAFROMFSTR(componentName)) + 1 + // component name with slash
        strlen(mqtt->getDevice()->getUniqueId()) + 1 + // device ID with slash
        strlen(objectId) + 1 + // object ID with slash
        strlen_P(HAConfigTopic) + 1; // including null terminator
}

bool HASerializer::generateConfigTopic(
    char* output,
    const __FlashStringHelper* componentName,
    const char* objectId
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !output ||
        !componentName ||
        !objectId ||
        !mqtt ||
        !mqtt->getDiscoveryPrefix() ||
        !mqtt->getDevice() ||
        !mqtt->getDevice()->getUniqueId() ||
        !HAJson::isValidDiscoveryTopicToken(mqtt->getDevice()->getUniqueId()) ||
        !HAJson::isValidDiscoveryTopicToken(objectId)
    ) {
        return false;
    }

    strcpy(output, mqtt->getDiscoveryPrefix());
    strcat_P(output, HASerializerSlash);

    strcat_P(output, AHAFROMFSTR(componentName));
    strcat_P(output, HASerializerSlash);

    strcat(output, mqtt->getDevice()->getUniqueId());
    strcat_P(output, HASerializerSlash);

    strcat(output, objectId);
    strcat_P(output, HASerializerSlash);

    strcat_P(output, HAConfigTopic);
    return true;
}

uint16_t HASerializer::calculateDataTopicLength(
    const char* objectId,
    const __FlashStringHelper* topic
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !topic ||
        !mqtt ||
        !mqtt->getDataPrefix() ||
        !mqtt->getDevice() ||
        !mqtt->getDevice()->getUniqueId()
    ) {
        return 0;
    }

    uint16_t size =
        strlen(mqtt->getDataPrefix()) + 1 + // prefix with slash
        strlen(mqtt->getDevice()->getUniqueId()) + 1 + // device ID with slash
        strlen_P(AHAFROMFSTR(topic));

    if (objectId) {
        size += strlen(objectId) + 1; // object ID with slash;
    }

    return size + 1; // including null terminator
}

bool HASerializer::generateDataTopic(
    char* output,
    const char* objectId,
    const __FlashStringHelper* topic
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !output ||
        !topic ||
        !mqtt ||
        !mqtt->getDataPrefix() ||
        !mqtt->getDevice() ||
        !mqtt->getDevice()->getUniqueId()
    ) {
        return false;
    }

    strcpy(output, mqtt->getDataPrefix());
    strcat_P(output, HASerializerSlash);

    strcat(output, mqtt->getDevice()->getUniqueId());
    strcat_P(output, HASerializerSlash);

    if (objectId) {
        strcat(output, objectId);
        strcat_P(output, HASerializerSlash);
    }

    strcat_P(output, AHAFROMFSTR(topic));
    return true;
}

bool HASerializer::compareDataTopics(
    const char* actualTopic,
    const char* objectId,
    const __FlashStringHelper* topic
)
{
    if (!actualTopic) {
        return false;
    }

    const uint16_t topicLength = calculateDataTopicLength(objectId, topic);
    if (topicLength == 0) {
        return false;
    }

    char expectedTopic[topicLength];
    if (!generateDataTopic(expectedTopic, objectId, topic)) {
        return false;
    }

    return memcmp(actualTopic, expectedTopic, topicLength) == 0;
}

HASerializer::HASerializer(
    HABaseDeviceType* deviceType,
    const uint8_t maxEntriesNb
) :
    _deviceType(deviceType),
    _entriesNb(0),
    _maxEntriesNb(maxEntriesNb),
    _entries(new SerializerEntry[maxEntriesNb])
{

}

HASerializer::~HASerializer()
{
    delete[] _entries;
}

void HASerializer::set(
    const __FlashStringHelper* property,
    const void* value,
    PropertyValueType valueType
)
{
    if (!property || !value) {
        return;
    }

    SerializerEntry* entry = addEntry();
    if (!entry) {
        return;
    }

    entry->type = PropertyEntryType;
    entry->subtype = static_cast<uint8_t>(valueType);
    entry->property = property;
    entry->value = value;
}

void HASerializer::set(const FlagType flag)
{
    if (flag == WithDevice || flag == WithUniqueId) {
        SerializerEntry* entry = addEntry();
        if (!entry) {
            return;
        }

        entry->type = FlagEntryType;
        entry->subtype = static_cast<uint8_t>(flag);
        entry->property = nullptr;
        entry->value = nullptr;
    } else if (flag == WithAvailability) {
        if (_deviceType) {
            _deviceType->configureAvailabilityEntries(this);
        }
    }
}

void HASerializer::topic(const __FlashStringHelper* topic)
{
    if (!_deviceType || !topic) {
        return;
    }

    SerializerEntry* entry = addEntry();
    if (!entry) {
        return;
    }

    entry->type = TopicEntryType;
    entry->property = topic;
}

HASerializer::SerializerEntry* HASerializer::addEntry()
{
    if (_entriesNb >= _maxEntriesNb) {
        if (_maxEntriesNb == UINT8_MAX) {
            return nullptr;
        }

        uint16_t expandedCapacity = _maxEntriesNb == 0 ? 4 : _maxEntriesNb * 2;
        if (expandedCapacity > UINT8_MAX) {
            expandedCapacity = UINT8_MAX;
        }

        SerializerEntry* expandedEntries =
            new (std::nothrow) SerializerEntry[expandedCapacity];
        if (!expandedEntries) {
            return nullptr;
        }

        for (uint8_t i = 0; i < _entriesNb; i++) {
            expandedEntries[i] = _entries[i];
        }

        delete[] _entries;
        _entries = expandedEntries;
        _maxEntriesNb = static_cast<uint8_t>(expandedCapacity);
    }

    return &_entries[_entriesNb++];
}

uint16_t HASerializer::calculateSize() const
{
    uint32_t size =
        strlen_P(HASerializerJsonDataPrefix) +
        strlen_P(HASerializerJsonDataSuffix);

    for (uint8_t i = 0; i < _entriesNb; i++) {
        const uint16_t entrySize = calculateEntrySize(&_entries[i]);
        if (entrySize == 0) {
            return 0;
        }

        size += entrySize;

        // items separator
        if (i > 0) {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
        }

        if (size > UINT16_MAX) {
            return 0;
        }
    }

    return static_cast<uint16_t>(size);
}

bool HASerializer::flush() const
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || (_deviceType && !mqtt->getDevice())) {
        return false;
    }

    if (calculateSize() == 0) {
        return false;
    }

    if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonDataPrefix))) {
        return false;
    }

    for (uint8_t i = 0; i < _entriesNb; i++) {
        if (i > 0) {
            if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertiesSeparator))) {
                return false;
            }
        }

        if (!flushEntry(&_entries[i])) {
            return false;
        }
    }

    return mqtt->writePayload(AHATOFSTR(HASerializerJsonDataSuffix));
}

uint16_t HASerializer::calculateEntrySize(const SerializerEntry* entry) const
{
    switch (entry->type) {
    case PropertyEntryType: {
        if (!entry->property) {
            return 0;
        }
        const uint16_t valueSize = calculatePropertyValueSize(entry);
        if (valueSize == 0) {
            return 0;
        }
        const uint32_t size =
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen_P(AHAFROMFSTR(entry->property)) +
            strlen_P(HASerializerJsonPropertySuffix) +
            valueSize;
        return size > UINT16_MAX ? 0 : static_cast<uint16_t>(size);
    }

    case TopicEntryType:
        return calculateTopicEntrySize(entry);

    case AvailabilityArrayEntryType:
        return calculateAvailabilityArrayEntrySize(entry);

    case FlagEntryType:
        return calculateFlagSize(
            static_cast<FlagType>(entry->subtype)
        );

    default:
        return 0;
    }
}

uint16_t HASerializer::calculateTopicEntrySize(
    const SerializerEntry* entry
) const
{
    uint32_t size =
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(AHAFROMFSTR(entry->property)) +
        strlen_P(HASerializerJsonPropertySuffix);

    uint16_t topicSize = 0;
    if (entry->value) {
        topicSize = HAJson::calculateEscapedStringSize(
            static_cast<const char*>(entry->value)
        );
    } else {
        if (!_deviceType || !_deviceType->uniqueId()) {
            return 0;
        }

        const uint16_t length = calculateDataTopicLength(
            _deviceType->uniqueId(),
            entry->property
        );
        if (length == 0) {
            return 0;
        }

        char topic[length];
        if (!generateDataTopic(topic, _deviceType->uniqueId(), entry->property)) {
            return 0;
        }
        topicSize = HAJson::calculateEscapedStringSize(topic);
    }

    if (topicSize == 0 || (size + topicSize) > UINT16_MAX) {
        return 0;
    }

    return static_cast<uint16_t>(size + topicSize);
}

uint16_t HASerializer::calculateAvailabilityArrayEntrySize(
    const SerializerEntry* entry
) const
{
    if (!entry->value || !entry->property) {
        return 0;
    }

    const HAAvailabilityConfig* cfg = static_cast<const HAAvailabilityConfig*>(
        entry->value
    );
    const uint16_t jsonSize = cfg->calculateJsonSize();
    if (jsonSize == 0) {
        return 0;
    }

    const uint32_t size =
        strlen_P(HASerializerJsonPropertyPrefix) +
        strlen_P(AHAFROMFSTR(entry->property)) +
        strlen_P(HASerializerJsonPropertySuffix) +
        jsonSize;

    return size > UINT16_MAX ? 0 : static_cast<uint16_t>(size);
}

uint16_t HASerializer::calculateFlagSize(const FlagType flag) const
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || !mqtt->getDevice()) {
        return 0;
    }
    const HADevice* device = mqtt->getDevice();

    if (flag == WithDevice && device->getSerializer()) {
        const uint16_t deviceLength = device->getSerializer()->calculateSize();
        if (deviceLength == 0) {
            return 0;
        }

        const uint32_t size =
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen_P(HADeviceProperty) +
            strlen_P(HASerializerJsonPropertySuffix) +
            deviceLength;
        return size > UINT16_MAX ? 0 : static_cast<uint16_t>(size);
    } else if (flag == WithUniqueId && _deviceType && _deviceType->uniqueId()) {
        const uint16_t uniqueIdSize = HAJson::calculateEscapedStringSize(
            _deviceType->uniqueId()
        );
        if (uniqueIdSize == 0) {
            return 0;
        }

        uint32_t valueSize = uniqueIdSize;

        if (device->isExtendedUniqueIdsEnabled()) {
            if (!device->getUniqueId()) {
                return 0;
            }

            const uint16_t deviceIdSize = HAJson::calculateEscapedStringSize(
                device->getUniqueId()
            );
            if (deviceIdSize == 0) {
                return 0;
            }

            // Both helper sizes include quotes; the combined value has one pair.
            valueSize = deviceIdSize + uniqueIdSize - 1;
        }

        const uint32_t size =
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen_P(HAUniqueIdProperty) +
            strlen_P(HASerializerJsonPropertySuffix) +
            valueSize;
        return size > UINT16_MAX ? 0 : static_cast<uint16_t>(size);
    }

    return 0;
}

uint16_t HASerializer::calculatePropertyValueSize(
    const SerializerEntry* entry
) const
{
    switch (entry->subtype) {
    case ConstCharPropertyValue:
    case ProgmemPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        return entry->subtype == ConstCharPropertyValue
            ? HAJson::calculateEscapedStringSize(value)
            : HAJson::calculateEscapedProgmemStringSize(value);
    }

    case BoolPropertyType: {
        const bool value = *static_cast<const bool*>(entry->value);
        return value ? strlen_P(HATrue) : strlen_P(HAFalse);
    }

    case NumberPropertyType: {
        const HANumeric* value = static_cast<const HANumeric*>(
            entry->value
        );
        return value->calculateSize();
    }

    case ArrayPropertyType: {
        const HASerializerArray* array = static_cast<const HASerializerArray*>(
            entry->value
        );
        return array ? array->calculateSize() : 0;
    }

    case JsonLiteralPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        return value ? strlen(value) : 0;
    }

    default:
        return 0;
    }
}

bool HASerializer::flushEntry(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    switch (entry->type) {
    case PropertyEntryType: {
        if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) ||
            !mqtt->writePayload(entry->property) ||
            !mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertySuffix))) {
            return false;
        }

        return flushEntryValue(entry);
    }

    case TopicEntryType:
        return flushTopic(entry);

    case AvailabilityArrayEntryType:
        return flushAvailabilityArray(entry);

    case FlagEntryType:
        return flushFlag(entry);

    default:
        return true;
    }
}

bool HASerializer::flushEntryValue(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    switch (entry->subtype) {
    case ConstCharPropertyValue:
    case ProgmemPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        return writeJsonString(mqtt, value, entry->subtype == ProgmemPropertyValue);
    }

    case BoolPropertyType: {
        const bool value = *static_cast<const bool*>(entry->value);
        return mqtt->writePayload(AHATOFSTR(value ? HATrue : HAFalse));
    }

    case NumberPropertyType: {
        const HANumeric* value = static_cast<const HANumeric*>(
            entry->value
        );

        char tmp[HANumeric::MaxDigitsNb + 1];
        const uint16_t length = value->toStr(tmp);

        return mqtt->writePayload(tmp, length);
    }

    case ArrayPropertyType: {
        const HASerializerArray* array = static_cast<const HASerializerArray*>(
            entry->value
        );
        if (!array) {
            return false;
        }

        const uint16_t size = array->calculateSize();
        if (size == 0) {
            return false;
        }

        char* tmp = new (std::nothrow) char[size + 1];
        if (!tmp) {
            return false;
        }
        tmp[0] = 0;
        bool serialized = array->serialize(tmp);
        if (serialized) {
            serialized = mqtt->writePayload(tmp, size);
        }
        delete[] tmp;

        return serialized;
    }

    case JsonLiteralPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        if (!value) {
            return false;
        }

        return mqtt->writePayload(value, strlen(value));
    }

    default:
        return false;
    }
}

bool HASerializer::flushTopic(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    // property name
    if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) ||
        !mqtt->writePayload(entry->property) ||
        !mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertySuffix))) {
        return false;
    }

    if (entry->value) {
        const char* topic = static_cast<const char*>(entry->value);
        return writeJsonString(mqtt, topic, false);
    } else {
        if (!_deviceType || !_deviceType->uniqueId()) {
            return false;
        }

        const uint16_t length = calculateDataTopicLength(
            _deviceType->uniqueId(),
            entry->property
        );
        if (length == 0) {
            return false;
        }

        char topic[length];
        if (!generateDataTopic(
            topic,
            _deviceType->uniqueId(),
            entry->property
        )) {
            return false;
        }

        return writeJsonString(mqtt, topic, false);
    }
}

bool HASerializer::flushAvailabilityArray(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || !entry->value || !entry->property) {
        return false;
    }

    const HAAvailabilityConfig* cfg = static_cast<const HAAvailabilityConfig*>(
        entry->value
    );
    const uint16_t jsonSize = cfg->calculateJsonSize();
    if (jsonSize == 0) {
        return false;
    }

    char* buf = new (std::nothrow) char[jsonSize + 1];
    if (!buf) {
        return false;
    }

    const bool serialized = cfg->serialize(buf);
    if (!serialized) {
        delete[] buf;
        return false;
    }

    const bool written = mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) &&
        mqtt->writePayload(entry->property) &&
        mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertySuffix)) &&
        mqtt->writePayload(buf, jsonSize);
    delete[] buf;
    return written;
}

bool HASerializer::flushFlag(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || !mqtt->getDevice()) {
        return false;
    }
    const HADevice* device = mqtt->getDevice();
    const FlagType flag = static_cast<FlagType>(entry->subtype);

    if (flag == WithDevice && device->getSerializer()) {
        // property name
        if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) ||
            !mqtt->writePayload(AHATOFSTR(HADeviceProperty)) ||
            !mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertySuffix))) {
            return false;
        }

        // property value
        return device->getSerializer()->flush();
    } else if (flag == WithUniqueId && _deviceType && _deviceType->uniqueId()) {
        if (device->isExtendedUniqueIdsEnabled() && !device->getUniqueId()) {
            return false;
        }

        // property name
        if (!mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertyPrefix)) ||
            !mqtt->writePayload(AHATOFSTR(HAUniqueIdProperty)) ||
            !mqtt->writePayload(AHATOFSTR(HASerializerJsonPropertySuffix))) {
            return false;
        }

        // value
        const char* uniqueId = _deviceType->uniqueId();
        const char quote = '"';
        if (!mqtt->writePayload(&quote, 1)) {
            return false;
        }

        if (device->isExtendedUniqueIdsEnabled()) {
            const char* deviceUniqueId = device->getUniqueId();
            if (!writeJsonStringContents(mqtt, deviceUniqueId)) {
                return false;
            }
            const char separator = '_';
            if (!mqtt->writePayload(&separator, 1)) {
                return false;
            }
        }

        return writeJsonStringContents(mqtt, uniqueId) &&
            mqtt->writePayload(&quote, 1);
    }

    return false;
}