"""Bounded wait helpers shared by integration contracts."""

from __future__ import annotations

import time
from collections.abc import Callable
from typing import TypeVar


class ContractError(RuntimeError):
    """An external Home Assistant or MQTT contract could not be satisfied."""


T = TypeVar("T")


def wait_until(
    description: str,
    predicate: Callable[[], T | None | bool],
    *,
    timeout: float = 90,
    interval: float = 1,
) -> T:
    """Return the first truthy predicate result or raise a useful timeout."""
    deadline = time.monotonic() + timeout
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        try:
            result = predicate()
            if result:
                return result  # type: ignore[return-value]
        except Exception as error:  # Services are expected to be starting.
            last_error = error
        time.sleep(interval)
    suffix = f" (last error: {last_error})" if last_error else ""
    raise ContractError(f"timed out waiting for {description}{suffix}")
