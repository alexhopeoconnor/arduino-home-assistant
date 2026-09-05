#include <Arduino.h>
#include <string.h>
#include "HAAvailabilityConfig.h"
#include "HADictionary.h"
#include "HAJson.h"

static uint16_t jsonEscapedStringSize(const char* s)
{
    return HAJson::calculateEscapedStringSize(s);
}

static bool appendEscapedString(char* buf, char* end, const char* s)
{
    char* cursor = buf + strlen(buf);
    return HAJson::appendEscapedString(cursor, end, s);
}

HAAvailabilityConfig::HAAvailabilityConfig() :
    _count(0)
{
    memset(_entries, 0, sizeof(_entries));
}

HAAvailabilityConfig::~HAAvailabilityConfig()
{
    clear();
}

char* HAAvailabilityConfig::duplicateString(const char* value) const
{
    if (!value || value[0] == '\0') {
        return nullptr;
    }

    const size_t len = strlen(value);
    char* copy = new char[len + 1];
    memcpy(copy, value, len + 1);
    return copy;
}

void HAAvailabilityConfig::clearEntry(Entry& entry)
{
    delete[] entry.topic;
    delete[] entry.valueTemplate;
    delete[] entry.payloadAvailable;
    delete[] entry.payloadNotAvailable;
    entry.topic = nullptr;
    entry.valueTemplate = nullptr;
    entry.payloadAvailable = nullptr;
    entry.payloadNotAvailable = nullptr;
}

bool HAAvailabilityConfig::add(
    const char* topic,
    const char* valueTemplate,
    const char* payloadAvailable,
    const char* payloadNotAvailable
)
{
    if (!topic || topic[0] == '\0' || _count >= MaxEntries) {
        return false;
    }

    Entry entry = {};
    entry.topic = duplicateString(topic);
    if (!entry.topic) {
        return false;
    }

    entry.valueTemplate = duplicateString(valueTemplate);
    entry.payloadAvailable = duplicateString(payloadAvailable);
    entry.payloadNotAvailable = duplicateString(payloadNotAvailable);

    _entries[_count] = entry;
    _count++;
    return true;
}

void HAAvailabilityConfig::clear()
{
    for (uint8_t i = 0; i < _count; i++) {
        clearEntry(_entries[i]);
    }

    _count = 0;
}

uint16_t HAAvailabilityConfig::calculateJsonSize() const
{
    uint32_t size =
        strlen_P(HASerializerJsonArrayPrefix) +
        strlen_P(HASerializerJsonArraySuffix);

    if (_count == 0) {
        return static_cast<uint16_t>(size);
    }

    size += (_count - 1) * strlen_P(HASerializerJsonPropertiesSeparator);

    for (uint8_t i = 0; i < _count; i++) {
        const Entry& e = _entries[i];
        // {"t":"topic" ... }
        size += strlen_P(HASerializerJsonDataPrefix);
        size += strlen_P(HASerializerJsonDataSuffix);

        // "t":"..."
        const uint16_t topicSize = jsonEscapedStringSize(e.topic);
        if (topicSize == 0) {
            return 0;
        }
        size +=
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen_P(HATopic) +
            strlen_P(HASerializerJsonPropertySuffix) +
            topicSize;

        if (e.valueTemplate && e.valueTemplate[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            const uint16_t valueTemplateSize = jsonEscapedStringSize(e.valueTemplate);
            if (valueTemplateSize == 0) {
                return 0;
            }
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAValueTemplateProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                valueTemplateSize;
        }
        if (e.payloadAvailable && e.payloadAvailable[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            const uint16_t payloadAvailableSize = jsonEscapedStringSize(e.payloadAvailable);
            if (payloadAvailableSize == 0) {
                return 0;
            }
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAPayloadAvailableProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                payloadAvailableSize;
        }
        if (e.payloadNotAvailable && e.payloadNotAvailable[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            const uint16_t payloadNotAvailableSize = jsonEscapedStringSize(e.payloadNotAvailable);
            if (payloadNotAvailableSize == 0) {
                return 0;
            }
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAPayloadNotAvailableProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                payloadNotAvailableSize;
        }
    }

    if (size > UINT16_MAX) {
        return 0;
    }

    return static_cast<uint16_t>(size);
}

bool HAAvailabilityConfig::serialize(char* output) const
{
    if (!output) {
        return false;
    }

    const uint16_t jsonSize = calculateJsonSize();
    if (jsonSize == 0) {
        return false;
    }

    char* const end = output + jsonSize;
    output[0] = 0;
    strcat_P(output, HASerializerJsonArrayPrefix);

    for (uint8_t i = 0; i < _count; i++) {
        if (i > 0) {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
        }

        strcat_P(output, HASerializerJsonDataPrefix);

        strcat_P(output, HASerializerJsonPropertyPrefix);
        strcat_P(output, HATopic);
        strcat_P(output, HASerializerJsonPropertySuffix);
        if (!appendEscapedString(output, end, _entries[i].topic)) {
            return false;
        }

        const Entry& e = _entries[i];
        if (e.valueTemplate && e.valueTemplate[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAValueTemplateProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            if (!appendEscapedString(output, end, e.valueTemplate)) {
                return false;
            }
        }
        if (e.payloadAvailable && e.payloadAvailable[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAPayloadAvailableProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            if (!appendEscapedString(output, end, e.payloadAvailable)) {
                return false;
            }
        }
        if (e.payloadNotAvailable && e.payloadNotAvailable[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAPayloadNotAvailableProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            if (!appendEscapedString(output, end, e.payloadNotAvailable)) {
                return false;
            }
        }

        strcat_P(output, HASerializerJsonDataSuffix);
    }

    strcat_P(output, HASerializerJsonArraySuffix);
    return true;
}
