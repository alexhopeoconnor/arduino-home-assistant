# Device Discovery

This example opts into Home Assistant MQTT device discovery. Instead of one retained discovery document per entity, ArduinoHA publishes one device document containing the component definitions and richer board metadata.

Use device discovery for a **new** device on Home Assistant 2024.11.0 or newer. Do not enable it on a device that has already published traditional single-component discovery without following the documented migration procedure.

After configuring `include/ExampleNetwork.h` and flashing the board, Home Assistant discovers **ArduinoHA device discovery example** and its Uptime sensor.

See [device discovery](../../docs/device-and-discovery.md#single-component-vs-device-discovery) and the shared [examples guide](../README.md).
