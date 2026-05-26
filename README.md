# SeatCanvas

SeatCanvas is a cross-platform seat map rendering library supporting iOS, Android, and OHOS (HarmonyOS) platforms. The library provides seat map rendering, interactive gesture handling, seat state management, and more.

## Core Features

- **Cross-Platform Support**: iOS, Android, OHOS, Web
- **High-Performance Rendering**: GPU-accelerated rendering based on the tgfx graphics library
- **SVG Basemap Support**: Supports venue basemaps in SVG format
- **Gesture Interaction**: Supports tap, pan, zoom gestures with iOS-style inertial scrolling and elastic bounce-back animations
- **Customizable Styles**: Supports circle and SVG seat style configurations with dynamic switching
- **Push-Based Seat State**: Apps push pricecode, availability status, and selection; C++ resolves style keys at render time (no per-frame delegate style lookup)

## Supported Platforms

- iOS 15.0+
- Android (via JNI)
- OHOS (HarmonyOS)
- Web (Emscripten + WebAssembly)

## Dependency Management

This project uses **tgfx's dependency management approach**, managing third-party dependencies through the `DEPS` file and the [depctl](https://github.com/0x1306a94/depctl) tool.

### Installing Dependencies

After cloning the project for the first time, sync dependencies:

```bash
./sync_deps.sh
```

Or manually:

```bash
# Install depctl tool
brew install 0x1306a94/tap/depctl

# Sync dependencies
depctl
```

### Dependency Details

Main dependencies:
- **tgfx**: Graphics rendering library (includes SVG parsing and layer support)
- **emsdk**: Emscripten SDK (Web/WASM builds; activated by `web/script/setup.emsdk.js` — `emsdk install latest` / `activate latest`)
- **json**: JSON parsing library (nlohmann/json, introduced via tgfx)

Dependency configuration is located in the `DEPS` file, using the same dependency management format as the tgfx project.

## Technical Implementation

### Gesture Inertia/Elastic Animation Algorithm

