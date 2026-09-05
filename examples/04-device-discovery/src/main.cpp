#include <Arduino.h>
#include <ArduinoHA.h>
#include "ExampleNetwork.h"

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensorNumber uptime("uptime");

void setup() {
    Serial.begin(115200);
    connectExampleWiFi();
    setExampleUniqueId(device);

    device.setName("ArduinoHA device discovery example");
    device.setManufacturer("Example Devices");
    device.setModel("ESP example");
    device.setSoftwareVersion("1.0.0");
    uptime.setName("Uptime");
    uptime.setUnitOfMeasurement("s");

    mqtt.enableDeviceDiscovery();
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
