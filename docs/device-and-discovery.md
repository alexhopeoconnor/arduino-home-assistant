# Device & discovery

## `HADevice`

Represents the physical board in Home Assistant: one device can expose multiple entities (sensors, switches, lights, …).

**Unique ID** (required): must be unique in your Home Assistant instance. Common choices:

- MAC address as bytes: `HADevice device(mac, sizeof(mac));`
- String: `HADevice device("myId");` — keep it short and alphanumeric.
- Or default-construct and call `setUniqueId(bytes, length)` in `setup()`.

**Optional metadata** (each costs some RAM/flash; skip on tiny MCUs unless needed):

- `setName`, `setSoftwareVersion`, `setManufacturer`, `setModel`, `setConfigurationUrl`
- `setModelId`, `setHardwareVersion`, `setSerialNumber`, `setSuggestedArea`, `setViaDevice`
- `addConnection("mac", "aa:bb:cc:dd:ee:ff")` or `setConnectionsJson("[[\"mac\",\"aa:bb:cc:dd:ee:ff\"]]")`

String setters take **pointers whose contents are not copied** — use literals or storage that outlives the call.

## Discovery

When MQTT connects, the library publishes Home Assistant **MQTT discovery** payloads so entities appear automatically.

Entities can be constructed either before or after `HAMqtt`. Entities which already
exist when `HAMqtt` is constructed are registered then; entities constructed later
register immediately. Both the device, MQTT object, and entities must have a lifetime
that outlasts `mqtt.loop()`.

`HAMqtt` has a configurable entity limit (24 by default). Check
`getRegisteredDeviceTypeCount()`, `getDeviceTypeLimit()`, and `getDeviceTypeRegistrationFailures()` in firmware diagnostics: registrations above the limit are rejected and logged rather than silently disappearing from discovery.

### Topic prefixes

Defaults:

- Discovery prefix: `homeassistant`
- Data prefix (states, commands): `aha`

Override before `begin()` if needed:

```cpp
mqtt.setDiscoveryPrefix("myHaPrefix");
mqtt.setDataPrefix("myDataPrefix");
```

### Single-component vs device discovery

- **Default:** one retained discovery topic per entity (single-component discovery).
- **Device discovery:** `HAMqtt::enableDeviceDiscovery()` publishes one retained device payload with components under `cmps`.

Both formats remain supported by Home Assistant. Device discovery requires Home Assistant **2024.11.0 or newer**. Use `enableDeviceDiscovery()` only for a new device which has never published this library's single-component discovery topics.

### Migrating an existing device to device discovery

Do not switch an existing device by calling `enableDeviceDiscovery()` alone. Home Assistant derives an entity's discovery identity from the topic, and a direct switch causes retained-topic conflicts. The migration deliberately publishes, in order:

1. `{"migrate_discovery":true}` to every old retained component config topic;
2. the retained `/device/<deviceId>/config` payload; and only then
3. an empty retained payload to every old component config topic.

Keep the device ID and each entity ID unchanged. They preserve the mapping from the old `<component>/<deviceId>/<entityId>/config` topic to `cmps[entityId]` in the new device payload.

Start migration once during setup, then advance *one stage per loop iteration* after the MQTT connection is available. Each method is retry-safe: do not advance if it returns `false`.

```cpp
bool migrationStarted = false;

void setup() {
    // Create device, mqtt, and entities; keep their IDs unchanged.
    migrationStarted = mqtt.beginDeviceDiscoveryMigration();
    mqtt.begin("192.168.1.50", "user", "password");
}

void loop() {
    mqtt.loop();

    switch (mqtt.getDeviceDiscoveryMigrationState()) {
    case HAMqtt::DeviceDiscoveryMigrationMarkersPending:
        mqtt.publishDeviceDiscoveryMigrationMarkers();
        break;
    case HAMqtt::DeviceDiscoveryMigrationMarkersPublished:
        mqtt.publishDeviceDiscoveryMigrationConfig();
        break;
    case HAMqtt::DeviceDiscoveryMigrationDevicePublished:
        mqtt.completeDeviceDiscoveryMigration();
        break;
    default:
        break;
    }
}
```

The migration stage is held in RAM. If the board reboots before completion, start the same migration again; the retained marker/config/cleanup publishes are idempotent. If you must abandon a started migration while connected, call `rollbackDeviceDiscoveryMigration()`. Once a device config has been published, it first writes `{"migrate_discovery":true}` to the device discovery topic, restores the legacy configs, then clears the device config and returns to single-component mode. A failed rollback remains pending and suppresses automatic device-bundle publication until retried successfully. Run all publishing stages from `loop()`, not inside an inbound MQTT callback.

Device discovery can also publish richer origin/device metadata, for example:

```cpp
device.setModelId("esp32-s3-devkit");
device.setHardwareVersion("rev-b");
device.setSerialNumber("SN-00042");
device.setSuggestedArea("Garage");
device.setViaDevice("main_gateway");
device.addConnection("mac", "AA:BB:CC:DD:EE:FF");
mqtt.setOriginSupportUrl("https://example.com/device-help");
```

For entity identifiers in Home Assistant, prefer **`setDefaultEntityId()`** over legacy **`setObjectId()`**.
`setObjectId()` remains source-compatible but no longer serializes the removed MQTT `obj_id` property. `setDefaultEntityId()` influences Home Assistant's entity ID only when it first creates the entity; existing users can retain a customized entity ID in the entity registry.

Common entity discovery metadata can be configured on most entity types via:

- `setEnabledByDefault(bool)`
- `setEntityPicture(const char*)`
- `setQos(uint8_t)`
- `setEncoding(const char*)`
- `setEntityCategory(const char*)`

### Runtime discovery changes

After changing discovery-related settings at runtime:

- `HABaseDeviceType::republishDiscovery()` to refresh discovery.
- `HABaseDeviceType::removeFromDiscovery()` to remove one entity from discovery.

In single-component mode, removal clears that entity's retained config topic. In device discovery mode, Home Assistant requires a platform-only component marker (`{"p":"sensor"}`, for example), followed by a compacted device bundle that omits the component. ArduinoHA performs both publishes. Call `republishDiscovery()` on that entity to add it back.

## Identifier and lifetime checklist

- Give `HADevice` a stable, non-empty unique ID and each entity a stable, topic-safe ID (`A-Z`, `a-z`, `0-9`, `_`, `-`).
- Use `enableExtendedUniqueIds()` when multiple devices could otherwise reuse the same entity IDs.
- Metadata setters store pointers; retain their backing strings. `HAText` copies the current state and command text it receives, so its state does not borrow a transient MQTT buffer.

## Trimming flash (optional)

You can exclude unused entity implementations with macros (see [MQTT usage](mqtt-usage.md#compiler-macros)).