The gesture inertial scrolling and elastic bounce-back animation algorithms in this project are ported from the [fluid-scroll](https://github.com/ktiays/fluid-scroll) project. The algorithm implements scrolling effects similar to iOS `UIScrollView`, including:

- **Scroll Inertia**: Smooth inertial scrolling with natural deceleration
- **Edge Bounce-Back**: Elastic bounce-back animation at edges for fluid boundary feedback
- **Velocity Tracking**: Precise gesture velocity calculation supporting inertial scrolling after fast swipes

The algorithm is implemented in the C++ core layer, ensuring a consistent gesture experience across platforms.

## Build System

The project uses the CMake build system, supporting:
- iOS (Xcode)
- Android (Gradle + CMake)
- OHOS (DevEco Studio + CMake)
- Web (Emscripten + CMake)

### Build Requirements

- CMake 3.22 or higher
- C++17 standard
- Platform-specific build tools (Xcode, Android NDK, DevEco Studio)
- **Web**: Node.js, npm, Ninja; run `./sync_deps.sh` first — do not install Emscripten via Homebrew; `npm run build:wasm` uses `third_party/emsdk` automatically

### VS Code CMake Configuration Example

`settings.json` configuration example:

```json
{
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
        "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/cmake/ios.toolchain.cmake",
        "-DPLATFORM=OS64",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        "-DCMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS=iphoneos",
        "-DDEPLOYMENT_TARGET=15.0",
        "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${workspaceFolder}/build_vscode/Products",
        "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${workspaceFolder}/build_vscode/Products",
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${workspaceFolder}/build_vscode/Products",
    ],
    "cmake.generator": "Xcode"
}
```

## Project Structure

```
SeatCanvas/
├── src/SeatCanvas/          # C++ core code
│   ├── core/                # Core modules
│   │   ├── renderer/            # Renderer components
│   │   │   ├── pass/            # Custom render passes
│   │   ├── gesture/             # Gesture handling
│   │   ├── style/               # Style configuration
│   │   ├── layers/              # Layer management
│   │   ├── parser/              # Basemap parsers
│   │   ├── animation/           # Animation system
│   │   └── drawers/             # Drawers
│   └── platform/            # Platform-specific implementations
│       ├── ios/
│       ├── android/
│       ├── ohos/
│       └── web/
├── web/                     # Web platform (TypeScript + Emscripten)
├── ios/                     # iOS samples and resources
├── android/                 # Android samples and resources
├── ohos/                    # OHOS samples and resources
├── resources/               # Resource files
├── third_party/             # Third-party dependencies (synced via depsync)
└── CMakeLists.txt          # CMake build configuration
```

### SeatCanvasSample.bundle Directory Structure

The sample app uses a resource bundle `SeatCanvasSample.bundle` with the following directory structure:

```
SeatCanvasSample.bundle/
└── default/                  # Default resource scope
    ├── basemap/             # Basemap files directory
    │   ├── performbg.svg
    │   ├── performbg_2.svg
    │   ├── 73807.svg
    │   └── 73808.svg
    ├── seatstyle/           # Seat style SVG icons directory
    │   ├── icon_seat_selectable.svg      # Selectable seat icon
    │   ├── icon_seat_selected.svg        # Selected seat icon
    │   └── icon_seat_nonselectable.svg   # Non-selectable seat icon
    ├── zonedata/            # Zone data JSON files directory
    │   ├── performbg.json
    │   └── performbg_2.json
    └── seatdata/            # Seat data JSON files directory
        ├── performbg.json
        └── performbg_2.json
```

**Resource Scope Description:**
- `default/`: Default resource scope containing the sample app's base resources
- `customized/`: Custom resource scope (optional), for storing user-customized resource files. Must maintain the same structure as `default`. Used for quickly previewing your own venue rendering results

**File Naming Rules:**
- Basemap files: SVG format, filenames correspond to JSON filenames in `zonedata/` and `seatdata/` directories (without extension)
- Zone data: JSON format, filename matches the corresponding basemap filename (`.svg` replaced with `.json`)
- Seat data: JSON format, filename matches the corresponding basemap filename (`.svg` replaced with `.json`)

**JSON Data Formats:**

Zone data (`zonedata/*.json`) format:
```json
[
  {
    "zoneId": "37492",
    "x": 33,
    "y": 411,
    "w": 289,
    "h": 329,
    "alternateColor": "#DF0AEEFF"
  }
]
```

Seat data (`seatdata/*.json`) format (geometry + pricecode; status/selection are pushed at runtime):
```json
{
  "37492": [
    {
      "seatId": "seat_0_0",
      "pricecode": "1",
      "x": 33,
      "y": 411,
      "rotation": 0
    }
  ]
}
```

**Platform Resource Paths:**
- Shared `SeatCanvasSample.bundle` via symlinks

## Seat Data, Style Keys, and Renderer Delegate

- **String style keys**: Seat style JSON is a list of entries, each with a string `key` and a `config` (circle or SVG). Register keys with `SeatStyleConfigBuilder`; keys must match `SeatRenderStyleId.compose(pricecode, status, selected)`. Unknown or empty keys skip drawing that seat.
- **Push-based render state**: Styles are not resolved per frame via delegate. Call `setDelegate` **before** `loadBaseMap()`. Push data in `didLoadBaseMap`: `registerPricecodes` → `applySeatStyleJSONConfig` → per zone: `updateSeatDatas`/`updateSeats` (with `pricecode`) → `updateSeatStatusesForZone` → `setSelectedSeatIds`; use `updateSelectedSeatIds` on tap. Set `seatRenderZoomThreshold` in `didUpdateZoomLevelConfig`.
- **Typical load order**: `setDelegate` → `loadBaseMap` → (core fires `didLoadBaseMap` when ready) → push pricecodes, styles, seat geometry, statuses, selection → `didUpdateZoomLevelConfig` fires → set `seatRenderZoomThreshold`.
- **`didLoadBaseMap` timing**: Fired after the basemap is parsed. It is **not** replayed if the delegate is set after `loadBaseMap()`. `didUpdateZoomLevelConfig` fires immediately after `didLoadBaseMap` and on device rotation.
- **`SeatCanvasZoomLevelConfigEvent`**: Carries `zoomLevels` (seat/row/zone/venue), min/max/current zoom. Set `seatRenderZoomThreshold` in `didUpdateZoomLevelConfig` callback.
- **`updateSeats` / `updateSeatDatas` side effect**: Resets that zone's statuses to 0 and clears selected state for removed seats; you must re-push status/selected afterward.
- **Updating and clearing**: Call `clearSeatData` on the view or controller to remove all seat data and refresh rendering.
- **Circle style**: For builders that support it, `overlay` and `checkmark` are optional; only `fill` is required.
- **Delegate callbacks**: `didLoadBaseMap` / `didUnloadBaseMap` for basemap lifecycle; `didTapSeat` / `didTapZone` for interaction. Stop polling and clear cached seat state in `didUnloadBaseMap`.

## Usage Examples

### iOS

#### Basic Usage

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.delegate = self
seatCanvasView.loadBaseMap(svgData)
```

#### Basemap lifecycle and seat data

Implement `SeatCanvasViewDelegate` and push styles/seats in `didLoadBaseMap`:

```swift
extension MyViewController: SeatCanvasViewDelegate {
    func seatCanvasViewDidLoadBaseMap(_ view: SeatCanvasView) {
        view.registerPricecodes(["1", "2"])
        view.applySeatStyleJSONConfig(styleJson)
        view.updateSeatDatas(zoneId: "37492", seats: seatDataArray)
        view.updateSeatStatusesForZone(zoneId: "37492", statuses: statuses)
        view.setSelectedSeatIds(selectedSeatIds)
    }

    func seatCanvasViewDidUnloadBaseMap(_ view: SeatCanvasView) {
        // Stop timers, clear cached seat state
    }

    func seatCanvasView(_ view: SeatCanvasView, didTapSeat zoneId: String, seatId: String) -> Bool {
        // Update selection, then view.updateSelectedSeatIds(added:removed:)
        return true
    }

    func seatCanvasView(_ view: SeatCanvasView, didTapZone zoneId: String) {}
}
```

Style keys must match `SeatRenderStyleId.compose(pricecode, status, selected)`.

#### Applying Style Configuration

```swift
let builder = SeatStyleConfigBuilder()
builder.addCircleStyle(styleId: "selectable_unselected", fill: .red, overlay: .black, checkmark: .white)
builder.addSVGStyle(styleId: "custom_svg", content: svgContent)

let styleJson = builder.toJSONData()
// Apply in didLoadBaseMap via view.applySeatStyleJSONConfig(styleJson)
```

#### Clearing seat data

```swift
seatCanvasView.clearSeatData()
```

### Android

#### Basic Usage

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.setDelegate(delegate)
seatCanvasView.loadBaseMap(svgData)
```

#### Basemap lifecycle and seat data

```kotlin
val delegate = object : SeatCanvasRendererDelegate {
    override fun didLoadBaseMap() {
        seatCanvasView.registerPricecodes(arrayOf("1", "2"))
        seatCanvasView.applySeatStyleJSONConfig(styleJson)
        seatCanvasView.updateSeats("37492", seatDataArray)
        seatCanvasView.updateSeatStatusesForZone("37492", statuses)
        seatCanvasView.setSelectedSeatIds(selectedSeatIds.toTypedArray())
    }

    override fun didUnloadBaseMap() {
        // Stop timers, clear cached seat state
    }

    override fun didTapSeat(zoneId: String, seatId: String): Boolean {
        // Update selection, then seatCanvasView.updateSelectedSeatIds(added, removed)
        return true
    }

    override fun didTapZone(zoneId: String) {}
}
seatCanvasView.setDelegate(delegate)
```

#### Applying Style Configuration

```kotlin
val builder = SeatStyleConfigBuilder()
builder.addCircleStyle("selectable_unselected", Color.RED, Color.BLACK, Color.WHITE)
builder.addSVGStyle("custom_svg", svgContent)

val styleJson = builder.toJSONData()
// Apply in didLoadBaseMap via seatCanvasView.applySeatStyleJSONConfig(styleJson)
```

#### Clearing seat data

```kotlin
seatCanvasView.clearSeatData()
```

### OHOS

#### Basic Usage

```typescript
import { SeatCanvasView, SeatCanvasViewController, BaseMapFormat } from 'libseatcanvas';

@State controller: SeatCanvasViewController = new SeatCanvasViewController()

this.controller.setDelegate(delegate)
SeatCanvasView({ controller: this.controller })
  .width('100%')
  .height('100%')

// Load basemap (push seat data in didLoadBaseMap)
let manager = getContext(this).resourceManager;
await this.controller.loadFromAssets(manager, `svg/${basemapName}.svg`, BaseMapFormat.SVG);
```

#### Applying Style Configuration

SeatCanvas supports two seat styles: **circle style** and **SVG style**.

**Circle style configuration example:**

```typescript
import { SeatStyleConfigBuilder } from 'libseatcanvas';

let builder = new SeatStyleConfigBuilder();
builder.addCircleStyle('selectable_unselected', '#FFFF0000', '#B2000000', '#FFFFFFFF');
builder.addCircleStyle('selectable_selected', '#FFFF0000', '#B2000000', '#FFFFFFFF');
builder.addCircleStyle('sold', '#FFAAAAAA');
builder.addCircleStyle('locked', '#FF999999');
builder.addCircleStyle('disabled', '#FF666666');

let config = builder.toJSONString();
controller.applySeatStyleJSONConfig(config);
```

**SVG style configuration example:**

```typescript
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { resourceManager } from '@kit.LocalizationKit';

// Build SVG style configuration using SeatStyleBuilder
let manager = getContext(this).resourceManager;
let config = SeatStyleBuilder.BuildSVGSeatStyleConfig(manager);
controller.applySeatStyleJSONConfig(config);
```

`SeatStyleBuilder.BuildSVGSeatStyleConfig()` automatically loads the following SVG icons from sample resource files:
- `SeatCanvasSample.bundle/default/seatstyle/icon_seat_selectable.svg` - Selectable seat
- `SeatCanvasSample.bundle/default/seatstyle/icon_seat_selected.svg` - Selected seat
- `SeatCanvasSample.bundle/default/seatstyle/icon_seat_nonselectable.svg` - Non-selectable seat (sold/locked/disabled)

#### Renderer delegate and clearing seats

```typescript
import { SeatCanvasRendererDelegate, SeatRenderStyleId } from 'libseatcanvas';

let delegate: SeatCanvasRendererDelegate = {
  didLoadBaseMap: () => {
    controller.registerPricecodes(['1', '2']);
    controller.applySeatStyleJSONConfig(styleJson);
    controller.updateSeats('37492', seatDataArray);
    controller.updateSeatStatusesForZone('37492', statuses);
    controller.setSelectedSeatIds(selectedSeatIds);
  },
  didUnloadBaseMap: () => {
    // Stop timers, clear cached seat state
  },
  didTapSeat: (zoneId: string, seatId: string) => {
    return true;
  },
  didTapZone: (zoneId: string) => {
  },
};

controller.setDelegate(delegate);
await controller.loadFromAssets(manager, assetPath, BaseMapFormat.SVG, parseConfig);
controller.clearSeatData();

// Style registration keys must match:
SeatRenderStyleId.compose('1', 1, false);
```

### Web

Live demo: [preview.seatcanvas-demo.pages.dev](https://preview.seatcanvas-demo.pages.dev)

#### Quick Start

From the repository root, sync dependencies (includes `third_party/emsdk`):

```bash
./sync_deps.sh
```

Then build and run the demo:

```bash
cd web
npm install
npm run build:wasm   # runs setup.emsdk + emcmake (first run may download the SDK)
npm start            # → http://localhost:8081
```

Optional: `npm run setup:emsdk` — install/activate Emscripten only, without compiling WASM.

#### Build Commands

```bash
npm run build:wasm         # Build WASM (C++ → Emscripten)
npm run build:wasm:debug   # Build WASM (debug, separate build directory)
npm run build:lib          # Build JS library (ESM + CJS) + type declarations
npm run build:demo         # Build demo page
npm run build:deploy       # Build deployable static site
npm run build              # Full build (wasm + lib + demo)
```

#### Usage

```typescript
import { SeatCanvasApp, SeatCanvasInit, SeatCanvasFont } from 'libseatcanvas';

// 1. Initialize WASM module
const module = await SeatCanvasInit({
    locateFile: (file: string) => '/wasm/' + file,
});

// 2. Register fallback fonts before creating renderer
SeatCanvasFont.registerFallbackFontNames();

// 3. Create app and initialize renderer
const app = new SeatCanvasApp('#seat-canvas');
app.init(module);

// 4. Setup delegate callbacks
const delegate = app.getDelegate()!;
delegate.setDidLoadBaseMapCallback(() => {
    const renderer = app.getRenderer()!;
    renderer.registerPricecodes(['1', '2']);
    renderer.applySeatStyleJSONConfig(styleJson);
    renderer.setSeatData('37492', seats);
    renderer.updateSeatStatusesForZone('37492', statuses);
    renderer.setSelectedSeatIds(selectedSeatIds);
});

delegate.setDidTapSeatCallback((zoneId, seatId) => {
    // Handle seat selection
    return true;
});

delegate.setDidUpdateZoomLevelConfigCallback((event) => {
    renderer.setSeatRenderZoomThreshold(event.zoomLevels.venue);
});

// 5. Start render loop
app.start();

// 6. Load basemap
app.loadBaseMapFromSVG(svgData);
```

### Demo Video
#### Youtube
[![Watch on YouTube](https://img.youtube.com/vi/WjgUrQ23ByA/maxresdefault.jpg)](https://www.youtube.com/watch?v=WjgUrQ23ByA)

#### Bilibil
[![Watch Demo on Bilibili](https://i2.hdslb.com/bfs/archive/7237f42fa85778cad7542bb570c7457149893dff.jpg)](https://www.bilibili.com/video/BV1QnL46qEBb)

## License

See the [LICENSE](LICENSE) file for details.
