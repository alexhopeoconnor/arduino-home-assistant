#include "HAJson.h"

#include <Arduino.h>
#include <limits.h>
#include <string.h>

namespace {
uint8_t escapedByteSize(const uint8_t value)
{
    switch (value) {
    case '"':
    case '\\':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
        return 2;
    default:
        return value < 0x20 ? 6 : 1;
    }
}

char hexDigit(const uint8_t value)
{
    return value < 10 ? static_cast<char>('0' + value) : static_cast<char>('A' + (value - 10));
}

void appendEscapedByte(char*& cursor, const uint8_t value)
{
    switch (value) {
    case '"':
        *cursor++ = '\\';
        *cursor++ = '"';
        return;
    case '\\':
        *cursor++ = '\\';
        *cursor++ = '\\';
        return;
    case '\b':
        *cursor++ = '\\';
        *cursor++ = 'b';
        return;
    case '\f':
        *cursor++ = '\\';
        *cursor++ = 'f';
        return;
    case '\n':
        *cursor++ = '\\';
        *cursor++ = 'n';
        return;
    case '\r':
        *cursor++ = '\\';
        *cursor++ = 'r';
        return;
    case '\t':
        *cursor++ = '\\';
        *cursor++ = 't';
        return;
    default:
        if (value < 0x20) {
            *cursor++ = '\\';
            *cursor++ = 'u';
            *cursor++ = '0';
            *cursor++ = '0';
            *cursor++ = hexDigit(static_cast<uint8_t>(value >> 4));
            *cursor++ = hexDigit(static_cast<uint8_t>(value & 0x0F));
        } else {
            *cursor++ = static_cast<char>(value);
        }
    }
}
} // namespace

uint16_t HAJson::calculateEscapedStringSize(const char* value)
{
    if (!value) {
        return 0;
    }

    uint32_t size = 2; // surrounding quotes
    for (const uint8_t* p = reinterpret_cast<const uint8_t*>(value); *p != 0; p++) {
        size += escapedByteSize(*p);
        if (size > UINT16_MAX) {
            return 0;
        }
    }

    return static_cast<uint16_t>(size);
}

uint16_t HAJson::calculateEscapedProgmemStringSize(const char* value)
{
    if (!value) {
        return 0;
    }

    uint32_t size = 2; // surrounding quotes
    for (uint16_t i = 0; ; i++) {
        const uint8_t byte = pgm_read_byte(value + i);
        if (byte == 0) {
            break;
        }

        size += escapedByteSize(byte);
        if (size > UINT16_MAX) {
            return 0;
        }
    }

    return static_cast<uint16_t>(size);
}

bool HAJson::appendEscapedString(char*& cursor, char* end, const char* value)
{
    if (!cursor || !end || !value || cursor > end) {
        return false;
    }

    const uint16_t size = calculateEscapedStringSize(value);
    if (size == 0 || static_cast<size_t>(end - cursor) < size) {
        return false;
    }

    *cursor++ = '"';
    for (const uint8_t* p = reinterpret_cast<const uint8_t*>(value); *p != 0; p++) {
        appendEscapedByte(cursor, *p);
    }
    *cursor++ = '"';
    *cursor = 0;
    return true;
}

bool HAJson::appendEscapedProgmemString(char*& cursor, char* end, const char* value)
{
    if (!cursor || !end || !value || cursor > end) {
        return false;
    }

    const uint16_t size = calculateEscapedProgmemStringSize(value);
    if (size == 0 || static_cast<size_t>(end - cursor) < size) {
        return false;
    }

    *cursor++ = '"';
    for (uint16_t i = 0; ; i++) {
        const uint8_t byte = pgm_read_byte(value + i);
        if (byte == 0) {
            break;
        }
        appendEscapedByte(cursor, byte);
    }
    *cursor++ = '"';
    *cursor = 0;
    return true;
}

bool HAJson::isValidDiscoveryTopicToken(const char* value)
{
    if (!value || value[0] == '\0') {
        return false;
    }

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(value); *p != 0; p++) {
        const bool isLower = *p >= 'a' && *p <= 'z';
        const bool isUpper = *p >= 'A' && *p <= 'Z';
        const bool isDigit = *p >= '0' && *p <= '9';
        if (!isLower && !isUpper && !isDigit && *p != '_' && *p != '-') {
            return false;
        }
    }

    return true;
}
