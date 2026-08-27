#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 compile --platform esp8266|esp32" >&2
    exit 2
}

[[ "${1:-}" == "compile" && "${2:-}" == "--platform" && $# -eq 3 ]] || usage

case "${3:-}" in
    esp8266|esp32) environment="${3}" ;;
    *) usage ;;
esac

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
pio test -d "$root" -e "$environment" --without-uploading --without-testing
echo "ArduinoHA compile check passed for ${3}"
