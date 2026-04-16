#include <AUnit.h>
#include <ArduinoHA.h>

using aunit::TestRunner;

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueId";

const char ComponentNameStr[] PROGMEM = {"componentName"};

class DummyDeviceType : public HABaseDeviceType
{
public:
    DummyDeviceType(const __FlashStringHelper* componentName, const char* uniqueId) :
        HABaseDeviceType(componentName, uniqueId) { }

protected:
    virtual void onMqttConnected() override {
        publishAvailability();
    }
};

AHA_TEST(MqttTest, maximum_number_of_device_types) {
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device, 1);
    DummyDeviceType deviceType(AHATOFSTR(ComponentNameStr), testUniqueId);

    assertEqual((uint8_t)1, mqtt.getDevicesTypesNb());
    assertEqual(&deviceType, mqtt.getDevicesTypes()[0]);
}

AHA_TEST(MqttTest, reconnect_interval_default_value) {
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device);

    assertEqual((uint16_t)10000, mqtt.getReconnectInterval());
}

AHA_TEST(MqttTest, reconnect_interval_setter) {
    HADevice device(testDeviceId);
    HAMqtt mqtt(nullptr, device);

    mqtt.setReconnectInterval(5000);
    assertEqual((uint16_t)5000, mqtt.getReconnectInterval());

    mqtt.setReconnectInterval(0); // ignored
    assertEqual((uint16_t)5000, mqtt.getReconnectInterval());
}

AHA_TEST(MqttTest, reconnect_interval_throttles_attempts) {
    PubSubClientMock* mock = new PubSubClientMock();
    HADevice device(testDeviceId);
    HAMqtt mqtt(mock, device);
    mqtt.begin("testHost", "testUser", "testPass");
    mqtt.setReconnectInterval(50);

    assertEqual((uint16_t)0, mock->getConnectCallsNb());
    mqtt.loop();
    assertEqual((uint16_t)1, mock->getConnectCallsNb());

    mock->disconnect();
    mqtt.loop();
    assertEqual((uint16_t)1, mock->getConnectCallsNb());

    delay(60);
    mqtt.loop();
    assertEqual((uint16_t)2, mock->getConnectCallsNb());
}

void setup()
{
    delay(1000);
    Serial.begin(115200);
    while (!Serial);
}

void loop()
{
    TestRunner::run();
    delay(1);
}
