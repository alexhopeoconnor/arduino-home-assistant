#pragma once
#if defined(ESP32)
#include <Network.h>
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#ifndef EXAMPLE_WIFI_SSID
#define EXAMPLE_WIFI_SSID "replace-with-wifi-name"
#endif
#ifndef EXAMPLE_WIFI_PASSWORD
#define EXAMPLE_WIFI_PASSWORD "replace-with-wifi-password"
#endif
#ifndef EXAMPLE_MQTT_HOST
#define EXAMPLE_MQTT_HOST "replace-with-mqtt-host"
#endif
#ifndef EXAMPLE_MQTT_USER
#define EXAMPLE_MQTT_USER ""
#endif
#ifndef EXAMPLE_MQTT_PASSWORD
#define EXAMPLE_MQTT_PASSWORD ""
#endif

inline void connectExampleWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(EXAMPLE_WIFI_SSID, EXAMPLE_WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }
}

inline void setExampleUniqueId(HADevice& device) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));
}
