# Autotest Guide

`./autotest.sh` is the single entry point for macOS automated tests. It builds and runs `SeatCanvasFullTests`, which includes **two kinds of tests** in one binary:

| Kind | Examples | How results are checked |
|------|----------|-------------------------|
| Unit / logic tests | Parser, gesture, style, renderer state (`SeatCanvasCoreRenderer_test`, `BaseMapMeshBuilder_test`, …) | gtest assertions |
| Rendering snapshot tests | Seat map screenshots (`SeatCanvasTestFixture` in `SeatRender_test.cpp`, …) | GPU pixel MD5 vs local cache |

Daily development: run `./autotest.sh` — no need to distinguish the two kinds manually.

## Prerequisites

- macOS only (`SEATCANVAS_BUILD_TESTS=ON`)
- Xcode toolchain, `./sync_deps.sh` on first run
- See [Build Guide — Unit Test Notes](build.md#unit-test-notes) for agent/build constraints (`clean`, sandbox, etc.)

## Daily development

```bash
./autotest.sh
```

`autotest.sh` automatically:

1. Configures `build_test/` (Ninja + `MAC_ARM64`)
2. Builds `SeatCanvasFullTests` and `SeatCanvasUpdateBaseline`
3. Refreshes rendering snapshot cache when `version.json` changed or cache is missing
4. Runs all tests

Use `./autotest.sh clean` only when the build tree is corrupted or CMake options changed incompatibly.

## Rendering snapshot tests

A subset of `SeatCanvasFullTests` renders seat maps headlessly (Metal) and compares GPU snapshots. This uses a **two-layer** baseline (same model as tgfx):

| Layer | Path | In git? | Purpose |
|-------|------|---------|---------|
| Version marker | `tests/baseline/version.json` | Yes | Git short hash per test key; marks which baseline generation is current |
| Pixel MD5 cache | `tests/baseline/.cache/metal/md5.json` | No (`.gitignore`) | Actual pixel MD5 for the local Metal GPU |

Local M1 and CI Paravirtual GPU produce different pixels. Each environment keeps its own MD5 cache; only `version.json` is committed.

Related scripts:

| Script | Role |
|--------|------|
| `autotest.sh` | Build, auto-refresh cache if stale, run all tests |
| `update_baseline.sh` | Sync cache after pull; verify `develop` on feature branches |
| `accept_baseline.sh` | Accept new screenshots after intentional rendering changes |

CMake targets (`tests/CMakeLists.txt`):

| Target | Role |
|--------|------|
| `SeatCanvasFullTests` | Run unit + snapshot tests; snapshots compare against cache |
| `SeatCanvasUpdateBaseline` | Record snapshot pixel MD5 into `.cache/metal/` |

### After pulling snapshot baseline changes

When someone else updated `tests/baseline/version.json`, your local cache is stale. Either:

```bash
./autotest.sh
```

(cache refresh is automatic), or:

```bash
./update_baseline.sh
```

`update_baseline.sh` syncs the MD5 cache and, on feature branches, verifies `develop` first when `version.json` differs.

### Accepting intentional rendering changes

When you **intentionally** change rendering and need a new snapshot baseline:

```bash
# 1. Run tests; snapshot failures write diffs to tests/out/*.webp
./autotest.sh

# 2. Review tests/out/, then accept
./accept_baseline.sh

# 3. Commit only version.json (not .cache/)
git add tests/baseline/version.json
git commit -m "Update baseline."
```

`accept_baseline.sh` runs tests, copies `tests/out/version.json` → `tests/baseline/version.json`, then runs `SeatCanvasUpdateBaseline` to refresh the local MD5 cache.

## Commands summary

| Scenario | Command |
|----------|---------|
| Daily verification (unit + snapshots) | `./autotest.sh` |
| Full rebuild | `./autotest.sh clean` |
| Pull snapshot baseline + optional develop check | `./update_baseline.sh` |
| Accept new screenshots | `./accept_baseline.sh` |
| Coverage report | `./autotest.sh coverage` |

## CI

- Autotest job runs on macOS with Metal (Paravirtual on `macos-26`).
- `tests/baseline/.cache/metal` is restored/saved via a **separate** GHA cache from `third_party` (key: `autotest-baseline-metal-${{ hashFiles('tests/baseline/version.json') }}`).
- On cache miss, `autotest.sh` seeds the CI MD5 cache via `SeatCanvasUpdateBaseline` before compare.
- Failed snapshot tests upload `tests/out/` as an artifact.

## Troubleshooting

**Unit test assertion failure**
→ Fix the logic; no baseline workflow involved.

**Snapshot tests fail after pull, but rendering looks correct**
→ Run `./autotest.sh` once to refresh local cache; or `./update_baseline.sh`.

**Snapshot tests fail and `tests/out/*.webp` shows a real regression**
→ Fix rendering; do not run `accept_baseline.sh`.

**Snapshot cache seems corrupted**
→ `rm -rf tests/baseline/.cache/metal` then `./autotest.sh`.

**Tests fail to configure or link after CMake changes**
→ Retry with `./autotest.sh clean` once; see [Build Guide — Unit Test Notes](build.md#unit-test-notes).
