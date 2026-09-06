"""MQTT publishing and observation helpers for retained-discovery contracts."""

from __future__ import annotations

import threading
import time
import uuid
from collections.abc import Iterable
from typing import Any

import paho.mqtt.client as mqtt

from .wait import ContractError, wait_until


class _MqttClient:
    def __init__(self, host: str, port: int, *, client_prefix: str):
        self._connected = threading.Event()
        self._client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2,
            client_id=f"{client_prefix}-{uuid.uuid4()}",
        )
        self._client.on_connect = self._on_connect
        self._client.connect(host, port, keepalive=15)
        self._client.loop_start()
        if not self._connected.wait(timeout=15):
            self.close()
            raise ContractError(f"MQTT client {client_prefix} did not connect within 15 seconds")

    def _on_connect(self, _client: mqtt.Client, _userdata: Any, _flags: Any, reason_code: Any, _properties: Any) -> None:
        if reason_code == 0:
            self._connected.set()

    def close(self) -> None:
        self._client.loop_stop()
        self._client.disconnect()


class RetainedPublisher(_MqttClient):
    """A QoS-1 retained publisher used by schema/migration fixtures."""

    def __init__(self, host: str, port: int):
        super().__init__(host, port, client_prefix="contract-publisher")

    def publish(self, topic: str, payload: str, *, retain: bool = True) -> None:
        info = self._client.publish(topic, payload, qos=1, retain=retain)
        info.wait_for_publish(timeout=10)
        if not info.is_published():
            raise ContractError(f"MQTT publish timed out for {topic}")

    def retained_payload(self, topic: str, *, timeout: float = 30) -> str:
        received: list[str] = []

        def on_message(_client: mqtt.Client, _userdata: Any, message: mqtt.MQTTMessage) -> None:
            if message.topic == topic:
                received.append(message.payload.decode("utf-8"))

        self._client.on_message = on_message
        self._client.subscribe(topic, qos=1)
        try:
            return wait_until(
                f"retained MQTT message {topic}",
                lambda: received[0] if received else None,
                timeout=timeout,
            )
        finally:
            self._client.unsubscribe(topic)
            self._client.on_message = None


class MqttObserver(_MqttClient):
    """Observe MQTT traffic without imposing a discovery-schema interpretation."""

    def __init__(self, host: str, port: int, topics: Iterable[str] = ("homeassistant/#",)):
        self._messages: list[dict[str, Any]] = []
        self._lock = threading.Lock()
        self._subscription_ack = threading.Event()
        super().__init__(host, port, client_prefix="contract-observer")
        self._client.on_message = self._on_message
        self._client.on_subscribe = self._on_subscribe
        for topic in topics:
            result, _mid = self._client.subscribe(topic, qos=1)
            if result != mqtt.MQTT_ERR_SUCCESS:
                self.close()
                raise ContractError(f"unable to subscribe to MQTT topic {topic}")
            if not self._subscription_ack.wait(timeout=15):
                self.close()
                raise ContractError(f"MQTT subscription to {topic} was not acknowledged within 15 seconds")
            self._subscription_ack.clear()

    def _on_subscribe(
        self,
        _client: mqtt.Client,
        _userdata: Any,
        _mid: int,
        _reason_codes: Any,
        _properties: Any,
    ) -> None:
        self._subscription_ack.set()

    def _on_message(self, _client: mqtt.Client, _userdata: Any, message: mqtt.MQTTMessage) -> None:
        with self._lock:
            self._messages.append(
                {
                    "topic": message.topic,
                    "payload": message.payload.decode("utf-8", errors="replace"),
                    "qos": message.qos,
                    "retain": message.retain,
                    "received_at": time.time(),
                }
            )

    def messages(self) -> list[dict[str, Any]]:
        with self._lock:
            return list(self._messages)

    def wait_for_topic(self, topic: str, *, timeout: float = 60) -> dict[str, Any]:
        return wait_until(
            f"MQTT topic {topic}",
            lambda: next((entry for entry in reversed(self.messages()) if entry["topic"] == topic), None),
            timeout=timeout,
        )
