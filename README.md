# Arduino Home Assistant integration 🏠

[![](https://img.shields.io/github/v/release/alexhopeoconnor/arduino-home-assistant?label=Version)](https://github.com/alexhopeoconnor/arduino-home-assistant/releases)
[![](https://img.shields.io/badge/Documentation-40BC13)](https://github.com/alexhopeoconnor/arduino-home-assistant/blob/main/docs/README.md)
[![](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&color=%23fe8e86)](https://github.com/sponsors/dawidchyrzynski)

ArduinoHA integrates Arduino- and ESP-based devices with Home Assistant over MQTT.
It is designed to keep RAM and flash use low. The original project targeted the
Arduino Uno with an Ethernet Shield; this maintained fork is continuously tested
on ESP8266 and ESP32 release targets.

## Install

### PlatformIO

[`library.json`](library.json) declares the **PubSubClient** dependency. Pin the
maintained release tag in an application:

```ini
lib_deps =
    home-assistant-integration=https://github.com/alexhopeoconnor/arduino-home-assistant.git#v3.0.0
```

The suffix after `#` is a Git ref. PlatformIO clones this repository and checks
out that tag; it does not download a GitHub Release asset.

### Arduino IDE

This maintained fork is not published through the Arduino Library Manager.
Download the source archive for a release, extract it, move the extracted library
directory to `<sketchbook>/libraries/home-assistant-integration`, then restart the IDE.
[`library.properties`](library.properties) provides the local library name and
metadata expected by the Arduino IDE.

## Tests and releases

CI compile-checks the PlatformIO Unity suite for the ESP8266 and ESP32 without
a board:

```bash
pio test -e esp8266 --without-uploading --without-testing
pio test -e esp32 --without-uploading --without-testing
```

Before publishing a version, update `library.json`, `library.properties`,
`CHANGELOG.md`, and the relevant documentation. Validate and create the
annotated tag with:

```bash
./scripts/prepare-release.sh vMAJOR.MINOR.PATCH --tag
```

Push the branch and tag. The tag workflow validates the PlatformIO package
again and creates the GitHub Release.

## Features

* Two-way communication (state reporting and command execution)
* MQTT discovery (device is added to the Home Assistant panel automatically)
* Rich discovery metadata for entities, devices, origin, templates and availability
* MQTT Last Will and Testament
* Support for custom MQTT messages (publishing and subscribing)
* Auto reconnect with MQTT broker
* Reporting availability (online/offline states) of a device
* Markdown documentation in [`docs/`](docs/README.md); class-level notes in headers under `src/`
* Covered by unit tests ([PlatformIO](https://platformio.org/) + [Unity](https://github.com/ThrowTheSwitch/Unity); see `test/test_*`)

## Discovery Notes

ArduinoHA supports two MQTT discovery modes:

* Single-component discovery, which remains the default behavior and publishes one retained discovery payload per entity.
* Device discovery, which can be enabled explicitly with `HAMqtt::enableDeviceDiscovery()` and publishes a single retained `homeassistant/device/.../config` payload with component mappings under `cmps`.

For entity ID suggestions in Home Assistant, prefer `setDefaultEntityId()` over `setObjectId()`.
`setObjectId()` is still available as a legacy fallback, but newer Home Assistant versions are moving toward `default_entity_id`.

If you need to manage discovery at runtime:

* Use `HABaseDeviceType::republishDiscovery()` after changing discovery-relevant config at runtime.
* Use `HABaseDeviceType::removeFromDiscovery()` to clear the retained discovery payload for a single entity.

When device discovery mode is enabled, runtime discovery refreshes automatically clear any stale retained per-entity config before republishing the device discovery payload.

Recent discovery additions include:

* Shared entity metadata such as `enabled_by_default`, `entity_picture`, `qos`, `encoding`
* Device/origin metadata such as `model_id`, `hw_version`, `serial_number`, `suggested_area`, `via_device`, `connections`, `support_url`
* Availability payload overrides and multi-topic availability discovery metadata

## Supported HA types

| Home Assistant type | Supported |
| ------------------- | :--------: |
| Alarm control panel |     ❌     |
| Binary sensor       |     ✅     |
| Button              |     ✅     |
| Camera              |     ✅     |
| Cover               |     ✅     |
| Device tracker      |     ✅     |
| Device trigger      |     ✅     |
| Event               |     ❌     |
| Fan                 |     ✅     |
| Humidifier          |     ❌     |
| Image               |     ❌     |
| HVAC                |     ✅     |
| Lawn mower          |     ❌     |
| Light               |     ✅     |
| Lock                |     ✅     |
| Number              |     ✅     |
| Scene               |     ✅     |
| Select              |     ✅     |
| Sensor              |     ✅     |
| Siren               |     ❌     |
| Switch              |     ✅     |
| Update              |     ❌     |
| Tag scanner         |     ✅     |
| Text                |     ✅     |
| Vacuum              |     ❌     |
| Valve               |     ❌     |
| Water heater        |     ❌     |

## Examples

|Example|Description                  |
|-------|-----------------------------|
|[Binary sensor](examples/binary-sensor/binary-sensor.ino)|Using the binary sensor as a door contact sensor.|
|[Button](examples/button/button.ino)|Adding simple buttons to the Home Assistant panel.|
|[Camera](examples/esp32-cam/esp32-cam.ino)|Publishing the preview from the ESP32-CAM module.|
|[Cover](examples/cover/cover.ino)|Controlling a window cover (open / close / stop).|
|[Device trigger](examples/multi-state-button/multi-state-button.ino)|Implementation of a simple wall switch that reports press and hold states.|
|[Fan](examples/fan/fan.ino)|Controlling a simple fan (state + speed).|
|[HVAC](examples/hvac/hvac.ino)|HVAC controller with multiple modes, power control and target temperature.|
|[Lock](examples/lock/lock.ino)|A simple door lock that's controlled by the Home Assistant.|
|[Light](examples/light/light.ino)|A simple light that allows changing brightness, color temperature and RGB color.|
|[Number](examples/number/number.ino)|Adding an interactive numeric slider in the Home Assistant panel.|
|[Scene](examples/scene/scene.ino)|Adding a custom scene in the Home Assistant panel. |
|[Select](examples/select/select.ino)|A dropdown selector that's displayed in the Home Assistant panel.|
|[Sensor](examples/sensor/sensor.ino)|A simple sensor that reports a state in a string representation (open / opening / close).|
|[Analog sensor](examples/sensor-analog/sensor-analog.ino)|Reporting the analog pin's voltage to the Home Assistant.|
|[Integer sensor](examples/sensor-integer/sensor-integer.ino)|Reporting the device's uptime to the Home Assistant.|
|[Switch](examples/led-switch/led-switch.ino)|The LED that's controlled by the Home Assistant.|
|[Multi-switch](examples/multi-switch/multi-switch.ino)|Multiple switches controlled by the Home Assistant.|
|[Tag scanner](examples/tag-scanner/tag-scanner.ino)|Scanning RFID tags using the MFRC522 module.|
|[Availability](examples/availability/availability.ino)|Reporting entities' availability (online / offline) to the Home Assistant.|
|[Advanced availability](examples/advanced-availability/advanced-availability.ino)|Advanced availability reporting with MQTT LWT (Last Will and Testament).|
|[MQTT advanced](examples/mqtt-advanced/mqtt-advanced.ino)|Subscribing to custom topics and publishing custom messages.|
|[MQTT with credentials](examples/mqtt-with-credentials/mqtt-with-credentials.ino)|Establishing connection with a MQTT broker using the credentials. |
|[NodeMCU (ESP8266)](examples/nodemcu/nodemcu.ino)|Basic example for ESP8266 devices.|
|[Arduino Nano 33 IoT](examples/nano33iot/nano33iot.ino)|Basic example for Arduino Nano 33 IoT (SAMD family).|
|[mDNS discovery](examples/mdns/mdns.ino)|Make your ESP8266 discoverable via the mDNS.|

## Supported and compatible hardware

ArduinoHA is designed around Arduino's network `Client` API and can be used
with Ethernet or Wi-Fi clients that implement it. This fork's automated
PlatformIO coverage is ESP8266 (Wemos D1 mini) and ESP32 (ESP32 DevKit).
Other Arduino targets may work, but they are compatibility targets rather than
a tested release guarantee; validate the compiler, network client, memory
budget, and Home Assistant discovery behavior in the consuming project.
