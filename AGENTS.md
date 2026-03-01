# SeatCanvas Agent Guide

This file provides guidance to AI coding agents working inside this repository.

## Quick Reference

**Common Commands**
- Build iOS: `./ios/gen_ios` (simulator: `./ios/gen_simulator`)
- Build Android: `cd android/SeatCanvasSample && ./gradlew assembleRelease`
- Build OHOS: `cd ohos && hvigorw assembleHar --mode module -p module=libseatcanvas@default -p buildMode=release -p product=default --no-daemon`
- Format code: `./codeformat.sh`
- Sync dependencies: `./sync_deps.sh`

**Key Files**
- Core renderer: `src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.{hpp,cpp}`
- Platform views: `src/SeatCanvas/platform/{ios,android,ohos}/`
- Style system: `src/SeatCanvas/core/style/`
- Gesture handling: `src/SeatCanvas/core/gesture/ElasticZoomPanController.{hpp,cpp}`
- Render passes: `src/SeatCanvas/core/renderer/pass/`

**Critical Rules** ⚠️
- NO direct coding for new features - design first, ask questions, get approval
- Always initialize variables on declaration
- Use `./codeformat.sh` before commit
- Don't generate documentation unless explicitly requested

## ⚠️ Critical Development Rules

**MUST FOLLOW**:

1. **Design First**: For new features, output key interfaces and pseudocode first, ask ALL questions, get confirmation before coding
2. **No Documentation**: Don't generate README/docs unless explicitly requested
3. **Code Reuse**: Reuse existing project functionality, keep changes minimal, avoid duplicate code
4. **No Backward Compatibility**: When refactoring, review and clean up redundant code without backward compatibility hacks
5. **Variable Init**: ALL variables must be initialized at declaration (even `= {}`), smart pointers initialized with `nullptr`
6. **Function Order**: Implementation order in .cpp files should match header declaration order whenever possible
7. **Language**: code and comments in English

## Project Overview

SeatCanvas is a cross-platform seat map rendering library supporting iOS, Android, and OHOS (HarmonyOS). It provides high-performance seat map rendering, interactive gesture handling, and customizable style configuration.

### Core Features

- **Cross-Platform**: iOS, Android, OHOS
- **High-Performance Rendering**: GPU-accelerated rendering via tgfx graphics library with custom render passes
- **SVG Basemap Support**: Parse and render venue basemaps in SVG format
- **Gesture Interaction**: Tap, pan, pinch-to-zoom with elastic bounce-back effects
- **Customizable Styles**: Circle and SVG seat styles with dynamic updates
- **Render Modes**: ClickToEnter (tap to enter region) and ZoomBased (auto-show based on zoom level)
- **Minimap Support**: Small map overlay with fade animations
- **Animation System**: Built-in animation system for smooth zoom and pan transitions

### Architecture Overview

```
User Input → PlatformView → SeatCanvasCoreRenderer
                                    │
              ┌─────────────────────┼─────────────────────┐
              ↓                     ↓                     ↓
    ElasticZoomPanController  CustomPasses        SeatStyleAtlas
              ↓                     ↓                     ↓
         Gestures            GPU Rendering           Textures
```

## Build Commands

### Build Commands by Platform

| Platform | Command | Notes |
|----------|---------|-------|
| iOS Device | `./ios/gen_ios && cd ios && xcodebuild -workspace ios/SeatCanvas.xcworkspace -scheme SeatCanvasSample -configuration Release -sdk iphoneos -arch arm64 CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO` | Requires Xcode |
| iOS Simulator | `./ios/gen_simulator` | Faster for testing |
| iOS (custom flags) | `./ios/gen_ios -DENABLE_TIME_PROFILER=ON` | Custom CMake flags |
| Android (all arch) | `cd android/SeatCanvasSample && ./gradlew assembleRelease` | All architectures |
| Android (arm64) | `cd android/SeatCanvasSample && ./gradlew assembleRelease -Parm64-only` | Faster builds |
| OHOS | `cd ohos && hvigorw assembleHar --mode module -p module=libseatcanvas@default -p buildMode=release -p product=default --no-daemon` | Requires HarmonyOS SDK |

### Dependency Management

First-time setup requires syncing dependencies via depctl:

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

### Code Formatting

Format all C++ and Swift code:
```bash
./codeformat.sh
```

Uses clang-format 14.x for C++ and swiftformat for Swift. A pre-commit hook automatically formats code before commits.

## Directory Structure

