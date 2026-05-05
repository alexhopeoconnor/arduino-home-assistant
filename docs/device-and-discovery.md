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

**Rule:** construct entity objects **after** `HAMqtt` so they can register with it.

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
- **Optional:** call `HAMqtt::enableDeviceDiscovery()` to publish a single **device** discovery payload with components under `cmps` (see project README for migration notes).

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

Common entity discovery metadata can be configured on most entity types via:

- `setEnabledByDefault(bool)`
- `setEntityPicture(const char*)`
- `setQos(uint8_t)`
- `setEncoding(const char*)`
- `setEntityCategory(const char*)`

### Runtime discovery changes

After changing discovery-related settings at runtime:

- `HABaseDeviceType::republishDiscovery()` to refresh discovery.
- `HABaseDeviceType::removeFromDiscovery()` to clear retained discovery for one entity.

If device discovery mode is enabled, the library clears stale per-entity configs when refreshing.

## Trimming flash (optional)

You can exclude unused entity implementations with macros (see [MQTT usage](mqtt-usage.md#compiler-macros)).
