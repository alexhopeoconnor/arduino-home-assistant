#include "ArduinoHADefines.h"
#include "HADevice.h"
#include "HAMqtt.h"
#include "utils/HAUtils.h"
#include "utils/HADictionary.h"
#include "utils/HAJson.h"
#include "utils/HASerializer.h"
#include <string.h>

static bool appendEscapedJsonString(char*& cursor, char* end, const char* value)
{
    return HAJson::appendEscapedString(cursor, end, value);
}

namespace {
void skipJsonWhitespace(const char*& cursor)
{
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }
}

bool isHexDigit(const char value)
{
    return (value >= '0' && value <= '9') ||
        (value >= 'a' && value <= 'f') ||
        (value >= 'A' && value <= 'F');
}

bool parseJsonString(const char*& cursor)
{
    if (*cursor != '"') {
        return false;
    }

    cursor++;
    while (*cursor != '\0') {
        const unsigned char value = static_cast<unsigned char>(*cursor++);
        if (value == '"') {
            return true;
        }

        if (value < 0x20) {
            return false;
        }

        if (value != '\\') {
            continue;
        }

        const char escape = *cursor++;
        if (escape == '\0') {
            return false;
        }

        if (escape == '"' || escape == '\\' || escape == '/' ||
            escape == 'b' || escape == 'f' || escape == 'n' ||
            escape == 'r' || escape == 't') {
            continue;
        }

        if (escape != 'u') {
            return false;
        }

        for (uint8_t i = 0; i < 4; i++) {
            if (!isHexDigit(*cursor++)) {
                return false;
            }
        }
    }

    return false;
}

bool isValidConnectionsJson(const char* value, const size_t maxLength)
{
    if (!value || value[0] == '\0' || strlen(value) >= maxLength) {
        return false;
    }

    const char* cursor = value;
    skipJsonWhitespace(cursor);
    if (*cursor++ != '[') {
        return false;
    }

    skipJsonWhitespace(cursor);
    if (*cursor == ']') {
        cursor++;
        skipJsonWhitespace(cursor);
        return *cursor == '\0';
    }

    while (true) {
        if (*cursor++ != '[') {
            return false;
        }

        skipJsonWhitespace(cursor);
        if (!parseJsonString(cursor)) {
            return false;
        }

        skipJsonWhitespace(cursor);
        if (*cursor++ != ',') {
            return false;
        }

        skipJsonWhitespace(cursor);
        if (!parseJsonString(cursor)) {
            return false;
        }

        skipJsonWhitespace(cursor);
        if (*cursor++ != ']') {
            return false;
        }

        skipJsonWhitespace(cursor);
        if (*cursor == ']') {
            cursor++;
            skipJsonWhitespace(cursor);
            return *cursor == '\0';
        }

        if (*cursor++ != ',') {
            return false;
        }

        skipJsonWhitespace(cursor);
    }
}
} // namespace

#define HADEVICE_INIT \
    _ownsUniqueId(false), \
    _serializer(new HASerializer(nullptr, 16)), \
    _availabilityTopic(nullptr), \
    _sharedAvailability(false), \
    _available(true), \
    _extendedUniqueIds(false), \
    _payloadAvailable(nullptr), \
    _payloadNotAvailable(nullptr), \
    _connectionsJson(), \
    _hasConnections(false), \
    _connectionsPropertyRegistered(false)

HADevice::HADevice() :
    _uniqueId(nullptr),
    HADEVICE_INIT
{

}

HADevice::HADevice(const char* uniqueId) :
    _uniqueId(uniqueId),
    HADEVICE_INIT
{
    _serializer->set(AHATOFSTR(HADeviceIdentifiersProperty), _uniqueId);
}

HADevice::HADevice(const byte* uniqueId, const uint16_t length) :
    _uniqueId(HAUtils::byteArrayToStr(uniqueId, length)),
    HADEVICE_INIT
{
    _ownsUniqueId = true;
    _serializer->set(AHATOFSTR(HADeviceIdentifiersProperty), _uniqueId);
}

HADevice::~HADevice()
{
    delete _serializer;

    if (_availabilityTopic) {
        delete[] _availabilityTopic;
    }

    if (_ownsUniqueId) {
        delete[] _uniqueId;
    }
}

bool HADevice::setUniqueId(const byte* uniqueId, const uint16_t length)
{
    if (_uniqueId) {
        return false; // unique ID cannot be changed at runtime once it's set
    }

    _uniqueId = HAUtils::byteArrayToStr(uniqueId, length);
    _ownsUniqueId = true;
    _serializer->set(AHATOFSTR(HADeviceIdentifiersProperty), _uniqueId);
    return true;
}

void HADevice::setManufacturer(const char* manufacturer)
{
    _serializer->set(AHATOFSTR(HADeviceManufacturerProperty), manufacturer);
}

void HADevice::setModel(const char* model)
{
    _serializer->set(AHATOFSTR(HADeviceModelProperty), model);
}

void HADevice::setName(const char* name)
{
    _serializer->set(AHATOFSTR(HANameProperty), name);
}

void HADevice::setSoftwareVersion(const char* softwareVersion)
{
    _serializer->set(
        AHATOFSTR(HADeviceSoftwareVersionProperty),
        softwareVersion
    );
}

void HADevice::setConfigurationUrl(const char* url)
{
    _serializer->set(
        AHATOFSTR(HADeviceConfigurationUrlProperty),
        url
    );
}

void HADevice::setModelId(const char* modelId)
{
    if (!modelId) {
        return;
    }

    _serializer->set(AHATOFSTR(HADeviceModelIdProperty), modelId);
}

