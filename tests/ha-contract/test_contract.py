"""Home Assistant MQTT discovery contract checks.

The test intentionally uses retained MQTT messages, like a deployed firmware
node. It inspects HA's entity registry over the authenticated WebSocket API so
the migration assertion is about HA's persistent identity, not just payload
shape.
"""

import json
import os
import pathlib
import time

from ha_mqtt_contract import (
    ContractError,
    HomeAssistantClient,
    RetainedPublisher as SharedRetainedPublisher,
    wait_until,
)


HA_URL = os.environ.get("HA_URL", "http://homeassistant:8123").rstrip("/")
MQTT_HOST = os.environ.get("MQTT_HOST", "mqtt")
MQTT_PORT = int(os.environ.get("MQTT_PORT", "1883"))
MODE = os.environ.get("CONTRACT_MODE", "migration")
STATE = pathlib.Path("/state")
TOKEN_FILE = STATE / "ha-token"
DEVICE_ID = "contract_device"
EDGE_DEVICE_ID = "contract_edge_device"
EXPECT_DISABLED_CLEANUP = os.environ.get("CONTRACT_EXPECT_DISABLED_CLEANUP") == "1"


def fail(message):
    raise ContractError(message)


def legacy_topic(object_id, device_id=DEVICE_ID):
    return f"homeassistant/sensor/{device_id}/{object_id}/config"


def device_topic(device_id=DEVICE_ID):
    return f"homeassistant/device/{device_id}/config"


def component(object_id, unique_id=None, device_id=DEVICE_ID, **extra):
    payload = {
        "p": "sensor",
        "name": object_id.replace("_", " ").title(),
        "uniq_id": unique_id or f"{device_id}_{object_id}",
        "stat_t": f"contract/{object_id}/state",
    }
    payload.update(extra)
    return payload


def device_payload(components, device_id=DEVICE_ID):
    return {
        "dev": {"ids": [device_id], "name": "ArduinoHA contract device"},
        "o": {"name": "ArduinoHA", "sw": "contract"},
        "cmps": components,
    }


def legacy_payload(object_id, unique_id=None, device_id=DEVICE_ID, **extra):
    payload = component(object_id, unique_id, **extra)
    payload["dev"] = {"ids": [device_id], "name": "ArduinoHA contract device"}
    payload.pop("p")
    return payload


def find_unique(entries, unique_id):
    matches = [entry for entry in entries if entry.get("unique_id") == unique_id]
    if len(matches) > 1:
        fail(f"duplicate entity-registry entries for {unique_id}: {matches}")
    return matches[0] if matches else None


def wait_for_entry(ws, unique_id):
    return wait_until(
        f"entity registry entry {unique_id}",
        lambda: find_unique(ws.registry_entries(), unique_id),
        timeout=60,
    )