```
SeatCanvas/
├── src/SeatCanvas/              # C++ core code
│   ├── core/                    # Core modules
│   │   ├── renderer/            # Renderer components
│   │   │   ├── pass/            # Custom render passes
│   │   │   └── SeatCanvasCoreRenderer.hpp/cpp
│   │   ├── gesture/             # Gesture handling
│   │   ├── style/               # Style configuration
│   │   ├── layers/              # Layer management
│   │   ├── parser/              # Basemap parsers
│   │   ├── animation/           # Animation system
│   │   └── drawers/             # Drawers
│   ├── platform/                # Platform-specific implementations
│   │   ├── ios/                 # iOS platform code
│   │   ├── android/             # Android platform code
│   │   └── ohos/                # OHOS platform code
│   └── swift/                   # Swift bridging code
├── ios/                         # iOS samples and resources
├── android/                     # Android samples and resources
├── ohos/                        # OHOS samples and resources
├── resources/                   # Resource files
│   └── SVGBaseMap.bundle/       # Test resources (SVG basemap examples)
├── third_party/                 # Third-party dependencies (synced via depctl)
└── CMakeLists.txt               # CMake build configuration
```

## Core Components

### 1. SeatCanvasCoreRenderer

**Location**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.hpp`

Core renderer responsible for rendering logic and lifecycle management.

**Key Methods**: `setBaseMapConfig()`, `setStyleKeyToConfigFromJSON()`, `handleTap/Pan/Pinch()`, `zoomToRect()`, `start/stop()`, `draw()`

**Internal Components**: `CustomBaseMapPass`, `CustomSeatPass`, `SeatStyleAtlasManager`, `ElasticZoomPanController`, `Animator`

### 2. PlatformView

**Location**: `src/SeatCanvas/core/renderer/PlatformView.hpp`

Platform view abstraction providing GPU context and window. Platform implementations: iOS (`IOSPlatformView.mm`), Android (`AndroidPlatformView.cpp`), OHOS (`OHOSPlatformView.cpp`)

### 3. Custom Render Passes

**Location**: `src/SeatCanvas/core/renderer/pass/`

**CustomBaseMapPass**: Basemap GPU rendering using BaseMapMeshBuilder for mesh construction

**CustomSeatPass**: Seat GPU rendering with instanced rendering, texture atlas for style management, UV offset updates

**Render Pipeline**: CommandEncoder → CustomBaseMapPass → CustomSeatPass → Canvas drawing → Layer drawing → Present

### 4. Gesture Handling

**Location**: `src/SeatCanvas/core/gesture/`

**Core Classes**:
- `ElasticZoomPanController` (elastic zoom/pan with boundary bounce-back)
- `Scroller` (scroll animation, inertial scrolling)
- `VelocityTracker` (velocity calculation)
- `SpringBack` (elastic bounce-back)

**Gesture States**: `Began`, `Changed`, `Ended`, `Cancelled`

### 5. Seat Style System

**Location**: `src/SeatCanvas/core/style/`

**Style Types**:
- `CircleSeatStyleConfig` (circle style)
- `SVGSeatStyleConfig` (SVG style)
- `SeatStyleKey` (zoneId + styleId)

**Style Rendering**: `SeatStyleRenderer` (base class) → `CanvasSeatStyleRenderer` (renders to texture atlas)

**Atlas Management**: `SeatStyleAtlasManager` (dynamic atlas generation/updates, UV offsets, multi-density adaptation)

### 6. Basemap Parser

**Location**: `src/SeatCanvas/core/parser/`

**Interface**: `IBaseMapParser` → `BaseMapParserFactory` (factory pattern) → `SVGBaseMapParser` (SVG implementation)

**Parse Result**: `BaseMapParseResult` (Layer + RegionInfo + SeatInfo)

### 7. Render Modes

**Location**: `src/SeatCanvas/core/SeatRenderMode.hpp`

**Modes**:
- `ClickToEnter` (tap to enter region view, seat coordinates relative to region)
- `ZoomBased` (auto-show based on zoom level, seat coordinates relative to canvas)

### 8. Layer System

**Location**: `src/SeatCanvas/core/layers/`

**Core Layers**:
- `BaseMapRootLayer` (basemap layer tree)
- `SeatRegionLayer` (region layer)
- `SeatTextLayer` (text layer)

### 9. Drawers

**Location**: `src/SeatCanvas/core/drawers/`

**Drawers**:
- `SeatRegionNameLayerTree` (region names)
- `SeatOverlayLayerTree` (minimap with opacity animations)

### 10. Animation System

**Location**: `src/SeatCanvas/core/animation/`

**Core Class**: `Animator` (animation lifecycle, interpolation, callback notifications)

### 11. Delegate Pattern

**Location**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRendererDelegate.hpp`

**Interface**:
- `shouldSelectSeat()` (can prevent selection)
- `didSelectSeat()`
- `didDeselectSeat()`

