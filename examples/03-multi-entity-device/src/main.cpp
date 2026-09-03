#include <Arduino.h>
#include <ArduinoHA.h>
#include "ExampleNetwork.h"

#if defined(ESP32)
constexpr uint8_t kOutputPin = 2;
#else
constexpr uint8_t kOutputPin = LED_BUILTIN;
#endif

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensorNumber uptime("uptime");
HASwitch output("output");

void onOutputCommand(bool state, HASwitch* sender) {
    digitalWrite(kOutputPin, state ? HIGH : LOW);
    sender->setState(state);
}

void setup() {
    Serial.begin(115200);
    pinMode(kOutputPin, OUTPUT);
    connectExampleWiFi();
    setExampleUniqueId(device);

    device.setName("ArduinoHA multi-entity example");
    device.setManufacturer("Example Devices");
    device.setSoftwareVersion("1.0.0");
    device.enableSharedAvailability();
    device.enableLastWill();

    uptime.setName("Uptime");
    uptime.setUnitOfMeasurement("s");
    output.setName("Example output");
    output.onCommand(onOutputCommand);
    mqtt.begin(EXAMPLE_MQTT_HOST, EXAMPLE_MQTT_USER, EXAMPLE_MQTT_PASSWORD);
}

void loop() {
    mqtt.loop();
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) {
        uptime.setValue(static_cast<uint32_t>(millis() / 1000));
        lastUpdate = millis();
    }
}