void HADevice::setHardwareVersion(const char* hardwareVersion)
{
    if (!hardwareVersion) {
        return;
    }

    _serializer->set(AHATOFSTR(HADeviceHwVersionProperty), hardwareVersion);
}

void HADevice::setSerialNumber(const char* serialNumber)
{
    if (!serialNumber) {
        return;
    }

    _serializer->set(AHATOFSTR(HADeviceSerialNumberProperty), serialNumber);
}

void HADevice::setSuggestedArea(const char* suggestedArea)
{
    if (!suggestedArea) {
        return;
    }

    _serializer->set(AHATOFSTR(HADeviceSuggestedAreaProperty), suggestedArea);
}

void HADevice::setViaDevice(const char* viaDevice)
{
    if (!viaDevice) {
        return;
    }

    _serializer->set(AHATOFSTR(HADeviceViaDeviceProperty), viaDevice);
}

bool HADevice::addConnection(const char* type, const char* value)
{
    if (!type || !value || type[0] == '\0' || value[0] == '\0') {
        return false;
    }

    char next[MaxConnectionsJsonLength];
    if (_hasConnections) {
        strncpy(next, _connectionsJson, MaxConnectionsJsonLength - 1);
        next[MaxConnectionsJsonLength - 1] = 0;
    } else {
        strcpy(next, "[]");
    }

    const size_t len = strlen(next);
    if (len < 2 || next[len - 1] != ']') {
        return false;
    }

    char* cursor = next + len - 1;
    char* end = next + MaxConnectionsJsonLength - 1;

    if (cursor > next + 1) {
        if (cursor + 1 >= end) {
            return false;
        }
        *cursor++ = ',';
    }

    if (cursor + 1 >= end) {
        return false;
    }
    *cursor++ = '[';
    *cursor = 0;

    if (!appendEscapedJsonString(cursor, end, type)) {
        return false;
    }

    if (cursor + 1 >= end) {
        return false;
    }
    *cursor++ = ',';
    *cursor = 0;

    if (!appendEscapedJsonString(cursor, end, value)) {
        return false;
    }

    if (cursor + 2 >= end) {
        return false;
    }
    *cursor++ = ']';
    *cursor++ = ']';
    *cursor = 0;

    strncpy(_connectionsJson, next, MaxConnectionsJsonLength - 1);
    _connectionsJson[MaxConnectionsJsonLength - 1] = 0;
    _hasConnections = true;

    if (!_connectionsPropertyRegistered) {
        _serializer->set(
            AHATOFSTR(HADeviceConnectionsProperty),
            _connectionsJson,
            HASerializer::JsonLiteralPropertyValue
        );
        _connectionsPropertyRegistered = true;
    }

    return true;
}

bool HADevice::setConnectionsJson(const char* connectionsJson)
{
    if (!isValidConnectionsJson(connectionsJson, MaxConnectionsJsonLength)) {
        return false;
    }

    strcpy(_connectionsJson, connectionsJson);
    _hasConnections = true;

    if (!_connectionsPropertyRegistered) {
        _serializer->set(
            AHATOFSTR(HADeviceConnectionsProperty),
            _connectionsJson,
            HASerializer::JsonLiteralPropertyValue
        );
        _connectionsPropertyRegistered = true;
    }

    return true;
}

void HADevice::setPayloadAvailable(const char* payload)
{
    _payloadAvailable = payload;
}

void HADevice::setPayloadNotAvailable(const char* payload)
{
    _payloadNotAvailable = payload;
}

void HADevice::setAvailability(bool online)
{
    _available = online;
    publishAvailability();
}

bool HADevice::enableSharedAvailability()
{
    if (_sharedAvailability) {
        return true; // already enabled
    }

    const uint16_t topicLength = HASerializer::calculateDataTopicLength(
        nullptr,
        AHATOFSTR(HAAvailabilityTopic)
    );
    if (topicLength == 0) {
        return false;
    }

    _availabilityTopic = new char[topicLength];

    if (HASerializer::generateDataTopic(
        _availabilityTopic,
        nullptr,
        AHATOFSTR(HAAvailabilityTopic)
    ) > 0) {
        _sharedAvailability = true;
        return true;
    }

    return false;
}

void HADevice::enableLastWill()
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || !_availabilityTopic) {
        return;
    }

    const char* lw = (_payloadNotAvailable && _payloadNotAvailable[0] != '\0')
        ? _payloadNotAvailable
        : "offline";

    mqtt->setLastWill(
        _availabilityTopic,
        lw,
        true
    );
}

void HADevice::publishAvailability() const
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!_availabilityTopic || !mqtt) {
        return;
    }

    if (_available) {
        if (_payloadAvailable && _payloadAvailable[0] != '\0') {
            const uint16_t len = strlen(_payloadAvailable);
            if (mqtt->beginPublish(_availabilityTopic, len, true)) {
                mqtt->writePayload(_payloadAvailable, len);
                mqtt->endPublish();
            }
        } else {
            const uint16_t len = strlen_P(HAOnline);
            if (mqtt->beginPublish(_availabilityTopic, len, true)) {
                mqtt->writePayload(AHATOFSTR(HAOnline));
                mqtt->endPublish();
            }
        }
    } else {
        if (_payloadNotAvailable && _payloadNotAvailable[0] != '\0') {
            const uint16_t len = strlen(_payloadNotAvailable);
            if (mqtt->beginPublish(_availabilityTopic, len, true)) {
                mqtt->writePayload(_payloadNotAvailable, len);
                mqtt->endPublish();
            }
        } else {
            const uint16_t len = strlen_P(HAOffline);
            if (mqtt->beginPublish(_availabilityTopic, len, true)) {
                mqtt->writePayload(AHATOFSTR(HAOffline));
                mqtt->endPublish();
            }
        }
    }
}
