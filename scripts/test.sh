#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 compile|examples --platform esp8266|esp32" >&2
    exit 2
}

[[ $# -eq 3 && ( "${1:-}" == "compile" || "${1:-}" == "examples" ) && "${2:-}" == "--platform" ]] || usage
case "${3:-}" in
    esp8266|esp32) environment="${3}" ;;
    *) usage ;;
esac

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
case "$1" in
    compile)
        pio test -d "$root" -e "$environment" --without-uploading --without-testing
        echo "ArduinoHA compile check passed for $environment"
        ;;
    examples)
        mapfile -t examples < <(find "$root/examples" -mindepth 1 -maxdepth 1 -type d -name '[0-9][0-9]-*' -print | sort)
        if (( ${#examples[@]} == 0 )); then
            echo "No example projects found" >&2
            exit 1
        fi
        for example in "${examples[@]}"; do
            pio run -d "$example" -e "$environment" </dev/null
        done
        echo "ArduinoHA examples compile check passed for $environment"
        ;;
esac
