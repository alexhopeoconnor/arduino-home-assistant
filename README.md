# Arduino Home Assistant integration

[![](https://img.shields.io/github/v/release/alexhopeoconnor/arduino-home-assistant?label=Version)](https://github.com/alexhopeoconnor/arduino-home-assistant/releases)
[![](https://img.shields.io/badge/Documentation-40BC13)](docs/README.md)

ArduinoHA is the maintained MQTT-discovery library behind compact Arduino, ESP8266, and ESP32 integrations with Home Assistant. It uses Arduino's standard network `Client` API and is continuously compile-tested on ESP8266 and ESP32.

## Why use it

- **Home Assistant discovery:** entities appear automatically from retained MQTT discovery payloads.
- **Two-way entities:** report local state and receive Home Assistant commands with a small, explicit API.
- **One physical device:** group multiple entities, metadata, shared availability, and MQTT Last Will under `HADevice`.
- **Control the footprint:** compile out entity implementations a firmware does not use.
- **Two discovery shapes:** start with one payload per entity or opt into a single device-discovery payload.

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

    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    temperature.setName("Temperature");
    temperature.setUnitOfMeasurement("°C");
    mqtt.begin("mqtt.local", "mqtt_user", "mqtt_password");
}

void loop() {
    mqtt.loop();
    // Call temperature.setValue(...) when your reading changes.
}
```

Read [Getting started](docs/getting-started.md) before copying this into production: it explains object lifetime, MQTT lifecycle, ESP32 includes, and install routes.

## Install

```ini
lib_deps =
    home-assistant-integration=https://github.com/alexhopeoconnor/arduino-home-assistant.git#v3.0.0
```

PlatformIO clones the Git repository and checks out the ref after `#`; that ref is a release tag, not a GitHub Release asset. Arduino IDE is supported through the included [`library.properties`](library.properties); see [Getting started](docs/getting-started.md#install-the-library).

## Documentation

The [documentation map](docs/README.md) is the starting point:

- [Getting started](docs/getting-started.md): connection lifecycle and minimal sketches.
- [Device and discovery](docs/device-and-discovery.md): device metadata, discovery modes, and runtime refresh.
- [MQTT usage](docs/mqtt-usage.md): callbacks, availability, custom MQTT, logging, and footprint flags.
- [Entity guide](docs/entities.md): supported entity types and the best matching example.
- [Examples](examples/README.md): curated entry points and the full example index.

## Development and releases

```bash
./scripts/test.sh compile --platform esp8266
./scripts/test.sh compile --platform esp32
./scripts/check-docs.sh
./scripts/prepare-release.sh vMAJOR.MINOR.PATCH --tag
```

The release preflight validates both package manifests and the matching changelog section. A pushed tag repeats the board-free compile checks and creates a GitHub Release from that section; it does not publish to the PlatformIO Registry or deploy firmware.

See the [changelog](CHANGELOG.md) and [licence](LICENSE).
