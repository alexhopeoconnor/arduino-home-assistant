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
HASwitch output("output");

void onOutputCommand(bool state, HASwitch* sender) {
    digitalWrite(kOutputPin, state ? HIGH : LOW);
    sender->setState(state);
}

void setup() {
    Serial.begin(115200);
    pinMode(kOutputPin, OUTPUT);
    digitalWrite(kOutputPin, LOW);
    connectExampleWiFi();
    setExampleUniqueId(device);

    device.setName("ArduinoHA switch example");
    output.setName("Example output");
    output.onCommand(onOutputCommand);
    mqtt.begin(EXAMPLE_MQTT_HOST, EXAMPLE_MQTT_USER, EXAMPLE_MQTT_PASSWORD);
}

void loop() {
    mqtt.loop();
}
