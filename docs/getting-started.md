# Getting started

## Prerequisites

ArduinoHA talks to Home Assistant over **MQTT** (TCP). You need an MQTT broker reachable from your board. On Home Assistant OS, the [Mosquitto add-on](https://github.com/home-assistant/addons/blob/master/mosquitto/DOCS.md) is a common choice.

## Install the library

**PlatformIO (recommended):** add the maintained release tag in
`platformio.ini`. Its [`library.json`](../library.json) resolves PubSubClient:

```ini
lib_deps =
    home-assistant-integration=https://github.com/alexhopeoconnor/arduino-home-assistant.git#v3.1.0
```

**Arduino IDE:** this fork is not indexed by Library Manager. Download the
source archive for a release, extract it, move the extracted library directory
to `<sketchbook>/libraries/home-assistant-integration`, and restart the IDE.

You also need a network `Client` (Ethernet or Wi-Fi) compatible with the
Arduino networking API.

## Minimal layout

1. Create **`HADevice`** and **`HAMqtt`** once (global or inside a long-lived object).
2. Call **`HAMqtt::begin(...)`** once, at the **end** of `setup()` — it only stores broker settings; the actual connection runs during **`HAMqtt::loop()`**.
3. Call **`mqtt.loop()`** regularly in `loop()` (not necessarily every iteration).
4. Construct **entity classes** (sensors, switches, …) before or after `HAMqtt`; ArduinoHA registers both orders. Keep all objects alive for the whole MQTT lifetime and configure discovery before the first connection.

### Ethernet (example)

```cpp
#include <Ethernet.h>
#include <ArduinoHA.h>

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};
EthernetClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

void setup() {
    Ethernet.begin(mac);
    mqtt.begin("192.168.1.50", "mqtt_user", "mqtt_password");
}

void loop() {
    Ethernet.maintain();
    mqtt.loop();
}
```

### ESP8266 / ESP32 (example)

```cpp
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

void setup() {
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    mqtt.begin("192.168.1.50", "mqtt_user", "mqtt_password");
}

void loop() {
    mqtt.loop();
}
```

## `begin()` variants

All are valid; pick one. Hostnames work instead of IP addresses.

- `mqtt.begin("192.168.1.50")` — anonymous, port 1883  
- `mqtt.begin("192.168.1.50", 8888)` — anonymous, custom port  
- `mqtt.begin("192.168.1.50", "user", "pass")` — credentials, port 1883  
- `mqtt.begin("192.168.1.50", 8888, "user", "pass")` — credentials + custom port  

`begin()` is intentionally a one-time configuration call. Reconnects are owned by `mqtt.loop()`, which uses the configured reconnect interval. Do not call `begin()` in a reconnect timer. `mqtt.disconnect()` now also produces the registered disconnected and state-change callbacks, making an explicit shutdown observable in the same way as a transport loss.

Use Home Assistant **2024.11.0 or newer** when opting into device discovery. See [Device & discovery](device-and-discovery.md) for the required staged migration from an existing single-component device.

## Security note

Credentials go over **plain TCP** unless you use a TLS-capable stack and broker setup. On a trusted LAN this is often acceptable; treat untrusted networks accordingly.

## Examples

See [`examples/`](../examples/) — start with `nodemcu` / `nano33iot` or an entity example matching what you need.
