# Build Guide

Run `./sync_deps.sh` before the first build (or when `third_party` is missing).

## Build Commands

From the **repository root** (each block includes `cd` where needed).

**iOS** (run `xcodebuild` outside the sandbox; see [iOS Build Notes](#ios-build-notes)):

```bash
./ios/gen_ios
xcodebuild -workspace ios/SeatCanvas.xcworkspace \
  -scheme SeatCanvasSample \
  -configuration Release \
  -sdk iphoneos \
  -arch arm64 \
  CODE_SIGN_IDENTITY="" \
  CODE_SIGNING_REQUIRED=NO
```

From the **repository root** (each block includes `cd` where needed).

**macOS** (run `xcodebuild` outside the sandbox; see [macOS Build Notes](#macos-build-notes)):

```bash
./mac/gen_mac
xcodebuild -workspace mac/SeatCanvas.xcworkspace \
  -scheme SeatCanvas \
  -configuration Release \
  -sdk macosx \
  -arch arm64 \
  CODE_SIGN_IDENTITY="" \
  CODE_SIGNING_REQUIRED=NO
```

**Android** (JDK 17+ required; see [Android Build Notes](#android-build-notes)):

```bash
cd android/SeatCanvasSample
./gradlew assembleRelease -Parm64-only --no-daemon
```

**OHOS** (see [OHOS Build Notes](#ohos-build-notes)):

```bash
cd ohos
export DEVECO_SDK_HOME=/Applications/DevEco-Studio.app/Contents/sdk
$DEVECO_SDK_HOME/../tools/hvigor/bin/hvigorw assembleHar \
  --mode module \
  -p module=libseatcanvas@default \
  -p buildMode=release \
  -p product=default \
  --no-daemon
```

**Unit tests** (macOS only; see [Unit Test Notes](#unit-test-notes)):

```bash
./autotest.sh
```

**Web** (requires Emscripten; see [Web Build Notes](#web-build-notes)):

```bash
cd web
npm install
npm run build:wasm        # C++ → WASM
npm run build:wasm:debug  # debug build (uses build_debug/)
npm run build:lib         # JS library + types
npm run build:demo        # demo page
npm run build:deploy      # static site for Cloudflare Pages
npm run build             # full build
```

### Variants

| Platform | Command | Notes |
|----------|---------|-------|
| Unit tests | `./autotest.sh` | macOS only; do **not** pass `clean` unless necessary (see [Unit Test Notes](#unit-test-notes)) |
| iOS (CMake flags) | `./ios/gen_ios -D<FLAG>=ON` then `xcodebuild` as above | e.g. `-DENABLE_TIME_PROFILER=ON` |
| Android (all arch) | `cd android/SeatCanvasSample && ./gradlew assembleRelease` | Slower; all architectures |
| Web (debug) | `cd web && npm run build:wasm:debug` | Debug WASM with `-sSAFE_HEAP=1`; uses `build_debug/` |
| Web (deploy) | `cd web && npm run build:deploy` | Produces `web/deploy/` for static hosting |

## Dependency Management

First-time setup:

```bash
./sync_deps.sh
```

Or manually:

```bash
brew install 0x1306a94/tap/depctl
depctl --skip-paths third_party/tgfx/third_party/shaderc
```

### OHOS Dependencies

```bash
cd ohos
ohpm config set registry https://ohpm.openharmony.cn/ohpm/
ohpm install --all
```

## Code Formatting

Format all C++ and Swift code:

```bash
./codeformat.sh
```

Uses clang-format 14.x for C++ and swiftformat for Swift. A pre-commit hook automatically formats code before commits.

## Dependencies

- **tgfx**: graphics rendering library, SVG parsing, layer support
- **json**: nlohmann/json for style config parsing

**Build system**: CMake, C++17, supports iOS/Android/OHOS/Web, enables tgfx SVG and Layers support.

## iOS Build Notes

- `./ios/gen_ios` only generates the Xcode project — it is **not** a full build. Always follow with `xcodebuild` from the repo root (see [Build Commands](#build-commands)).
- Default local build uses **device** (`-sdk iphoneos`). Do **not** substitute `./ios/gen_simulator` unless explicitly requested.
- **Agent: run `xcodebuild` outside the sandbox.** Do not invoke `xcodebuild` inside a sandboxed shell. The sandbox blocks access to CoreSimulator, temp directories, and XPC services that Xcode requires. Request full (non-sandbox) permissions when running iOS build commands.
- Typical sandbox failures:
  - `Operation not permitted`
  - `couldn't create cache file`
  - `Connection invalid`
  - CoreSimulator connection errors

## Unit Test Notes

- **Agent: run unit tests via `./autotest.sh` only.** Do not hand-roll `cmake` / `ninja` / run `SeatCanvasUnitTests` directly unless the user explicitly asks. The script configures (`Ninja` + `MAC_ARM64`), builds `SeatCanvasUnitTests`, and runs the binary from `build_test/`.
- **Do not pass `clean` unless necessary.** Default `./autotest.sh` is enough for normal verification. Use `./autotest.sh clean` only when the build tree is corrupted, CMake options changed incompatibly, or a full rebuild is required — `clean` deletes the entire `build_test/` directory and forces a full recompile.
- Unit tests are enabled only on macOS (`SEATCANVAS_BUILD_TESTS=ON`). iOS / Android / OHOS / Web builds do not include this target.
- Prerequisites: same as macOS builds (Xcode toolchain, `./sync_deps.sh` on first run).
- Screenshot baseline tests are part of `SeatCanvasFullTests`. See [Baseline Screenshot Tests](#baseline-screenshot-tests).

## Baseline Screenshot Tests

Seat map rendering tests compare GPU snapshots against a **two-layer** baseline (same model as tgfx):

| Layer | Path | In git? | Purpose |
|-------|------|---------|---------|
| Version marker | `tests/baseline/version.json` | Yes | Git short hash per test key; marks which baseline generation is current |
| Pixel MD5 cache | `tests/baseline/.cache/metal/md5.json` | No (`.gitignore`) | Actual pixel MD5 for the local Metal GPU |

Local M1 and CI Paravirtual GPU produce different pixels. Each environment keeps its own MD5 cache; only `version.json` is committed.

### Daily development

For normal code changes, run:

```bash
./autotest.sh
```

`autotest.sh` automatically:

1. Builds `SeatCanvasFullTests` and `SeatCanvasUpdateBaseline`
2. Refreshes `tests/baseline/.cache/metal/` when `version.json` changed or cache is missing (`SeatCanvasUpdateBaseline`)
3. Runs all tests and compares against the local cache

No manual cache steps are needed for day-to-day work.

### After pulling baseline changes

When someone else updated `tests/baseline/version.json`, your local cache is stale. Either:

```bash
./autotest.sh
```

(cache refresh is automatic), or run the full workflow with develop verification:

```bash
./update_baseline.sh
```

`update_baseline.sh` syncs the MD5 cache and, on feature branches, verifies `develop` first when `version.json` differs.

### Accepting intentional rendering changes

When you **intentionally** change rendering and need a new baseline:

```bash
# 1. Run tests; failures write diffs to tests/out/*.webp
./autotest.sh

# 2. Review tests/out/, then accept
./accept_baseline.sh

# 3. Commit only version.json (not .cache/)
git add tests/baseline/version.json
git commit -m "Update baseline."
```

`accept_baseline.sh` runs tests, copies `tests/out/version.json` → `tests/baseline/version.json`, then runs `SeatCanvasUpdateBaseline` to refresh the local MD5 cache.

### Commands summary

| Scenario | Command |
|----------|---------|
| Daily verification | `./autotest.sh` |
| Full rebuild | `./autotest.sh clean` |
| Pull baseline + optional develop check | `./update_baseline.sh` |
| Accept new screenshots | `./accept_baseline.sh` |

### CI

- Autotest runs on macOS with Metal (Paravirtual on `macos-26`).
- `tests/baseline/.cache/metal` is restored/saved via a **separate** GHA cache from `third_party` (key: `autotest-baseline-metal-${{ hashFiles('tests/baseline/version.json') }}`).
- On cache miss, `autotest.sh` seeds the CI MD5 cache via `SeatCanvasUpdateBaseline` before compare.
- Failed tests upload `tests/out/` as an artifact.

### Troubleshooting

**Baseline tests fail after pull, but rendering looks correct**
→ Run `./autotest.sh` once to refresh local cache; or `./update_baseline.sh`.

**Baseline tests fail and `tests/out/*.webp` shows a real regression**
→ Fix rendering; do not run `accept_baseline.sh`.

**Cache seems corrupted**
→ `rm -rf tests/baseline/.cache/metal` then `./autotest.sh`.

## macOS Build Notes

- `./mac/gen_mac` only generates the Xcode project — it is **not** a full build. Always follow with `xcodebuild` from the repo root (see [Build Commands](#build-commands)).
- Default local build uses **device** (`-sdk macosx`).
- **Agent: run `xcodebuild` outside the sandbox.** Do not invoke `xcodebuild` inside a sandboxed shell. The sandbox blocks access to CoreSimulator, temp directories, and XPC services that Xcode requires. Request full (non-sandbox) permissions when running macOS build commands.
- Typical sandbox failures:
  - `Operation not permitted`
  - `couldn't create cache file`
  - `Connection invalid`
  - CoreSimulator connection errors

## Android Build Notes

- Requires **JDK 17+** (AGP 8.x / Gradle 8.x). Verify before building:

```bash
java -version   # must report 17 or higher
```

- **jenv** (preferred when available):

```bash
jenv versions                              # list installed JDKs; use the full name shown
jenv shell 17.0                            # example: match an entry from jenv versions
# or, from android/SeatCanvasSample:
jenv local 17.0                            # persist for this directory
```

- **Without jenv** — set `JAVA_HOME` explicitly:

```bash
# macOS: use built-in java_home helper
export JAVA_HOME=$(/usr/libexec/java_home -v 17)

# macOS (Homebrew), if openjdk@17 is installed:
# Apple Silicon:
export JAVA_HOME="/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home"
# Intel:
# export JAVA_HOME="/usr/local/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home"
# install if missing: brew install openjdk@17
```

- Default local build uses **arm64-only** with `--no-daemon` for faster iteration (see [Build Commands](#build-commands)).

## OHOS Build Notes

- Requires **DevEco Studio** with HarmonyOS SDK (no separate JDK setup — DevEco/hvigor handles the toolchain).
- Set `DEVECO_SDK_HOME` and use the bundled `hvigorw` (see [Build Commands](#build-commands)).
- Install dependencies first (see [OHOS Dependencies](#ohos-dependencies)).
- **Stale C++ native cache** — `ohos/.cxx` holds CMake output for the C++ core (`libseatcanvas` native code). If native build fails unexpectedly, clean and rebuild:

```bash
rm -rf ohos/.cxx
# then re-run the OHOS command in Build Commands
```

## Troubleshooting

**Build fails with missing dependencies**
→ Run `./sync_deps.sh`

**Code formatting hook fails**
→ `brew install clang-format@14`

**iOS build fails with signing error**
→ Add `CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO` to xcodebuild command

**iOS build fails with sandbox / CoreSimulator errors**
→ Run `xcodebuild` outside the sandbox (see [iOS Build Notes](#ios-build-notes))

**Android build fails with unsupported Java / Gradle JVM errors**
→ Use JDK 17+ and confirm `java -version` (see [Android Build Notes](#android-build-notes))

**OHOS dependencies fail to install**
→ `ohpm config set registry https://ohpm.openharmony.cn/ohpm/`

**OHOS C++ native build fails or behaves inconsistently**
→ `rm -rf ohos/.cxx` (C++ CMake cache) then rebuild (see [OHOS Build Notes](#ohos-build-notes))

**Unit tests fail to configure or link after CMake changes**
→ Retry with `./autotest.sh clean` once; otherwise see [Unit Test Notes](#unit-test-notes)

**Baseline screenshot tests fail**
→ See [Baseline Screenshot Tests](#baseline-screenshot-tests)

## Web Build Notes

- Requires **Emscripten** via `third_party/emsdk` (`web/script/setup.emsdk.js`: `emsdk install latest` / `activate latest`). Run `./sync_deps.sh` first, then `npm run build:wasm` — `web/script/cmake.js` runs setup automatically. Optional one-off setup: `cd web && npm run setup:emsdk`. Do not install Emscripten via Homebrew.

- Debug build uses `build_debug/` directory; release uses `build/`. The two directories are independent — switching modes does not require a clean rebuild.
- **`cmake.js` artifact flow**:
  1. Configures and builds via `emcmake cmake` + `cmake --build`
  2. Copies `.wasm` and `.js` glue files to `lib/wasm/`, `src/wasm/`, and `demo/wasm/`
- **Library build** (`build:lib`): bundles TypeScript into ESM+CJS, generates `.d.ts` type declarations. WASM import (`./wasm/libseatcanvas`) is externalized — consumers must provide the WASM files or use `locateFile`.
- **Demo** (`build:demo`): bundles `demo/index.ts` to `demo/index.js`. The demo imports wasm from `./wasm/libseatcanvas.js` (relative to the demo page).
- **Deploy** (`build:deploy`): assembles a self-contained `deploy/` directory with HTML, bundled JS, WASM, sample resources, and a `_headers` file for Cloudflare Pages (COOP/COEP for SharedArrayBuffer).
- **CI / clean builds**: GitHub Actions `web` job runs `npm run build:wasm` on `ubuntu-latest` (`depctl` + `third_party/emsdk` via `setup.emsdk.js`, same as local). Full pipeline locally: `npm run build` (wasm → lib → demo).
- Resource files (`sample_resources/`) are copied from the project root's `resources/SeatCanvasSample.bundle` via `copy-resources.js`, which also auto-generates `basemaps.json`.

## Troubleshooting
