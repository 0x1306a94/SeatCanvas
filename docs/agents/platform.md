# Platform Integration

## iOS

**Main Files**:
- `src/SeatCanvas/platform/ios/` — C++ platform code
- `src/SeatCanvas/platform/apple/swift/` — Swift bridging
- `src/SeatCanvas/platform/ios/renderer/IOSPlatformView.mm` — Metal surface (`MTKView` / tgfx `MetalWindow`)
- Platform-facing UI: `ios/` sample app

**Usage**: Swift/Objective-C++ bridging, Metal rendering via `MTKView`, CADisplayLink render loop

**Style key helper**: `SeatRenderStyleId.compose(pricecode:status:selected:)` (same format as C++ `composeSeatStyleId`)

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. In `didLoadBaseMap`, apply styles and push seat data; in `didUnloadBaseMap`, stop timers and clear cached seat state. Callbacks are not replayed if the delegate is set after loading.

```swift
seatCanvasView.delegate = self
seatCanvasView.loadBaseMap(svgData, format: .svg)
// In didLoadBaseMap: seatRenderZoomThreshold → registerPricecodes → applySeatStyleJSONConfig → setSeatData / updateSeatStatuses*
```

## Android

**Main Files**:
- `src/SeatCanvas/platform/android/` — C++ platform code
- `android/` — Kotlin sample app, JNI bridging
- `src/SeatCanvas/platform/android/renderer/AndroidPlatformView.cpp` — OpenGL ES context

**Usage**: JNI bridging, OpenGL ES rendering, TextureView render target, ValueAnimator render loop

**Style key helper**: `SeatRenderStyleId.compose(pricecode, status, selected)` (same format as C++ `composeSeatStyleId`)

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. Push styles and seat data in `didLoadBaseMap`; clear state in `didUnloadBaseMap`.

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

**Basemap lifecycle**: Call `setDelegate` **before** `loadBaseMap`. Push styles and seat data in `didLoadBaseMap`; clear state in `didUnloadBaseMap`.

```typescript
controller.setDelegate(delegate)
await controller.loadFromAssets(manager, assetPath, BaseMapFormat.SVG, parseConfig)
```

## Troubleshooting

**GPU rendering issues**
→ Check `getFPS()` output, ensure PlatformView is properly initialized

**Seat selection not working**
→ Verify delegate is set via `setDelegate()`, seat styles are pushed (`registerPricecodes`, `updateSeatStatuses*`, `setSelectedSeatIds`), style keys match `SeatRenderStyleId.compose`, and `didTapSeat` runs for hits

**Zoom animation stuttering**
→ Check `getFPS()`, ensure no blocking operations on main thread
