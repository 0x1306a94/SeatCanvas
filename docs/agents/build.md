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
