#ifndef AHA_JSON_H
#define AHA_JSON_H

#include <stdint.h>

/**
 * Small JSON helpers used by discovery serializers.
 *
 * The library streams discovery documents directly to PubSubClient, so the
 * calculated size must exactly match the escaped JSON representation before a
 * retained payload is started.
 */
namespace HAJson
{
    /**
     * Returns the number of bytes needed to encode value as a JSON string,
     * including its surrounding quotes. Returns zero for a null value or when
     * the result cannot fit in a uint16_t.
     */
    uint16_t calculateEscapedStringSize(const char* value);

    /**
     * As calculateEscapedStringSize(), but reads value from program memory.
     */
    uint16_t calculateEscapedProgmemStringSize(const char* value);

    /**
     * Appends a program-memory value as a JSON string to [cursor, end).
     */
    bool appendEscapedProgmemString(char*& cursor, char* end, const char* value);

    /**
     * Appends value as a JSON string to [cursor, end). end points at the last
     * usable byte for the terminating null character. The output is left
     * unchanged when the complete escaped value cannot fit.
     */
    bool appendEscapedString(char*& cursor, char* end, const char* value);

    /**
     * Home Assistant discovery node/object IDs may only contain these topic
     * token characters: [A-Za-z0-9_-].
     */
    bool isValidDiscoveryTopicToken(const char* value);
}

#endif
