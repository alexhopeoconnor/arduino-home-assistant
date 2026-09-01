# Entities

ArduinoHA supports these Home Assistant MQTT discovery entity classes. Choose the focused example where possible; it shows the entity's update or command callback pattern in a runnable sketch.

| Entity | Example |
| --- | --- |
| Binary sensor | [binary-sensor](../examples/binary-sensor/binary-sensor.ino) |
| Button | [button](../examples/button/button.ino) |
| Camera | [esp32-cam](../examples/esp32-cam/esp32-cam.ino) |
| Cover | [cover](../examples/cover/cover.ino) |
| Device tracker | Use the API header; no dedicated sketch yet |
| Device trigger | [multi-state-button](../examples/multi-state-button/multi-state-button.ino) |
| Fan | [fan](../examples/fan/fan.ino) |
| HVAC | [hvac](../examples/hvac/hvac.ino) |
| Light | [light](../examples/light/light.ino) |
| Lock | [lock](../examples/lock/lock.ino) |
| Number | [number](../examples/number/number.ino) |
| Scene | [scene](../examples/scene/scene.ino) |
| Select | [select](../examples/select/select.ino) |
| Sensor | [sensor](../examples/sensor/sensor.ino), [sensor-analog](../examples/sensor-analog/sensor-analog.ino), or [sensor-integer](../examples/sensor-integer/sensor-integer.ino) |
| Switch | [led-switch](../examples/led-switch/led-switch.ino) or [multi-switch](../examples/multi-switch/multi-switch.ino) |
| Tag scanner | [tag-scanner](../examples/tag-scanner/tag-scanner.ino) |
| Text | Use the API header; no dedicated sketch yet |

The library does not currently implement alarm control panels, events, humidifiers, images, lawn mowers, sirens, updates, vacuums, valves, or water heaters. Those are deliberate unsupported surfaces, not configuration switches.

## Common lifecycle

Create long-lived `HADevice`, `HAMqtt`, and entity objects, then call `mqtt.begin(...)` once and `mqtt.loop()` regularly. Entities may be constructed before or after `HAMqtt`; both orders register safely. Read [Getting started](getting-started.md) for the connection flow and [device discovery](device-and-discovery.md) for discovery settings.

For shared online/offline state, use `device.enableSharedAvailability()` and `device.enableLastWill()` before connecting. The [availability examples](../examples/availability/) show the simplest setup, while [advanced availability](../examples/advanced-availability/) covers custom payloads and Last Will behaviour.

Back to the [documentation map](README.md).
