#ifndef AHA_HABASEDEVICETYPE_H
#define AHA_HABASEDEVICETYPE_H

#include <Arduino.h>
#include "../ArduinoHADefines.h"
#include "../utils/HAAvailabilityConfig.h"
#include "../utils/HANumeric.h"

class HAMqtt;
class HASerializer;

class HABaseDeviceType
{
public:
    enum NumberPrecision {
        /// No digits after the decimal point.
        PrecisionP0 = 0,

        /// One digit after the decimal point.
        PrecisionP1,

        /// Two digits after the decimal point.
        PrecisionP2,

        /// Three digits after the decimal point.
        PrecisionP3
    };

    /**
     * Creates a new device type instance and registers it in the HAMqtt class.
     *
     * @param componentName The name of the Home Assistant component (e.g. `binary_sensor`).
     *                      You can find all available component names in the Home Assistant documentation.
     *                      The component name needs to be stored in the flash memory.
     * @param uniqueId The unique ID of the device type. It needs to be unique in a scope of the HADevice.
     */
    HABaseDeviceType(
        const __FlashStringHelper* componentName,
        const char* uniqueId
    );

    virtual ~HABaseDeviceType();

    /**
     * Returns unique ID of the device type.
     */
    inline const char* uniqueId() const
        { return _uniqueId; }

    /**
     * Returns component name defined by the device type.
     * It's used for the MQTT discovery topic.
     */
    inline const __FlashStringHelper* componentName() const
        { return _componentName; }

    /**
     * Returns `true` if the availability was configured for this device type.
     */
    inline bool isAvailabilityConfigured() const
        { return (_availability != AvailabilityDefault); }

    /**
     * Returns online state of the device type.
     */
    inline bool isOnline() const
        { return (_availability == AvailabilityOnline); }

    /**
     * Sets name of the device type that will be used as a label in the HA panel.
     * Keep the name short to save the resources.
     *
     * @param name The device type name.
     */
    inline void setName(const char* name)
        { _name = name; }

    /**
     * Returns name of the deviced type that was assigned via setName method.
     * It can be nullptr if there is no name assigned.
     */
    inline const char* getName() const
        { return _name; }

    /**
     * Sets the default entity ID that Home Assistant should use when creating
     * the entity for the first time.
     * Keep the ID short to save the resources.
     *
     * @param entityId The default entity ID.
     */
    inline void setDefaultEntityId(const char* entityId)
        { _defaultEntityId = entityId; }

    /**
     * Returns the default entity ID that was assigned via setDefaultEntityId.
     * It can be nullptr if there is no ID assigned.
     */
    inline const char* getDefaultEntityId() const
        { return _defaultEntityId; }

    /**
     * Legacy alias for the MQTT `object_id` discovery property.
     * Prefer setDefaultEntityId() for new code.
     *
     * @param objectId The object ID.
     */
    inline void setObjectId(const char* objectId)
        { _objectId = objectId; }

    /**
     * Returns the object ID that was set by setObjectId method.
     * It can be nullptr if there is no ID assigned.
     */
    inline const char* getObjectId() const
        { return _objectId; }

    /**
     * Sets the entity category for this entity.
     *
     * @param entityCategory The category name.
     */
    inline void setEntityCategory(const char* entityCategory)
        { _entityCategory = entityCategory; }

    /**
     * Returns the entity category for this entity.
     */
    inline const char* getEntityCategory() const
        { return _entityCategory; }

    void setEnabledByDefault(bool enabled);
    void clearEnabledByDefault();

    void setEntityPicture(const char* url);
    void clearEntityPicture();

    void setQos(uint8_t qos);
    void clearQos();

    void setEncoding(const char* encoding);
    void clearEncoding();

    void setPayloadAvailable(const char* payload);
    void setPayloadNotAvailable(const char* payload);

    void setAvailabilityMode(const char* mode);

    /**
     * Adds a full-topic availability entry for discovery (`avty` list) and publishing.
     * Only used when this entity does not use shared device availability.
     */
    bool addAvailabilityEntry(
        const char* topic,
        const char* valueTemplate = nullptr,
        const char* payloadAvailable = nullptr,
        const char* payloadNotAvailable = nullptr
    );

    void clearAvailabilityEntries();

    /**
     * Sets availability of the device type.
     * Setting the initial availability enables availability reporting for this device type.
     * Please note that not all device types support this feature.
     * Follow HA documentation of a specific device type to get more information.
     *
     * @param online Specifies whether the device type is online.
     */
    virtual void setAvailability(bool online);

    /**
     * Removes this entity from MQTT discovery by publishing an empty retained
     * payload on its config topic.
     */
    bool removeFromDiscovery();

    /**
     * Republishes MQTT discovery config for this entity.
     */
    bool republishDiscovery();

#ifdef ARDUINOHA_TEST
    inline HASerializer* getSerializer() const
        { return _serializer; }

    inline void buildSerializerTest()
        { buildSerializer(); }
#endif

protected:
    /**
     * Returns instance of the HAMqtt class.
     */
    static HAMqtt* mqtt();

