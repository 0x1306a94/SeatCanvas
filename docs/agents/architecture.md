# Architecture

## Overview

```
User Input → PlatformView → SeatCanvasCoreRenderer
                                    │
              ┌─────────────────────┼─────────────────────┐
              ↓                     ↓                     ↓
    ElasticZoomPanController  CustomPasses        SeatStyleAtlas
              ↓                     ↓                     ↓
         Gestures            GPU Rendering           Textures
```

## Core Components

### SeatCanvasCoreRenderer
**Location**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.{hpp,cpp}`

Core renderer responsible for rendering logic and lifecycle management.

**Key Methods**: `setBaseMapConfig()`, `setStyleKeyToConfigFromJSON()`, `handleTap/Pan/Pinch()`, `zoomToRect()`, `start/stop()`, `draw()`

**Internal Components**: `CustomBaseMapPass`, `CustomSeatPass`, `SeatStyleAtlasManager`, `ElasticZoomPanController`, `Animator`

### PlatformView
**Location**: `src/SeatCanvas/core/renderer/PlatformView.hpp`

Platform view abstraction providing GPU context and window.

Platform implementations: `IOSPlatformView.mm`, `AndroidPlatformView.cpp`, `OHOSPlatformView.cpp`

### Custom Render Passes
**Location**: `src/SeatCanvas/core/renderer/pass/`

- **CustomBaseMapPass**: Basemap GPU rendering using `BaseMapMeshBuilder` for mesh construction
- **CustomSeatPass**: Seat GPU rendering with instanced rendering, texture atlas for style management, UV offset updates

### Gesture Handling
**Location**: `src/SeatCanvas/core/gesture/`

- `ElasticZoomPanController` — elastic zoom/pan with boundary bounce-back
- `Scroller` — scroll animation, inertial scrolling
- `VelocityTracker` — velocity calculation
- `SpringBack` — elastic bounce-back

**Gesture States**: `Began`, `Changed`, `Ended`, `Cancelled`

### Seat Style System
**Location**: `src/SeatCanvas/core/style/`

- `CircleSeatStyleConfig` / `SVGSeatStyleConfig` — style types
- `SeatStyleKey` — zoneId + styleId
- `SeatStyleRenderer` → `CanvasSeatStyleRenderer` — renders to texture atlas
- `SeatStyleAtlasManager` — dynamic atlas generation/updates, UV offsets, multi-density

### Basemap Parser
**Location**: `src/SeatCanvas/core/parser/`

`IBaseMapParser` → `BaseMapParserFactory` (factory) → `SVGBaseMapParser`

Parse result: `BaseMapParseResult` (Layer + RegionInfo + SeatInfo)

**Configuration**: `BaseMapConfig` (BaseMapMeshBuilder + Layer + BaseMapRootLayer + original dimensions)

### Render Modes
**Location**: `src/SeatCanvas/core/SeatRenderMode.hpp`

- `ClickToEnter` — tap to enter region view; seat coordinates relative to region
- `ZoomBased` — auto-show based on zoom level; seat coordinates relative to canvas

### Layer System
**Location**: `src/SeatCanvas/core/layers/`

- `BaseMapRootLayer` — basemap layer tree
- `SeatRegionLayer` — region layer
- `SeatTextLayer` — text layer

### Drawers
**Location**: `src/SeatCanvas/core/drawers/`

- `SeatRegionNameLayerTree` — region names
- `SeatOverlayLayerTree` — minimap with opacity animations

### Animation System
**Location**: `src/SeatCanvas/core/animation/`

`Animator` — animation lifecycle, interpolation, callback notifications

### Delegate Pattern
**Location**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRendererDelegate.hpp`

- `shouldSelectSeat()` — can prevent selection
- `didSelectSeat()`
- `didDeselectSeat()`

## Render Pipeline

1. **Initialization**: Create `SeatCanvasCoreRenderer` → Set `PlatformView` → Create `ElasticZoomPanController` → Initialize styles
2. **Load Basemap**: `setBaseMapConfig()` → `BaseMapParserFactory` parse → Extract layers and region info
3. **Set Styles**: `setStyleKeyToConfigFromJSON()` → `SeatStyleAtlasManager` generates texture atlas
4. **Render Loop**: DisplayLink driven → `draw()` → CustomBaseMapPass → CustomSeatPass → Layer drawing → Present
5. **Gesture Handling**: Platform layer passes events → `ElasticZoomPanController` processes → Update state → Re-render
6. **Data Updates**: Update mesh/atlas → `invalidateContent()` marks for redraw

## Key APIs

**Basemap**: `setBaseMapConfig(baseMapConfig, renderMode)`

**Style**: `setStyleKeyToConfig()`, `setStyleKeyToConfigFromJSON(bytes, len)`

**Zoom**: `getZoomScale()`, `setZoomScale()`, `getMinimumZoomScale()`, `getMaximumZoomScale()`, `getContentOffset()`, `setContentOffset()`

**Gesture**: `handleTap(location)`, `handlePan(state, translation, timestampMs)`, `handlePinch(state, scale, center)`

**Region**: `zoomToRect(rect, animated, padding, durationMs)`, `setSelectedzoneId(zoneId)`, `getSeatRegionDataByPoint(x, y)`

**Coordinates**: `convertScreenToOriginal()`, `convertOriginalToScreen()`, `getVisibleOriginalRect()`, `isPointInContentArea()`

**Render control**: `start()`, `stop()`, `draw(force)`, `invalidateContent()`, `getFPS()`

**Config**: `setDelegate(delegate)`, `ZoomLevelConfig` (seat/row/zone/venue — controls seat rendering timing in ZoomBased mode)

## Performance

Instanced rendering reduces draw calls → Texture atlas manages styles → Mesh caching → Conditional rendering (zoom levels) → DisplayLink frame rate control → Incremental updates

## Important Notes

- Rendering operations execute on main thread
- Use smart pointers for memory management
- Properly release C++ resources when platform views are destroyed
- Distinguish between screen coordinates, content coordinates, and original coordinates
- Handle gesture recognizer conflicts and priorities
