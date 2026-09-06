"""Small artifact writers which keep container tests independent of host tools."""

from __future__ import annotations

import json
import pathlib
from collections.abc import Iterable
from typing import Any


def _path(root: str | pathlib.Path, name: str) -> pathlib.Path:
    path = pathlib.Path(root) / name
    path.parent.mkdir(parents=True, exist_ok=True)
    return path


def write_json_artifact(root: str | pathlib.Path, name: str, value: Any) -> pathlib.Path:
    """Write a deterministic UTF-8 JSON artifact and return its path."""
    path = _path(root, name)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def write_json_lines(root: str | pathlib.Path, name: str, values: Iterable[Any]) -> pathlib.Path:
    """Write a deterministic JSONL artifact and return its path."""
    path = _path(root, name)
    with path.open("w", encoding="utf-8") as output:
        for value in values:
            output.write(json.dumps(value, sort_keys=True) + "\n")
    return path
