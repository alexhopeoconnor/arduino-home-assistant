#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastActivateCallbackCall.reset();

#define assertActivateCallbackCalled(callerPtr) \
    TEST_ASSERT_TRUE(lastActivateCallbackCall.called); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastActivateCallbackCall.caller);

#define assertActivateCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastActivateCallbackCall.called);

struct ActivateCallback {
    bool called = false;
    HAScene* caller = nullptr;

    void reset() {
        called = false;
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueScene";
static ActivateCallback lastActivateCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/scene/testDevice/uniqueScene/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueScene/cmd_t"};
const char CommandMessage[] PROGMEM = {"on"};

void onCommandReceived(HAScene* caller)
{
    lastActivateCallbackCall.called = true;
    lastActivateCallbackCall.caller = caller;
}

void test_SceneTest_invalid_unique_id(void) {
    prepareTest

    HAScene scene(nullptr);
    scene.buildSerializerTest();
    HASerializer* serializer = scene.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_SceneTest_default_params(void) {
    prepareTest

    HAScene scene(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"uniq_id\":\"uniqueScene\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HAScene scene(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueScene\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HAScene scene(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueScene\":{"
                    "\"p\":\"scene\","
                    "\"uniq_id\":\"uniqueScene\","
                    "\"pl_on\":\"ON\","
                    "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_SceneTest_command_subscription(void) {
    prepareTest

    HAScene scene(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_SceneTest_availability(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HAScene
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueScene/avty_t"),
        "online",
        true
    );
}

void test_SceneTest_name_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueScene\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_object_id_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueScene\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_default_entity_id_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setDefaultEntityId("scene.test_scene");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"def_ent_id\":\"scene.test_scene\","
            "\"uniq_id\":\"uniqueScene\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_entity_category_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"uniq_id\":\"uniqueScene\","
            "\"ent_cat\":\"diagnostic\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_icon_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"uniq_id\":\"uniqueScene\","
            "\"ic\":\"testIcon\","
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_retain_setter(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        scene,
        (
            "{"
            "\"uniq_id\":\"uniqueScene\","
            "\"ret\":true,"
            "\"pl_on\":\"ON\","
            "\"cmd_t\":\"testData/testDevice/uniqueScene/cmd_t\""
            "}"
        )
    );
}

void test_SceneTest_command_callback(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), AHATOFSTR(CommandMessage));

    assertActivateCallbackCalled(&scene)
}

void test_SceneTest_no_command_callback(void) {
    prepareTest

    HAScene scene(testUniqueId);
    mock->fakeMessage(AHATOFSTR(CommandTopic), AHATOFSTR(CommandMessage));

    assertActivateCallbackNotCalled()
}

void test_SceneTest_different_scene_command(void) {
    prepareTest

    HAScene scene(testUniqueId);
    scene.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueSceneDifferent/cmd_t"),
        AHATOFSTR(CommandMessage)
    );

    assertActivateCallbackNotCalled()
}

