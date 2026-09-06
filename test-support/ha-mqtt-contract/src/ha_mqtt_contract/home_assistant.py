"""Home Assistant onboarding, MQTT setup, registry, and service helpers."""

from __future__ import annotations

import json
import pathlib
import time
from typing import Any

import requests
import websocket

from .wait import ContractError, wait_until


class HomeAssistantClient:
    """Authenticated HA API client with a deliberately small contract surface."""

    def __init__(self, base_url: str, token: str):
        self.base_url = base_url.rstrip("/")
        self.token = token
        self._socket: websocket.WebSocket | None = None
        self._next_id = 1

    @property
    def _headers(self) -> dict[str, str]:
        return {"Authorization": f"Bearer {self.token}"}

    @classmethod
    def bootstrap(
        cls,
        base_url: str,
        state_dir: str | pathlib.Path,
        *,
        owner_name: str = "HA MQTT Contract Owner",
        username: str = "ha-mqtt-contract",
        password: str = "ha-mqtt-contract-password",
    ) -> "HomeAssistantClient":
        """Create or load an isolated owner token for a disposable HA volume."""
        base_url = base_url.rstrip("/")
        state = pathlib.Path(state_dir)
        token_file = state / "ha-token"
        cls.wait_until_ready(base_url)
        if token_file.exists():
            return cls(base_url, token_file.read_text(encoding="utf-8").strip())

        client_id = "http://ha-mqtt-contract.local/"
        user = {
            "client_id": client_id,
            "name": owner_name,
            "username": username,
            "password": password,
            "language": "en",
        }
        created = cls._response_json(
            requests.post(f"{base_url}/api/onboarding/users", json=user, timeout=10),
            "Home Assistant onboarding user creation",
        )
        auth_code = created.get("auth_code")
        if not auth_code:
            raise ContractError("Home Assistant onboarding did not return an auth_code")
        token_response = cls._response_json(
            requests.post(
                f"{base_url}/auth/token",
                data={"client_id": client_id, "grant_type": "authorization_code", "code": auth_code},
                timeout=10,
            ),
            "Home Assistant token exchange",
        )
        token = token_response.get("access_token")
        if not token:
            raise ContractError("Home Assistant token exchange did not return an access_token")

        client = cls(base_url, token)
        for path, payload in (
            ("/api/onboarding/core_config", {}),
            ("/api/onboarding/analytics", {"preferences": {}}),
        ):
            response = requests.post(f"{base_url}{path}", headers=client._headers, json=payload, timeout=10)
            if response.status_code not in (200, 201, 400, 404):
                raise ContractError(
                    f"Home Assistant onboarding step {path} failed: {response.status_code} {response.text}"
                )
        state.mkdir(parents=True, exist_ok=True)
        token_file.write_text(token, encoding="utf-8")
        return client

    @classmethod
    def from_state(cls, base_url: str, state_dir: str | pathlib.Path) -> "HomeAssistantClient":
        token_file = pathlib.Path(state_dir) / "ha-token"
        if not token_file.exists():
            raise ContractError("Home Assistant token has not been bootstrapped")
        return cls(base_url, token_file.read_text(encoding="utf-8").strip())

    @staticmethod
    def _response_json(response: requests.Response, context: str) -> dict[str, Any]:
        if not response.ok:
            raise ContractError(f"{context} failed ({response.status_code}): {response.text}")
        try:
            value = response.json()
        except ValueError as error:
            raise ContractError(f"{context} returned invalid JSON: {error}") from error
        if not isinstance(value, dict):
            raise ContractError(f"{context} returned unexpected JSON: {value}")
        return value

    @classmethod
    def wait_until_ready(cls, base_url: str, *, timeout: float = 180) -> None:
        def ready() -> bool:
            response = requests.get(f"{base_url.rstrip('/')}/api/", timeout=5)
            return response.status_code in (200, 401)

        wait_until("Home Assistant HTTP API", ready, timeout=timeout)

    def configure_mqtt(self, host: str, port: int) -> None:
        """Create HA's MQTT config entry, negotiating current/older form schemas."""
        flow = self._response_json(
            requests.post(
                f"{self.base_url}/api/config/config_entries/flow",
                headers=self._headers,
                json={"handler": "mqtt"},
                timeout=15,
            ),
            "MQTT config-entry flow creation",
        )
        if flow.get("type") == "create_entry":
            return
        if flow.get("type") != "form" or not flow.get("flow_id"):
            raise ContractError(f"unexpected MQTT config-entry flow result: {flow}")
        user_input: dict[str, Any] = {"broker": host, "port": port}
        schema_names = {
            field.get("name") for field in flow.get("data_schema", []) if isinstance(field, dict)
        }
        if "other_settings" in schema_names:
            user_input["other_settings"] = {
                "set_client_cert": False,
                "set_ca_cert": "off",
                "transport": "tcp",
            }
        configured = self._response_json(
            requests.post(
                f"{self.base_url}/api/config/config_entries/flow/{flow['flow_id']}",
                headers=self._headers,
                json=user_input,
                timeout=15,
            ),
            "MQTT config-entry flow configuration",
        )
        if configured.get("type") != "create_entry":
            raise ContractError(f"MQTT config-entry flow did not create an entry: {configured}")
        time.sleep(3)

    def _ensure_socket(self) -> websocket.WebSocket:
        if self._socket is not None:
            return self._socket
        scheme = "wss" if self.base_url.startswith("https://") else "ws"
        address = self.base_url.split("://", 1)[1]
        socket = websocket.create_connection(f"{scheme}://{address}/api/websocket", timeout=15)
        required = json.loads(socket.recv())
        if required.get("type") != "auth_required":
            socket.close()
            raise ContractError(f"unexpected Home Assistant WebSocket greeting: {required}")
        socket.send(json.dumps({"type": "auth", "access_token": self.token}))
        authenticated = json.loads(socket.recv())
        if authenticated.get("type") != "auth_ok":
            socket.close()
            raise ContractError(f"Home Assistant WebSocket authentication failed: {authenticated}")
        self._socket = socket
        return socket

    def close(self) -> None:
        if self._socket is not None:
            self._socket.close()
            self._socket = None

    def call(self, message_type: str, **kwargs: Any) -> Any:
        socket = self._ensure_socket()
        message_id = self._next_id
        self._next_id += 1
        socket.send(json.dumps({"id": message_id, "type": message_type, **kwargs}))
        while True:
            result = json.loads(socket.recv())
            if result.get("id") != message_id:
                continue
            if not result.get("success"):
                raise ContractError(f"WebSocket {message_type} failed: {result}")
            return result.get("result")

    def entity_registry(self) -> list[dict[str, Any]]:
        result = self.call("config/entity_registry/list")
        if not isinstance(result, list):
            raise ContractError(f"entity registry returned unexpected value: {result}")
        return result

    # A concise compatibility spelling for contract suites; this remains schema-neutral.
    def registry_entries(self) -> list[dict[str, Any]]:
        return self.entity_registry()

    def device_registry(self) -> list[dict[str, Any]]:
        result = self.call("config/device_registry/list")
        if not isinstance(result, list):
            raise ContractError(f"device registry returned unexpected value: {result}")
        return result

    def entities_for_device(self, device_id: str) -> list[dict[str, Any]]:
        return [entry for entry in self.entity_registry() if entry.get("device_id") == device_id]

    def wait_for_entity(self, unique_id: str, *, timeout: float = 60) -> dict[str, Any]:
        def find() -> dict[str, Any] | None:
            matches = [entry for entry in self.entity_registry() if entry.get("unique_id") == unique_id]
            if len(matches) > 1:
                raise ContractError(f"duplicate entity-registry entries for {unique_id}: {matches}")
            return matches[0] if matches else None

        return wait_until(f"entity registry entry {unique_id}", find, timeout=timeout)

    def state(self, entity_id: str) -> dict[str, Any] | None:
        response = requests.get(f"{self.base_url}/api/states/{entity_id}", headers=self._headers, timeout=10)
        if response.status_code == 404:
            return None
        return self._response_json(response, f"state lookup for {entity_id}")

    def wait_for_state(self, entity_id: str, state: str, *, timeout: float = 60) -> dict[str, Any]:
        return wait_until(
            f"Home Assistant state {entity_id}={state}",
            lambda: value if (value := self.state(entity_id)) and value.get("state") == state else None,
            timeout=timeout,
        )

    def call_service(self, domain: str, service: str, data: dict[str, Any]) -> list[dict[str, Any]]:
        response = requests.post(
            f"{self.base_url}/api/services/{domain}/{service}",
            headers=self._headers,
            json=data,
            timeout=15,
        )
        if not response.ok:
            raise ContractError(f"service {domain}.{service} failed ({response.status_code}): {response.text}")
        try:
            result = response.json()
        except ValueError as error:
            raise ContractError(f"service {domain}.{service} returned invalid JSON: {error}") from error
        if not isinstance(result, list):
            raise ContractError(f"service {domain}.{service} returned unexpected JSON: {result}")
        return result
