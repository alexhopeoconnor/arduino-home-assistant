# ArduinoHA examples

Each directory is an Arduino sketch. Start with the network example matching your board, then pick an entity example from the table. Copy credentials into your local development configuration; do not commit them in a sketch.

| Starting point | Use it for |
| --- | --- |
| [nodemcu](nodemcu/nodemcu.ino) | Basic ESP8266 Wi-Fi and MQTT connection |
| [nano33iot](nano33iot/nano33iot.ino) | Basic Arduino Nano 33 IoT connection |
| [mqtt-with-credentials](mqtt-with-credentials/mqtt-with-credentials.ino) | MQTT authentication |
| [mqtt-advanced](mqtt-advanced/mqtt-advanced.ino) | Custom MQTT subscriptions and publishing |
| [availability](availability/availability.ino) | Per-entity availability |
| [advanced-availability](advanced-availability/advanced-availability.ino) | Shared availability and MQTT Last Will |

## Entity examples

| Example | Home Assistant behaviour |
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

The examples are intentionally small, not production firmware frameworks. For reusable Wi-Fi configuration, MQTT wiring, profiles, OTA, and migrations, use [DeviceFramework](https://github.com/alexhopeoconnor/DeviceFramework) in a consuming firmware.

See the [entity guide](../docs/entities.md) and [project overview](../README.md).
