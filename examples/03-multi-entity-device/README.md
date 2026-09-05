# Multi-entity Device

This example groups an Uptime sensor and writable output under one `HADevice`. It adds common metadata, one shared availability topic, and MQTT Last Will so Home Assistant marks the complete device unavailable if its network connection disappears.

Configure `include/ExampleNetwork.h`, flash the selected target, and inspect the device page in Home Assistant. Both entities belong to **ArduinoHA multi-entity example** and share its availability state.

Use this shape when one physical board exposes several related controls or sensors.

See [device and discovery](../../docs/device-and-discovery.md) and [MQTT usage](../../docs/mqtt-usage.md).
