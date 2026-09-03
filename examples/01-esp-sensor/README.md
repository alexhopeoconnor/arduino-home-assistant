# ESP Sensor

This is the first complete ArduinoHA project for an ESP8266 D1 mini or ESP32 development board. It connects Wi-Fi, derives a stable device ID from the board MAC address, and publishes a changing **Uptime** sensor to Home Assistant.

Before uploading, replace the `EXAMPLE_*` values in `include/ExampleNetwork.h`, or provide them as PlatformIO build flags. The header contains safe placeholders and must not contain real credentials when committed.

After MQTT connects, Home Assistant discovers **ArduinoHA sensor example** and its Uptime entity. Rebooting the board keeps the same Home Assistant identity because its MAC address is stable.

See [getting started](../../docs/getting-started.md) and the shared [examples guide](../README.md).