## Platform Integration

### iOS

**Main Files**: `platform/ios/ui/SeatCanvasView.swift`, `platform/ios/renderer/IOSPlatformView.mm`

**Usage**: Swift/Objective-C++ bridging, OpenGL ES rendering, CADisplayLink render loop

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.loadBaseMap(svgData, format: .svg)
seatCanvasView.applySeatStyleJSONConfig(styleJsonData)
```

### Android

**Main Files**: `android/.../SeatCanvasView.kt`, `platform/android/renderer/AndroidPlatformView.cpp`

**Usage**: JNI bridging, OpenGL ES rendering, TextureView render target, ValueAnimator render loop

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.loadBaseMap(svgData, BaseMapFormat.SVG)
```

### OHOS

**Main Files**: `ohos/.../SeatCanvasView.ets`, `platform/ohos/OHOSPlatformView.cpp`

**Usage**: NAPI bridging, OpenGL ES rendering, XComponent render target

```typescript
@State controller: SeatCanvasViewController = new SeatCanvasViewController()
SeatCanvasView({ controller: this.controller })
```

## Basemap Format

**Location**: `src/SeatCanvas/core/parser/BaseMapFormat.hpp`

**Format**: SVG (parsed via `SVGBaseMapParser`)

**Configuration**: `BaseMapConfig` (BaseMapMeshBuilder + Layer + BaseMapRootLayer + original dimensions)

## Render Pipeline

1. **Initialization**: Create `SeatCanvasCoreRenderer` → Set `PlatformView` → Create `ElasticZoomPanController` → Initialize styles
2. **Load Basemap**: `setBaseMapConfig()` → `BaseMapParserFactory` parse → Extract layers and region info
3. **Set Styles**: `setStyleKeyToConfigFromJSON()` → `SeatStyleAtlasManager` generates texture atlas
4. **Render Loop**: `DisplayLink` driven → `draw()` → CustomBaseMapPass → CustomSeatPass → Layer drawing → Present
5. **Gesture Handling**: Platform layer passes events → `ElasticZoomPanController` processes → Update state → Re-render
6. **Data Updates**: Update mesh/atlas → `invalidateContent()` marks for redraw

## Key APIs

**Basemap Configuration**: `setBaseMapConfig(baseMapConfig, renderMode)`

**Style Configuration**: `setStyleKeyToConfig(styleKeyToConfig)`, `setStyleKeyToConfigFromJSON(bytes, len)`

**Zoom Control**: `getZoomScale()`, `setZoomScale()`, `getMinimumZoomScale()`, `getMaximumZoomScale()`, `getContentOffset()`, `setContentOffset()`

**Gesture Handling**: `handleTap(location)`, `handlePan(state, translation, timestampMs)`, `handlePinch(state, scale, center)`

**Region Operations**: `zoomToRect(rect, animated, padding, durationMs)`, `setSelectedzoneId(zoneId)`, `getSeatRegionDataByPoint(x, y)`

**Coordinate Conversion**: `convertScreenToOriginal()`, `convertOriginalToScreen()`, `getVisibleOriginalRect()`, `isPointInContentArea()`

**Render Control**: `start()`, `stop()`, `draw(force)`, `invalidateContent()`, `getFPS()`

**Delegate**: `setDelegate(delegate)`

**Zoom Level Config**: `ZoomLevelConfig` (seat/row/zone/venue, controls seat rendering timing in ZoomBased mode)

## Development Guide

### Adding New Seat Styles

1. Inherit from `SeatStyleConfig`
2. Implement `SeatStyleRenderer`
3. Add style key to `SeatStyleKey`
4. Add JSON support in `SeatStyleConfigJSONHelper`
5. Register with renderer

### Adding New Basemap Formats

1. Implement `IBaseMapParser`
2. Add enum to `BaseMapFormat`
3. Register in `BaseMapParserFactory`
4. Implement parse logic returning `BaseMapParseResult`

### Custom Render Passes

1. Inherit from `CustomRenderPass`
2. Implement `onDraw()`
3. Implement shader methods
4. Implement `outputImage()`
5. Integrate in `SeatCanvasCoreRenderer`

## Dependencies and Build

**Dependencies**:
- tgfx (graphics rendering library, SVG parsing, layer support)
- json (nlohmann/json for style config parsing)

**Build**: CMake, C++17, supports iOS/Android/OHOS, enables tgfx SVG and Layers support

## Coding Standards

### Commit Message Format

- **Length**: Within 120 characters
- **Language**: English only
- **Ending**: Must end with period (.)
- **Focus**: User-perceivable changes, not implementation details
- **Format**: `<verb> <what> <context>.`

