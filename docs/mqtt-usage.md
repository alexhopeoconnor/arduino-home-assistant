# MQTT usage

## Callbacks and tuning

`HAMqtt` supports optional callbacks and PubSubClient tuning:

```cpp
void onMessage(const char* topic, const uint8_t* payload, uint16_t length) { /* ... */ }
void onConnected() { /* ... */ }
void onDisconnected() { /* ... */ }
void onStateChanged(HAMqtt::ConnectionState state) { /* ... */ }

void setup() {
    mqtt.onMessage(onMessage);
    mqtt.onConnected(onConnected);
    mqtt.onDisconnected(onDisconnected);
    mqtt.onStateChanged(onStateChanged);
    mqtt.setBufferSize(512);   // default 256
    mqtt.setKeepAlive(60);     // seconds, default 15
    mqtt.begin("192.168.1.50", "user", "pass");
}
```

## Custom subscriptions

Subscribe after each successful connection (for example in `onConnected`), because subscriptions are tied to the MQTT session:

```cpp
void onConnected() {
    mqtt.subscribe("my/custom/topic");
}
```

Handle payloads in `onMessage`.

## Publishing arbitrary payloads

```cpp
mqtt.publish("customTopic", "payload", false);  // not retained
mqtt.publish("customTopic", "payload", true);     // retained
```

## Availability

**Shared availability (recommended):** one availability topic for the whole device — works well with **Last Will** (LWT):

```cpp
device.enableSharedAvailability();
device.enableLastWill();   // broker publishes offline when TCP drops
// device.setAvailability(false);  // optional: start as offline
```

**Per-entity availability:** call `someEntity.setAvailability(true/false)` on each type. Does not use LWT the same way as shared mode; see examples under `examples/availability/`.

## Compiler macros

Defined in `ArduinoHADefines.h` or via build flags.

- **`ARDUINOHA_DEBUG`** — enables library logging (useful when debugging MQTT). By default lines go to **`Serial`** unless you install a sink with **`arduinoHASetLogSink(ArduinoHALogSink*)`** (for example **DeviceFramework** forwards debug output into its main log stream).

**Exclude unused device types** (saves flash from vtables), e.g.:

`EX_ARDUINOHA_BINARY_SENSOR`, `EX_ARDUINOHA_BUTTON`, `EX_ARDUINOHA_CAMERA`, `EX_ARDUINOHA_COVER`, `EX_ARDUINOHA_DEVICE_TRACKER`, `EX_ARDUINOHA_DEVICE_TRIGGER`, `EX_ARDUINOHA_FAN`, `EX_ARDUINOHA_HVAC`, `EX_ARDUINOHA_LIGHT`, `EX_ARDUINOHA_LOCK`, `EX_ARDUINOHA_NUMBER`, `EX_ARDUINOHA_SCENE`, `EX_ARDUINOHA_SELECT`, `EX_ARDUINOHA_SENSOR`, `EX_ARDUINOHA_SWITCH`, `EX_ARDUINOHA_TAG_SCANNER`
