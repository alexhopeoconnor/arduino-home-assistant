#ifndef AHA_HAAVAILABILITYCONFIG_H
#define AHA_HAAVAILABILITYCONFIG_H

#include <stdint.h>

/**
 * Holds up to four MQTT availability entries for discovery JSON (`avty`).
 * Topic strings must remain valid for the lifetime of the config (same pattern as other ArduinoHA setters).
 */
class HAAvailabilityConfig
{
public:
    static const uint8_t MaxEntries = 4;

    struct Entry {
        const char* topic;
        const char* valueTemplate;
        const char* payloadAvailable;
        const char* payloadNotAvailable;
    };

    HAAvailabilityConfig();
    ~HAAvailabilityConfig();

    /**
     * Adds an availability entry. `topic` must be a full MQTT topic string.
     * @return false when full or topic is null/empty.
     */
    bool add(
        const char* topic,
        const char* valueTemplate = nullptr,
        const char* payloadAvailable = nullptr,
        const char* payloadNotAvailable = nullptr
    );

    void clear();

    inline uint8_t count() const
        { return _count; }

    inline const Entry& getEntry(const uint8_t index) const
        { return _entries[index]; }

    /**
     * Size of the JSON array only, including `[` and `]`, excluding property name and separators.
     */
    uint16_t calculateJsonSize() const;

    /**
     * Writes the JSON array into `output` (must be at least calculateJsonSize()+1 bytes).
     */
    bool serialize(char* output) const;

private:
    void clearEntry(Entry& entry);
    char* duplicateString(const char* value) const;

    Entry _entries[MaxEntries];
    uint8_t _count;
};

#endif
