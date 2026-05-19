# SeatCanvas

SeatCanvas is a cross-platform seat map rendering library supporting iOS, Android, and OHOS (HarmonyOS) platforms, with Web platform support planned. The library provides seat map rendering, interactive gesture handling, seat state management, and more.

## Core Features

- **Cross-Platform Support**: iOS, Android, OHOS, Web (planned)
- **High-Performance Rendering**: GPU-accelerated rendering based on the tgfx graphics library
- **SVG Basemap Support**: Supports venue basemaps in SVG format
- **Gesture Interaction**: Supports tap, pan, zoom gestures with iOS-style inertial scrolling and elastic bounce-back animations
- **Customizable Styles**: Supports circle and SVG seat style configurations with dynamic switching

## Supported Platforms

- iOS 15.0+
- Android (via JNI)
- OHOS (HarmonyOS)
- Web (planned, via WebAssembly)

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
- Web (planned, via Emscripten)

### Build Requirements

- CMake 3.22 or higher
- C++17 standard
- Platform-specific build tools (Xcode, Android NDK, DevEco Studio)

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

Seat data (`seatdata/*.json`) format:
```json
{
  "37492": [
    {
      "seatId": "seat_0_0",
      "status": 0,
      "x": 33,
      "y": 411
    }
  ]
}
```

**Platform Resource Paths:**
- Shared `SeatCanvasSample.bundle` via symlinks

## Seat Data, Style Keys, and Renderer Delegate

- **String style keys**: Seat style JSON is a list of entries, each with a string `key` and a `config` (circle or SVG). Register keys with `SeatStyleConfigBuilder`; the same strings must be returned from `styleIdForSeat` (or the platform equivalent) so each seat resolves to a registered style. Unknown or empty keys skip drawing that seat.
- **Per-seat `styleId`**: `SeatData` may carry an optional `styleId`. The renderer still consults your delegate for the final style when implemented.
- **Updating and clearing**: Use `updateSeatDatas` (iOS) / `updateSeats` (Android, OHOS) to set geometry per zone. Call `clearSeatData` on the view or controller to remove all seat data and refresh rendering.
- **Circle style**: For builders that support it, `overlay` and `checkmark` are optional; only `fill` is required.
- **Delegate**: Implement `styleIdForSeat`, `didTapSeat`, and optionally `didTapZone` to drive selection and redraws (see platform types in the sample app).

## Usage Examples

### iOS

#### Basic Usage

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.delegate = self
seatCanvasView.loadBaseMap(svgData)
```

#### Applying Style Configuration

```swift
let builder = SeatStyleConfigBuilder()
builder.addCircleStyle(styleId: "selectable_unselected", fill: .red, overlay: .black, checkmark: .white)
builder.addSVGStyle(styleId: "custom_svg", content: svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

#### Seat data and clearing

```swift
seatCanvasView.updateSeatDatas(zoneId: "37492", seats: seatDataArray)
seatCanvasView.clearSeatData()
```

### Android

#### Basic Usage

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.loadBaseMap(svgData)
```

#### Applying Style Configuration

```kotlin
val builder = SeatStyleConfigBuilder()
builder.addCircleStyle("selectable_unselected", Color.RED, Color.BLACK, Color.WHITE)
builder.addSVGStyle("custom_svg", svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

#### Seat data and clearing

```kotlin
seatCanvasView.updateSeats("37492", seatDataArray)
seatCanvasView.clearSeatData()
```

### OHOS

#### Basic Usage

```typescript
import { SeatCanvasView, SeatCanvasViewController, BaseMapFormat } from 'libseatcanvas';

@State controller: SeatCanvasViewController = new SeatCanvasViewController()

SeatCanvasView({ controller: this.controller })
  .width('100%')
  .height('100%')

// Load basemap
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
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_canSelected.svg` - Selectable seat
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_selected.svg` - Selected seat
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_noSelected.svg` - Non-selectable seat (sold/locked/disabled)

#### Renderer delegate and clearing seats

```typescript
import { SeatCanvasRendererDelegate } from 'libseatcanvas';

let delegate: SeatCanvasRendererDelegate = {
  styleIdForSeat: (zoneId: string, seatId: string) => {
    return 'selectable_unselected';
  },
  didTapSeat: (zoneId: string, seatId: string) => {
    return true;
  },
  didTapZone: (zoneId: string) => {
  },
};

controller.setDelegate(delegate);
controller.updateSeats('37492', seatDataArray);
controller.clearSeatData();
```

### Web (Planned)

Web platform support is under development and will be implemented via WebAssembly.

[![SeatCanvas Demo](https://img.youtube.com/vi/3RaeqIqPPs8/0.jpg)](https://www.youtube.com/watch?v=3RaeqIqPPs8)


## License

See the [LICENSE](LICENSE) file for details.
