# HA/MQTT contract testkit

This small Python package supplies generic test transport primitives for a
disposable Home Assistant + MQTT contract environment:

- Home Assistant onboarding, MQTT config-entry setup, WebSocket registry access,
  REST state/service helpers, and readiness waiting;
- retained MQTT publishing and discovery observation; and
- JSON/JSONL artifact output plus bounded retry helpers.

It deliberately contains no ArduinoHA discovery fixture, DeviceFramework
import, entity expectation, or release policy. ArduinoHA owns its migration
fixture in `tests/ha-contract`; DeviceFramework owns its hardware fixture and
Docker adapter in its own repository. Other projects can reuse this package by
providing their own retained MQTT messages and assertions.

The package is copied into a test container as a local build context and is not
part of the Arduino firmware library export.