def migration_contract():
    publisher = SharedRetainedPublisher(MQTT_HOST, MQTT_PORT)
    ws = HomeAssistantClient.bootstrap(
        HA_URL,
        STATE,
        owner_name="ArduinoHA Contract Owner",
        username="arduinoha-contract",
        password="arduinoha-contract-password",
    )
    ws.configure_mqtt(MQTT_HOST, MQTT_PORT)
    try:
        # Existing single-component entity and a user-owned registry customization.
        unique = f"{DEVICE_ID}_temperature"
        publisher.publish(legacy_topic("temperature"), json.dumps(legacy_payload("temperature", unique)))
        publisher.publish("contract/temperature/state", "21.5")
        original = wait_for_entry(ws, unique)
        original_id = original["id"]
        renamed_entity_id = "sensor.contract_temperature_user_name"
        ws.call(
            "config/entity_registry/update",
            entity_id=original["entity_id"],
            new_entity_id=renamed_entity_id,
            disabled_by="user",
        )

        # HA's required sequence: marker, device payload, then retained cleanup.
        publisher.publish(legacy_topic("temperature"), '{"migrate_discovery":true}')
        publisher.publish(device_topic(), json.dumps(device_payload({"temperature": component("temperature", unique)})))
        publisher.publish(legacy_topic("temperature"), "")

        migrated = wait_for_entry(ws, unique)
        if migrated["id"] != original_id:
            fail("single-to-device migration changed the entity registry ID")
        if migrated["entity_id"] != renamed_entity_id or migrated.get("disabled_by") != "user":
            fail(f"migration did not preserve user registry settings: {migrated}")
        if find_unique(ws.registry_entries(), unique) is None:
            fail("migrated entity disappeared from the entity registry")

        # Reverse migration is also ordered: marker the device topic, restore
        # legacy discovery, then clear the device topic. This is the protocol
        # HA documents for preserving the existing registry entry.
        publisher.publish(device_topic(), '{"migrate_discovery":true}')
        publisher.publish(legacy_topic("temperature"), json.dumps(legacy_payload("temperature", unique)))
        publisher.publish(device_topic(), "")
        rolled_back = wait_for_entry(ws, unique)
        if rolled_back["id"] != original_id:
            fail("device-to-single rollback changed the entity registry ID")
        if rolled_back["entity_id"] != renamed_entity_id or rolled_back.get("disabled_by") != "user":
            fail(f"rollback did not preserve user registry settings: {rolled_back}")

        # Restore device discovery so the retained-restart check continues to
        # exercise the forward migration form used by deployed firmware.
        publisher.publish(legacy_topic("temperature"), '{"migrate_discovery":true}')
        publisher.publish(device_topic(), json.dumps(device_payload({"temperature": component("temperature", unique)})))
        publisher.publish(legacy_topic("temperature"), "")
        remigrated = wait_for_entry(ws, unique)
        if remigrated["id"] != original_id:
            fail("repeat single-to-device migration changed the entity registry ID")

        retained = json.loads(publisher.retained_payload(device_topic()))
        if "temperature" not in retained.get("cmps", {}):
            fail("migration did not retain the primary device discovery payload")

        # Direct publication is deliberately not a migration protocol. It must not
        # create a second registry entry for the same stable unique ID. Keep it
        # on a separate device topic so it cannot invalidate the primary
        # retained-payload/restart contract.
        direct_unique = f"{EDGE_DEVICE_ID}_direct"
        publisher.publish(
            legacy_topic("direct", EDGE_DEVICE_ID),
            json.dumps(legacy_payload("direct", direct_unique, device_id=EDGE_DEVICE_ID)),
        )
        direct_entry = wait_for_entry(ws, direct_unique)
        publisher.publish(
            device_topic(EDGE_DEVICE_ID),
            json.dumps(device_payload(
                {"direct": component("direct", direct_unique)}, device_id=EDGE_DEVICE_ID
            )),
        )
        time.sleep(2)
        entries = [entry for entry in ws.registry_entries() if entry.get("unique_id") == direct_unique]
        if len(entries) != 1 or entries[0]["id"] != direct_entry["id"]:
            fail("direct device discovery publish created a duplicate registry entity")

        # Device-mode removal is two root updates: platform tombstone then
        # omission. This uses the edge fixture so retained-restart behavior is
        # independently asserted on the migrated primary fixture.
        removable_unique = f"{EDGE_DEVICE_ID}_removable"
        publisher.publish(
            device_topic(EDGE_DEVICE_ID),
            json.dumps(device_payload({
                "anchor": component("anchor", device_id=EDGE_DEVICE_ID),
                "removable": component("removable", removable_unique),
            }, device_id=EDGE_DEVICE_ID)),
        )
        wait_for_entry(ws, removable_unique)
        publisher.publish(
            device_topic(EDGE_DEVICE_ID),
            json.dumps(device_payload({
                "anchor": component("anchor", device_id=EDGE_DEVICE_ID),
                "removable": {"p": "sensor"},
            }, device_id=EDGE_DEVICE_ID)),
        )
        publisher.publish(
            device_topic(EDGE_DEVICE_ID),
            json.dumps(device_payload({"anchor": component("anchor", device_id=EDGE_DEVICE_ID)}, device_id=EDGE_DEVICE_ID)),
        )

        # Older Home Assistant versions do not clean initially disabled device
        # components. Keep that capability assertion in the development lane,
        # where it detects regressions without making the supported baseline
        # falsely fail.
        if EXPECT_DISABLED_CLEANUP:
            disabled_unique = f"{EDGE_DEVICE_ID}_disabled"
            publisher.publish(
                device_topic(EDGE_DEVICE_ID),
                json.dumps(device_payload({
                    "anchor": component("anchor", device_id=EDGE_DEVICE_ID),
                    "disabled": component(
                        "disabled", disabled_unique, enabled_by_default=False
                    ),
                }, device_id=EDGE_DEVICE_ID)),
            )
            disabled_entry = wait_for_entry(ws, disabled_unique)
            if disabled_entry.get("disabled_by") != "integration":
                fail(f"expected initially disabled entity to be integration-disabled: {disabled_entry}")
            publisher.publish(
                device_topic(EDGE_DEVICE_ID),
                json.dumps(device_payload({
                    "anchor": component("anchor", device_id=EDGE_DEVICE_ID),
                    "disabled": {"p": "sensor"},
                }, device_id=EDGE_DEVICE_ID)),
            )
            publisher.publish(
                device_topic(EDGE_DEVICE_ID),
                json.dumps(device_payload({"anchor": component("anchor", device_id=EDGE_DEVICE_ID)}, device_id=EDGE_DEVICE_ID)),
            )
            wait_until(
                "disabled device component cleanup",
                lambda: find_unique(ws.registry_entries(), disabled_unique) is None,
                timeout=60,
            )

        # Invalid retained discovery JSON never creates a registry entry. Escaped
        # strings are covered by the firmware-native serializer tests.
        publisher.publish(legacy_topic("malformed"), "{not-json")
        time.sleep(2)
        if find_unique(ws.registry_entries(), f"{DEVICE_ID}_malformed"):
            fail("malformed discovery payload created an entity")

        # This fixture documents the current HA field contract: def_ent_id is
        # allowed on first creation; obsolete obj_id is intentionally absent.
        default_payload = legacy_payload("default_name", def_ent_id="contract_default_name")
        if "obj_id" in default_payload:
            fail("contract fixture accidentally contains obsolete obj_id")
        publisher.publish(legacy_topic("default_name"), json.dumps(default_payload))
        wait_for_entry(ws, f"{DEVICE_ID}_default_name")

        STATE.mkdir(parents=True, exist_ok=True)
        (STATE / "migration-complete").write_text("ok", encoding="utf-8")
    finally:
        ws.close()
        publisher.close()


def retained_restart_contract():
    if not (STATE / "migration-complete").exists():
        fail("retained-restart mode requires the migration contract to run first")
    publisher = SharedRetainedPublisher(MQTT_HOST, MQTT_PORT)
    ws = HomeAssistantClient.from_state(HA_URL, STATE)
    try:
        migrated = wait_for_entry(ws, f"{DEVICE_ID}_temperature")
        if migrated["entity_id"] != "sensor.contract_temperature_user_name":
            fail("HA restart lost the user-owned entity rename")
        retained = json.loads(publisher.retained_payload(device_topic()))
        if "cmps" not in retained:
            fail("broker restart check did not receive retained device discovery")
    finally:
        ws.close()
        publisher.close()


def main():
    HomeAssistantClient.wait_until_ready(HA_URL)
    if MODE == "migration":
        migration_contract()
    elif MODE == "retained-restart":
        retained_restart_contract()
    else:
        fail(f"unknown CONTRACT_MODE: {MODE}")
    print(f"Home Assistant MQTT contract mode {MODE} passed")


if __name__ == "__main__":
    main()
