# Compatibility baseline

This maintenance line begins from fork commit `84cc0037b1c0` (release `v3.0.2`).
It intentionally tracks Home Assistant's MQTT discovery contract without merging
upstream development wholesale.

| Reference | Audited revision / target |
| --- | --- |
| Fork baseline | `84cc0037b1c0` (`v3.0.2`) |
| Upstream main | `1d333ab229b2` (`v2.1.0`) |
| Upstream develop | `a7039fad810b` (unreleased WIP 2.2.0) |
| Device discovery minimum | Home Assistant `2024.11.0` |
| Contract matrix | `2024.11.3`, current `stable`, current `dev` |

The only imported upstream code fix is the four missing-device-ID guards from
upstream commit `9c9d074`. Device discovery migration, JSON validation,
lifecycle handling, and tests are fork-native changes.

## Deliberate exclusions

- No merge or wholesale cherry-pick of upstream `develop` / WIP 2.2.0.
- No `obj_id` serialization: Home Assistant removed that discovery field. Use
  `setDefaultEntityId()` for new code.
- No binary-sensor `state_class`: it is not valid in the current HA MQTT binary
  sensor schema.
- No upstream IMqttClient abstraction or unreviewed entity-type feature PRs.

## Ongoing audit routine

The `upstream` remote has no usable push URL. Fetch and compare explicitly:

```bash
git fetch --prune upstream
git log --oneline main..upstream/develop
git diff --stat main...upstream/develop
```

Port only independently reviewed changes with regression tests; treat open
upstream pull requests as proposals, not release inputs. Run the native,
board-compile, and HA contract gates before publishing a new release.

