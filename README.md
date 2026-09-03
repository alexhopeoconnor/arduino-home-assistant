# Arduino Home Assistant integration

[![](https://img.shields.io/github/v/release/alexhopeoconnor/arduino-home-assistant?label=Version)](https://github.com/alexhopeoconnor/arduino-home-assistant/releases)
[![](https://img.shields.io/badge/Documentation-40BC13)](docs/README.md)

ArduinoHA lets an Arduino, ESP8266, or ESP32 application publish MQTT discovery data that Home Assistant understands. Give it a connected Arduino `Client`, declare entities, and Home Assistant creates the controls and telemetry automatically.

## Start with a sensor

```cpp
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensorNumber temperature("temperature");

void setup() {
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    // Your application connects Wi-Fi before MQTT begins.
    temperature.setName("Temperature");
    temperature.setUnitOfMeasurement("°C");
    mqtt.begin("mqtt.local", "mqtt_user", "mqtt_password");
}

void loop() {
    mqtt.loop();
    // Call temperature.setValue(...) when your reading changes.
}
```

Build [ESP Sensor](examples/01-esp-sensor/) for a complete ESP8266/ESP32 project. It clearly marks the network and broker values you must provide without committing credentials.

## What you can build

- **Automatic Home Assistant discovery:** retained MQTT discovery messages create entities without hand-written Home Assistant YAML.
- **Two-way controls:** sensors publish state while switches, lights, covers, and other writable entities receive explicit callbacks.
- **One physical device:** `HADevice` groups metadata, multiple entities, shared availability, and MQTT Last Will.
- **Device discovery:** publish one compact discovery document for a new multi-entity device, or keep traditional per-entity discovery.
- **Small footprint:** exclude entity implementations your firmware does not use.

## Choose an example

| Example | Learn how to… |
| --- | --- |
| [ESP Sensor](examples/01-esp-sensor/) | connect an ESP application and publish changing numeric telemetry |
| [Switch Callback](examples/02-switch-callback/) | reflect Home Assistant commands in a physical output and report state back |
| [Multi-entity Device](examples/03-multi-entity-device/) | group controls and telemetry with shared availability and Last Will |
| [Device Discovery](examples/04-device-discovery/) | publish one Home Assistant device-discovery document for a new device |
| [Entity recipes](examples/README.md#entity-recipes) | find the existing focused Arduino sketches for each supported entity |

## Install

```ini
lib_deps =
    home-assistant-integration=https://github.com/alexhopeoconnor/arduino-home-assistant.git#v3.1.0
```

PlatformIO clones the Git repository and checks out the ref after `#`; that ref is a release tag, not a GitHub Release asset. Arduino IDE is supported through the included [`library.properties`](library.properties).

See [getting started](docs/getting-started.md), the [documentation map](docs/README.md), [examples](examples/README.md), [release history](CHANGELOG.md), and [licence](LICENSE).
