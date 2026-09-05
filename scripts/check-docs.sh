#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
required=(
    README.md CHANGELOG.md
    docs/README.md docs/getting-started.md docs/device-and-discovery.md docs/mqtt-usage.md docs/entities.md
    examples/README.md
)

for path in "${required[@]}"; do
    [[ -f "$root/$path" ]] || { echo "Missing required documentation: $path" >&2; exit 1; }
done

while IFS= read -r -d '' markdown; do
    while IFS= read -r target; do
        [[ -z "$target" || "$target" == \#* || "$target" == http://* || "$target" == https://* || "$target" == mailto:* ]] && continue
        target="${target%%#*}"
        case "$target" in
            /*) candidate="$root/${target#/}" ;;
            *) candidate="$(dirname "$markdown")/$target" ;;
        esac
        [[ -e "$candidate" ]] || { echo "Broken relative link in ${markdown#$root/}: $target" >&2; exit 1; }
    done < <(sed -nE 's/.*\]\(([^ )]+)( "[^"]*")?\).*/\1/p' "$markdown")
done < <(find "$root" -path "$root/.git" -prune -o -name '*.md' -type f -print0)

while IFS= read -r example; do
    for required in README.md platformio.ini; do
        [[ -f "$example/$required" ]] || { echo "Incomplete guided example: ${example#$root/} is missing $required" >&2; exit 1; }
    done
    find "$example" -maxdepth 2 -type f \( -name '*.ino' -o -name '*.cpp' \) -print -quit | grep -q . || {
        echo "Incomplete guided example: ${example#$root/} has no sketch source" >&2
        exit 1
    }
done < <(find "$root/examples" -mindepth 1 -maxdepth 1 -type d -name '[0-9][0-9]-*' -print | sort)

echo "Documentation links and required files passed"