    /**
     * Subscribes to the given data topic.
     *
     * @param uniqueId THe unique ID of the device type assigned via the constructor.
     * @param topic Topic to subscribe (progmem string).
     */
    static void subscribeTopic(
        const char* uniqueId,
        const __FlashStringHelper* topic
    );

    /**
     * Returns true if this entity should publish the single-component
     * discovery config on connect.
     */
    bool shouldPublishSingleComponentConfig() const;

    /**
     * This method should build serializer that will be used for publishing the configuration.
     * The serializer is built each time the MQTT connection is acquired.
     * Follow implementation of the existing device types to get better understanding of the logic.
     */
    virtual void buildSerializer() { };

    /**
     * Writes shared MQTT discovery fields (enabled_by_default, entity_picture, qos, encoding).
     */
    void applyCommonEntityProperties(
        HASerializer* serializer,
        bool includeEncoding = true
    ) const;

    /**
     * Populates availability-related discovery entries. Called from HASerializer::WithAvailability.
     */
    void configureAvailabilityEntries(HASerializer* serializer) const;

    /**
     * This method is called each time the MQTT connection is acquired.
     * Each device type should publish its configuration and availability.
     * It can be also used for subscribing to MQTT topics.
     */
    virtual void onMqttConnected() = 0;

    /**
     * This method is called each time the device receives a MQTT message.
     * It can be any MQTT message so the method should always verify the topic.
     *
     * @param topic The topic on which the message was produced.
     * @param payload The payload of the message. It can be nullptr.
     * @param length The length of the payload.
     */
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    );

    /**
     * Destroys the existing serializer.
     */
    void destroySerializer();

    /**
     * Publishes configuration of this device type on the HA discovery topic.
     */
    bool publishConfig();

    /**
     * Publishes current availability of the device type.
     * The message is only produced if the availability is configured for this device type.
     */
    void publishAvailability();

    /**
     * Publishes the given flash string on the data topic.
     *
     * @param topic The topic to publish on (progmem string).
     * @param payload The message's payload (progmem string).
     * @param retained Specifies whether the message should be retained.
     */
    bool publishOnDataTopic(
        const __FlashStringHelper* topic,
        const __FlashStringHelper* payload,
        bool retained = false
    );

    /**
     * Publishes the given string on the data topic.
     *
     * @param topic The topic to publish on (progmem string).
     * @param payload The message's payload.
     * @param retained Specifies whether the message should be retained.
     */
    bool publishOnDataTopic(
        const __FlashStringHelper* topic,
        const char* payload,
        bool retained = false
    );

    /**
     * Publishes the given data on the data topic.
     *
     * @param topic The topic to publish on (progmem string).
     * @param payload The message's payload.
     * @param length The length of the payload.
     * @param retained Specifies whether the message should be retained.
     * @param isProgmemData Specifies whether the given data is stored in the flash memory.
     */
    bool publishOnDataTopic(
        const __FlashStringHelper* topic,
        const uint8_t* payload,
        const uint16_t length,
        bool retained = false,
        bool isProgmemData = false
    );

    /**
     * Publishes a RAM payload on an already fully-qualified MQTT topic.
     */
    bool publishAbsolute(const char* fullTopic, const char* payload, bool retained = false);

    /**
     * Adds the preferred entity ID property to the serializer.
     * `default_entity_id` takes precedence and the legacy `object_id` is only
     * emitted when no default entity ID was configured.
     */
    void setEntityIdProperty(HASerializer* serializer) const;

    /**
     * Returns a string pointer if it is non-empty, otherwise nullptr.
     */
    static const char* nonEmptyString(const char* value);

    /**
     * Builds a serializer used as a device discovery component payload.
     * Unsupported entities return nullptr.
     */
    virtual HASerializer* buildDeviceDiscoverySerializer();

    /**
     * Returns true when the entity can be included in MQTT device discovery.
     */
    virtual bool supportsDeviceDiscovery() const;

    /// The component name that was assigned via the constructor.
    const __FlashStringHelper* const _componentName;

    /// The unique ID that was assigned via the constructor.
    const char* _uniqueId;

    /// The name that was set using setName method. It can be nullptr.
    const char* _name;

    /// The object ID that was set using setObjectId method. It can be nullptr.
    const char* _objectId;

    /// The default entity ID used by Home Assistant on first discovery.
    const char* _defaultEntityId;

    /// The entity category for the entity. It can be nullptr.
    const char* _entityCategory;

    /// HASerializer that belongs to this device type. It can be nullptr.
    HASerializer* _serializer;

    bool _hasEnabledByDefault;
    bool _enabledByDefault;
    const char* _entityPicture;
    bool _hasQos;
    HANumeric _qosNumeric;
    const char* _encoding;
    const char* _payloadAvailable;
    const char* _payloadNotAvailable;
    const char* _availabilityMode;
    HAAvailabilityConfig _availabilityList;

private:
    enum Availability {
        AvailabilityDefault = 0,
        AvailabilityOnline,
        AvailabilityOffline
    };

    /// The current availability of this device type. AvailabilityDefault means that the initial availability was never set.
    Availability _availability;

    const char* effectivePayloadAvailable() const;
    const char* effectivePayloadNotAvailable() const;

    friend class HAMqtt;
    friend class HASerializer;
};

#endif
