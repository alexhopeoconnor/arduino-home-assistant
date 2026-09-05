# ArduinoHA examples

Start with a guided PlatformIO project when you are new to the library. Each one builds for ESP8266 and ESP32, contains safe placeholder credentials, and explains the Home Assistant result you should see.

```bash
pio run -d examples/01-esp-sensor -e esp8266
pio run -d examples/01-esp-sensor -e esp8266 -t upload
```

| Guided example | What it demonstrates |
| --- | --- |
| [ESP Sensor](01-esp-sensor/) | Wi-Fi application wiring, a unique device ID, and changing numeric telemetry |
| [Switch Callback](02-switch-callback/) | A Home Assistant command callback and local state acknowledgement |
| [Multi-entity Device](03-multi-entity-device/) | Multiple entities, shared availability, and MQTT Last Will |
| [Device Discovery](04-device-discovery/) | One device-discovery document for a newly deployed device |

## Entity recipes

The original focused sketches remain useful as short API recipes. They are intentionally transport-specific Arduino sketches, rather than full product firmware.

| Recipe | Home Assistant behaviour |
| --- | --- |
| [binary-sensor](binary-sensor/binary-sensor.ino) | Door/contact-style binary state |
| [button](button/button.ino) | Press action |
| [cover](cover/cover.ino) | Open, close, and stop commands |
| [esp32-cam](esp32-cam/esp32-cam.ino) | ESP32 camera preview |
| [fan](fan/fan.ino) | State and speed commands |
| [hvac](hvac/hvac.ino) | Modes, power, and target temperature |
| [led-switch](led-switch/led-switch.ino) | Basic writable switch |
| [light](light/light.ino) | Brightness, colour temperature, and RGB |
| [lock](lock/lock.ino) | Writable lock state |
| [multi-state-button](multi-state-button/multi-state-button.ino) | Device triggers from a wall switch |
| [multi-switch](multi-switch/multi-switch.ino) | Multiple writable switches |
| [number](number/number.ino) | Writable numeric value |
| [scene](scene/scene.ino) | Scene trigger |
| [select](select/select.ino) | Writable option list |
| [sensor](sensor/sensor.ino) | String state sensor |
| [sensor-analog](sensor-analog/sensor-analog.ino) | Analog voltage measurement |
| [sensor-integer](sensor-integer/sensor-integer.ino) | Integer uptime measurement |
| [tag-scanner](tag-scanner/tag-scanner.ino) | RFID tag reporting |

See the [entity guide](../docs/entities.md) and [project overview](../README.md).