**Good Examples**:
- `Add minimap fade animation for better visual feedback.`
- `Fix seat selection crash when tapping outside region bounds.`
- `Optimize GPU rendering by reducing draw calls with instancing.`

**Bad Examples**:
- `update code` (too vague)
- `修复bug` (wrong language)
- `Add feature, fix bug, refactor` (multiple changes, has comma)

### C++ Coding Standards

#### Code Formatting

Use **clang-format 14.x** with `.clang-format` config. Run `./codeformat.sh` to format all code.

#### Naming Conventions

* **Classes**: PascalCase (`SeatCanvasCoreRenderer`)
* **Functions**: camelCase (`setBaseMapConfig`, `getZoomScale`)
  - Class static methods, global functions/variables: Start with uppercase
  - Member methods/variables, local variables: Start with lowercase
* **Member Variables**: camelCase
* **Constants**: UPPER_SNAKE_CASE (`MINIMAP_FADE_DURATION_MS`)
* **Namespaces**: lowercase, supports nesting (`namespace kk::renderer`)
* **Enums**: PascalCase (`enum class GestureState`)
* **Variable Naming**: Avoid abbreviations, keep short and semantically clear

#### Code Style

* **Variable Init**: Always initialize variables at declaration (even `= {}`), smart pointers initialized with `nullptr`
* **Function Order**: Implementation order in .cpp files should match header declaration order whenever possible
* **Smart Pointers**: Prefer smart pointers for memory management
* **Const Correctness**: Use const wherever possible
* **Move Semantics**: Prefer move semantics for large objects
* **Memory Optimization**: Use `reserve` for vectors when size is known in advance

#### Comment Standards

* **include/ Directory APIs**: Require detailed comments with parameter descriptions
* **Other Public Methods**: One-sentence description of main functionality
* **Private Methods**: No comments required
* **In-Function Code**: No inline comments unless design intent cannot be understood from code alone
* **Documentation Comments**: Use Doxygen style (`///` or `/** */`)

#### Git Commits

Project has pre-commit hook that auto-formats code (C++ with clang-format, Swift with swiftformat).

## Performance Optimization

Instanced rendering reduces draw calls → Texture atlas manages styles → Mesh caching → Conditional rendering (zoom levels) → DisplayLink frame rate control → Incremental updates

## Important Notes

* Rendering operations execute on main thread
* Use smart pointers for memory management
* Properly release C++ resources when platform views are destroyed
* Distinguish between screen coordinates, content coordinates, and original coordinates
* Handle gesture recognizer conflicts and priorities
* Use `getFPS()` to monitor performance
* Choose render mode based on use case

## Common Tasks

### Loading Seat Map

Prepare SVG basemap → `setBaseMapConfig()` → `setStyleKeyToConfigFromJSON()` → Auto-extract regions and seat info

### Handling Seat Tap

Implement `SeatCanvasCoreRendererDelegate` → `setDelegate()` → `handleTap()` → `getSeatRegionDataByPoint()` to find region

### Custom Seat Styles

Prepare style JSON → `setStyleKeyToConfigFromJSON()` → Auto-update texture atlas → Apply styles via `SeatStyleKey`

### Region Zoom

`zoomToRect()` to zoom to region (with animation params) → `getVisibleOriginalRect()` to get visible area → `setSelectedzoneId()` to set selected region (ClickToEnter mode)

### Switching Render Modes

Specify `SeatRenderMode` in `setBaseMapConfig()`: `ZoomBased` (auto-show based on zoom) or `ClickToEnter` (tap to enter region view)

## Troubleshooting

**Build fails with missing dependencies**
→ Run `./sync_deps.sh` to sync all dependencies

**Code formatting hook fails**
→ Ensure clang-format 14.x is installed: `brew install clang-format@14`

**iOS build fails with signing error**
→ Add `CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO` to xcodebuild command

**GPU rendering issues**
→ Check `getFPS()` output, ensure PlatformView is properly initialized

**OHOS dependencies fail to install**
→ Check registry: `ohpm config set registry https://ohpm.openharmony.cn/ohpm/`

**Pre-commit hook reformats code unexpectedly**
→ This is expected behavior - commit will succeed after auto-formatting

**Seat selection not working**
→ Verify delegate is set via `setDelegate()` and `shouldSelectSeat()` returns true

**Zoom animation stuttering**
→ Check frame rate with `getFPS()`, ensure no blocking operations on main thread

## Related Files

- **README.md**: Project overview and build instructions
- **CMakeLists.txt**: CMake build configuration
- **docs/layer_version_implementation.md**: Layer version implementation documentation
- **include/**: Public header files
- **src/**: Source code implementation
- **platform/**: Platform-specific code
