# Platform Integration

## iOS

**Main Files**:
- `src/SeatCanvas/platform/ios/` — C++ platform code
- `src/SeatCanvas/platform/apple/swift/` — Swift bridging
- `src/SeatCanvas/platform/ios/renderer/IOSPlatformView.mm` — Metal surface (`MTKView` / tgfx `MetalWindow`)
- Platform-facing UI: `ios/` sample app

**Usage**: Swift/Objective-C++ bridging, Metal rendering via `MTKView`, CADisplayLink render loop

**Style key helper**: `SeatRenderStyleId.compose(pricecode:status:selected:)` (same format as C++ `composeSeatStyleId`)

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. In `didLoadBaseMap`, apply styles and push seat data; in `didUpdateZoomLevelConfig`, set `seatRenderZoomThreshold`; in `didUnloadBaseMap`, stop timers and clear cached seat state. Callbacks are not replayed if the delegate is set after loading.

```swift
seatCanvasView.delegate = self
seatCanvasView.loadBaseMap(svgData, format: .svg)
// In didLoadBaseMap: registerPricecodes → applySeatStyleJSONConfig → setSeatData / updateSeatStatuses*
// In didUpdateZoomLevelConfig: seatRenderZoomThreshold = zoomLevels.venue
```

## Android

**Main Files**:
- `src/SeatCanvas/platform/android/` — C++ platform code
- `android/` — Kotlin sample app, JNI bridging
- `src/SeatCanvas/platform/android/renderer/AndroidPlatformView.cpp` — OpenGL ES context

**Usage**: JNI bridging, OpenGL ES rendering, TextureView render target, ValueAnimator render loop

**Style key helper**: `SeatRenderStyleId.compose(pricecode, status, selected)` (same format as C++ `composeSeatStyleId`)

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. Push styles and seat data in `didLoadBaseMap`; set `seatRenderZoomThreshold` in `didUpdateZoomLevelConfig`; clear state in `didUnloadBaseMap`.

```kotlin
seatCanvasView.setDelegate(delegate)
seatCanvasView.loadBaseMap(svgData, BaseMapFormat.SVG)
```

## OHOS (HarmonyOS)

**Main Files**:
- `src/SeatCanvas/platform/ohos/` — C++ platform code
- `ohos/` — ArkTS sample app, NAPI bridging
- `src/SeatCanvas/platform/ohos/OHOSPlatformView.cpp` — OpenGL ES context

**Usage**: NAPI bridging, OpenGL ES rendering, XComponent render target

**Style key helper**: `SeatRenderStyleId.compose(pricecode, status, selected)` (same format as C++ `composeSeatStyleId`)

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. Push styles and seat data in `didLoadBaseMap`; set `seatRenderZoomThreshold` in `didUpdateZoomLevelConfig`; clear state in `didUnloadBaseMap`.

```typescript
controller.setDelegate(delegate)
await controller.loadFromAssets(manager, assetPath, BaseMapFormat.SVG, parseConfig)
```

## Web (Emscripten)

**Main Files**:
- `src/SeatCanvas/platform/web/` — C++ platform code, embind bindings
- `web/` — TypeScript library + demo
- `src/SeatCanvas/platform/web/WebPlatformView.cpp` — WebGL context (tgfx `WebGLWindow`)
- `src/SeatCanvas/platform/web/WebRendererCore.cpp` — C++ renderer wrapper + embind bindings

**Usage**: Emscripten + WebAssembly, WebGL 2.0 rendering, `requestAnimationFrame` render loop, embind for C++ ↔ JS bridging

**Architecture** (three layers, following libpag's pattern):

```
C++ (embind) → TypeScript Binding → Application
```

1. **C++ WASM layer**: `WebRendererCore` wraps `SeatCanvasCoreRenderer`, exposed via embind as `SeatCanvasRenderer`
2. **TypeScript binding layer** (`binding.ts`): attaches TS classes to the WASM module, calls `TGFXBind`
3. **Application layer** (`SeatCanvasApp`): high-level API for consumers

**Style key helper**: JS side passes `pricecode` as string; C++ converts to index via `pricecodeIndexForCode()`

**Basemap lifecycle**: Call `SeatCanvasFont.registerFallbackFontNames()` **before** `app.init()`. Set delegate callbacks after init. In `didLoadBaseMap`, apply styles and push seat data; in `didUpdateZoomLevelConfig`, set `seatRenderZoomThreshold`; in `didUnloadBaseMap`, stop timers and clear cached seat state.

```typescript
// 1. Init module
const module = await SeatCanvasInit({ locateFile: (f) => '/wasm/' + f });

// 2. Register fonts before creating renderer
SeatCanvasFont.registerFallbackFontNames();

// 3. Create app
const app = new SeatCanvasApp('#seat-canvas');
app.init(module);

// 4. Setup delegate
const delegate = app.getDelegate()!;
delegate.setDidLoadBaseMapCallback(() => {
    renderer.registerPricecodes(['1', '2']);
    renderer.applySeatStyleJSONConfig(styleJson);
    renderer.setSeatData('37492', seats);
    renderer.updateSeatStatusesForZone('37492', statuses);
    renderer.setSelectedSeatIds(selectedSeatIds);
});
delegate.setDidUpdateZoomLevelConfigCallback((event) => {
    renderer.setSeatRenderZoomThreshold(event.zoomLevels.venue);
});

// 5. Start and load
app.start();
app.loadBaseMapFromSVG(svgData);
```

**Font registration**: Unlike native platforms, web fonts are registered from the JS side. Use `SeatCanvasFont.registerFallbackFontNames(fontNames?)` to set fallback typefaces before the renderer is created. If not called, a default set of system fonts is used.

**Resize handling**: Observe the canvas's parent container (not the canvas itself). Call `updateCanvasSize()` then `renderer.updateSize()` and `renderer.invalidateContent()`.

## Troubleshooting

**GPU rendering issues**
→ Check `getFPS()` output, ensure PlatformView is properly initialized

**Seat selection not working**
→ Verify delegate is set via `setDelegate()`, seat styles are pushed (`registerPricecodes`, `updateSeatStatuses*`, `setSelectedSeatIds`), style keys match `SeatRenderStyleId.compose`, and `didTapSeat` runs for hits

**Zoom animation stuttering**
→ Check `getFPS()`, ensure no blocking operations on main thread
