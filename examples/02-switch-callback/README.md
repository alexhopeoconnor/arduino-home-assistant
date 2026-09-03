# Switch Callback

This example creates one writable Home Assistant switch. When Home Assistant sends a command, `onOutputCommand()` changes the local output and immediately reports the resulting state back through the supplied `HASwitch`.

The ESP8266 build uses `LED_BUILTIN`; the ESP32 build uses GPIO 2. Confirm your board’s LED polarity before treating that output as a real load.

Set the Wi-Fi and MQTT placeholders in `include/ExampleNetwork.h`, flash the board, then toggle **Example output** in Home Assistant.

See [MQTT usage](../../docs/mqtt-usage.md) and the shared [examples guide](../README.md).
