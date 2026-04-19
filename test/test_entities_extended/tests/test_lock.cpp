#include <Arduino.h>
#include <ArduinoHA.h>
#include "../../shared/aha_unity_helpers.h"

#define prepareTest \
    initMqttTest(testDeviceId) \
    lastCommandCallbackCall.reset();

#define assertCommandCallbackCalled(expectedCommand, callerPtr) \
    TEST_ASSERT_TRUE(lastCommandCallbackCall.called); \
    TEST_ASSERT_EQUAL(expectedCommand, lastCommandCallbackCall.command); \
    TEST_ASSERT_EQUAL_PTR(callerPtr, lastCommandCallbackCall.caller);

#define assertCommandCallbackNotCalled() \
    TEST_ASSERT_FALSE(lastCommandCallbackCall.called);

struct CommandCallback {
    bool called = false;
    HALock::LockCommand command = static_cast<HALock::LockCommand>(0);
    HALock* caller = nullptr;

    void reset() {
        called = false;
        command = static_cast<HALock::LockCommand>(0);
        caller = nullptr;
    }
};

static const char* testDeviceId = "testDevice";
static const char* testUniqueId = "uniqueLock";
static CommandCallback lastCommandCallbackCall;

const char ConfigTopic[] PROGMEM = {"homeassistant/lock/testDevice/uniqueLock/config"};
const char DeviceConfigTopic[] PROGMEM = {"homeassistant/device/testDevice/config"};
const char CommandTopic[] PROGMEM = {"testData/testDevice/uniqueLock/cmd_t"};
const char StateTopic[] PROGMEM = {"testData/testDevice/uniqueLock/stat_t"};

void onCommandReceived(HALock::LockCommand command, HALock* caller)
{
    lastCommandCallbackCall.called = true;
    lastCommandCallbackCall.command = command;
    lastCommandCallbackCall.caller = caller;
}

void test_LockTest_invalid_unique_id(void) {
    prepareTest

    HALock lock(nullptr);
    lock.buildSerializerTest();
    HASerializer* serializer = lock.getSerializer();

    TEST_ASSERT_TRUE(serializer == nullptr);
}

void test_LockTest_default_params(void) {
    prepareTest

    HALock lock(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"uniqueLock\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_LockTest_extended_unique_id(void) {
    prepareTest

    device.enableExtendedUniqueIds();
    HALock lock(testUniqueId);
    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"testDevice_uniqueLock\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_LockTest_device_discovery_payload(void) {
    prepareTest

    mqtt.enableDeviceDiscovery();
    HALock lock(testUniqueId);
    mqtt.loop();

    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, 
        AHATOFSTR(DeviceConfigTopic),
        (
            "{"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"o\":{\"name\":\"ArduinoHA\",\"sw\":\"2.1.0\"},"
            "\"cmps\":{"
                "\"uniqueLock\":{"
                    "\"p\":\"lock\","
                    "\"uniq_id\":\"uniqueLock\","
                    "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
                    "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
                "}"
            "}"
            "}"
        ),
        true
    );
}

void test_LockTest_command_subscription(void) {
    prepareTest

    HALock lock(testUniqueId);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getSubscriptionsNb());
    AHA_ASSERT_EQUAL_FLASH_TOPIC(AHATOFSTR(CommandTopic), mock->getSubscriptions()[0]->topic);
}

void test_LockTest_availability(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setAvailability(true);
    mqtt.loop();

    // availability is published after config in HALock
    AHA_ASSERT_MQTT_MESSAGE(mock, 
        1,
        F("testData/testDevice/uniqueLock/avty_t"),
        "online",
        true
    );
}

void test_LockTest_publish_last_known_state(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setCurrentState(HALock::StateUnlocked);
    mqtt.loop();

    TEST_ASSERT_EQUAL(2, mock->getFlushedMessagesNb());
    AHA_ASSERT_MQTT_MESSAGE(mock, 1, AHATOFSTR(StateTopic), "UNLOCKED", true);
}

void test_LockTest_publish_nothing_if_retained(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setRetain(true);
    lock.setCurrentState(HALock::StateUnlocked);
    mqtt.loop();

    TEST_ASSERT_EQUAL(1, mock->getFlushedMessagesNb()); // only config should be pushed
}

void test_LockTest_name_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setName("testName");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"name\":\"testName\","
            "\"uniq_id\":\"uniqueLock\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_object_id_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setObjectId("testId");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"obj_id\":\"testId\","
            "\"uniq_id\":\"uniqueLock\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_default_entity_id_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setDefaultEntityId("lock.test_lock");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"def_ent_id\":\"lock.test_lock\","
            "\"uniq_id\":\"uniqueLock\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_entity_category_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setEntityCategory("diagnostic");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"uniqueLock\","
            "\"ent_cat\":\"diagnostic\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_icon_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setIcon("testIcon");

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"uniqueLock\","
            "\"ic\":\"testIcon\","
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_retain_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setRetain(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"uniqueLock\","
            "\"ret\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_optimistic_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setOptimistic(true);

    AHA_ASSERT_ENTITY_CONFIG(
        mock,
        lock,
        (
            "{"
            "\"uniq_id\":\"uniqueLock\","
            "\"opt\":true,"
            "\"dev\":{\"ids\":\"testDevice\"},"
            "\"stat_t\":\"testData/testDevice/uniqueLock/stat_t\","
            "\"cmd_t\":\"testData/testDevice/uniqueLock/cmd_t\""
            "}"
        )
    );
}

void test_LockTest_current_state_setter(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.setCurrentState(HALock::StateLocked);

    TEST_ASSERT_EQUAL(0, mock->getFlushedMessagesNb());
    TEST_ASSERT_EQUAL(HALock::StateLocked, lock.getCurrentState());
}

void test_LockTest_publish_state_locked(void) {
    prepareTest

    mock->connectDummy();
    HALock lock(testUniqueId);

    TEST_ASSERT_TRUE(lock.setState(HALock::StateLocked));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "LOCKED", true);
}

void test_LockTest_publish_state_unlocked(void) {
    prepareTest

    mock->connectDummy();
    HALock lock(testUniqueId);

    TEST_ASSERT_TRUE(lock.setState(HALock::StateUnlocked));
    AHA_ASSERT_SINGLE_MQTT_MESSAGE(mock, AHATOFSTR(StateTopic), "UNLOCKED", true);
}

void test_LockTest_command_lock(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("LOCK"));

    assertCommandCallbackCalled(HALock::CommandLock, &lock)
}

void test_LockTest_command_unlock(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("UNLOCK"));

    assertCommandCallbackCalled(HALock::CommandUnlock, &lock)
}

void test_LockTest_command_open(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("OPEN"));

    assertCommandCallbackCalled(HALock::CommandOpen, &lock)
}

void test_LockTest_command_invalid(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.onCommand(onCommandReceived);
    mock->fakeMessage(AHATOFSTR(CommandTopic), F("NOT_SUPPORTED"));

    assertCommandCallbackNotCalled()
}

void test_LockTest_different_lock_command(void) {
    prepareTest

    HALock lock(testUniqueId);
    lock.onCommand(onCommandReceived);
    mock->fakeMessage(
        F("testData/testDevice/uniqueLockDifferent/cmd_t"),
        F("CLOSE")
    );

    assertCommandCallbackNotCalled()
}

