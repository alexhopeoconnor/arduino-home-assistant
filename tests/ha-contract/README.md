# Home Assistant MQTT contract tests

This harness runs retained MQTT discovery messages through Mosquitto and a real
Home Assistant container. It verifies the behavior that firmware unit tests
cannot: Home Assistant's entity-registry identity, user-owned registry changes,
and retained discovery after a Home Assistant restart.

It has two ordered modes:

1. `migration` publishes single-component discovery, applies a user rename and
   disable, then sends `migrate_discovery` markers, a device bundle, and legacy
   retained-topic cleanup.
2. `retained-restart` runs after Home Assistant restarts and verifies the
   retained device bundle plus preserved registry customization.

The migration case also rejects malformed retained JSON, checks that a direct
single-to-device publication does not duplicate registry identity, exercises the
device-component tombstone/omission sequence, and makes the current
`def_ent_id`/no-`obj_id` schema expectation explicit.

## Run locally

From the repository root:

```bash
HA_VERSION=2024.11.3 docker compose -f tests/ha-contract/compose.yaml up -d mqtt homeassistant
HA_VERSION=2024.11.3 docker compose -f tests/ha-contract/compose.yaml run --rm tests
HA_VERSION=2024.11.3 docker compose -f tests/ha-contract/compose.yaml restart homeassistant
CONTRACT_MODE=retained-restart HA_VERSION=2024.11.3 docker compose -f tests/ha-contract/compose.yaml run --rm tests
HA_VERSION=2024.11.3 docker compose -f tests/ha-contract/compose.yaml down -v
```

Use `HA_VERSION=stable` and `HA_VERSION=dev` for the current supported and
development Home Assistant images. The test uses only ephemeral named volumes;
`down -v` removes its broker data, Home Assistant config, owner token, and
registry state.

## Scope

The firmware's native Unity suite covers JSON escaping, invalid topic tokens,
serializer preflight, migration ordering, component removal, and lifecycle
behavior. This container suite covers Home Assistant's persistence contract. It
does not need a physical board: fixture discovery documents mirror retained
payloads emitted by ArduinoHA and isolate Home Assistant/MQTT compatibility.

