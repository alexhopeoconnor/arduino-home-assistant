"""Generic Home Assistant MQTT contract-test support.

This package deliberately knows nothing about ArduinoHA, DeviceFramework, or
any particular discovery schema.  Consumers own their fixtures and assertions;
the package owns only HA/MQTT transport, readiness, and artifact primitives.
"""

from .artifacts import write_json_artifact, write_json_lines
from .home_assistant import HomeAssistantClient
from .mqtt import MqttObserver, RetainedPublisher
from .wait import ContractError, wait_until

__all__ = [
    "ContractError",
    "HomeAssistantClient",
    "MqttObserver",
    "RetainedPublisher",
    "wait_until",
    "write_json_artifact",
    "write_json_lines",
]
