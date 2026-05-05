#include <Arduino.h>
#include <string.h>
#include "HAAvailabilityConfig.h"
#include "HADictionary.h"

static uint16_t jsonEscapedStringSize(const char* s)
{
    if (!s) {
        return 0;
    }
    return 2 * strlen_P(HASerializerJsonEscapeChar) + strlen(s);
}

static void appendEscapedString(char* buf, const char* s)
{
    strcat_P(buf, HASerializerJsonEscapeChar);
    strcat(buf, s);
    strcat_P(buf, HASerializerJsonEscapeChar);
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
    uint16_t size =
        strlen_P(HASerializerJsonArrayPrefix) +
        strlen_P(HASerializerJsonArraySuffix);

    if (_count == 0) {
        return size;
    }

    size += (_count - 1) * strlen_P(HASerializerJsonPropertiesSeparator);

    for (uint8_t i = 0; i < _count; i++) {
        const Entry& e = _entries[i];
        // {"t":"topic" ... }
        size += strlen_P(HASerializerJsonDataPrefix);
        size += strlen_P(HASerializerJsonDataSuffix);

        // "t":"..."
        size +=
            strlen_P(HASerializerJsonPropertyPrefix) +
            strlen_P(HATopic) +
            strlen_P(HASerializerJsonPropertySuffix) +
            jsonEscapedStringSize(e.topic);

        if (e.valueTemplate && e.valueTemplate[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAValueTemplateProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                jsonEscapedStringSize(e.valueTemplate);
        }
        if (e.payloadAvailable && e.payloadAvailable[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAPayloadAvailableProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                jsonEscapedStringSize(e.payloadAvailable);
        }
        if (e.payloadNotAvailable && e.payloadNotAvailable[0] != '\0') {
            size += strlen_P(HASerializerJsonPropertiesSeparator);
            size +=
                strlen_P(HASerializerJsonPropertyPrefix) +
                strlen_P(HAPayloadNotAvailableProperty) +
                strlen_P(HASerializerJsonPropertySuffix) +
                jsonEscapedStringSize(e.payloadNotAvailable);
        }
    }

    return size;
}

bool HAAvailabilityConfig::serialize(char* output) const
{
    if (!output) {
        return false;
    }

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
        appendEscapedString(output, _entries[i].topic);

        const Entry& e = _entries[i];
        if (e.valueTemplate && e.valueTemplate[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAValueTemplateProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            appendEscapedString(output, e.valueTemplate);
        }
        if (e.payloadAvailable && e.payloadAvailable[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAPayloadAvailableProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            appendEscapedString(output, e.payloadAvailable);
        }
        if (e.payloadNotAvailable && e.payloadNotAvailable[0] != '\0') {
            strcat_P(output, HASerializerJsonPropertiesSeparator);
            strcat_P(output, HASerializerJsonPropertyPrefix);
            strcat_P(output, HAPayloadNotAvailableProperty);
            strcat_P(output, HASerializerJsonPropertySuffix);
            appendEscapedString(output, e.payloadNotAvailable);
        }

        strcat_P(output, HASerializerJsonDataSuffix);
    }

    strcat_P(output, HASerializerJsonArraySuffix);
    return true;
}
