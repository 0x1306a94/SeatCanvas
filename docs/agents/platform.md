# Platform Integration

## iOS

**Main Files**:
- `src/SeatCanvas/platform/ios/` — C++ platform code
- `src/SeatCanvas/platform/apple/swift/` — Swift bridging
- `src/SeatCanvas/platform/ios/renderer/IOSPlatformView.mm` — Metal surface (`MTKView` / tgfx `MetalWindow`)
- Platform-facing UI: `ios/` sample app

**Usage**: Swift/Objective-C++ bridging, Metal rendering via `MTKView`, CADisplayLink render loop

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.loadBaseMap(svgData, format: .svg)
seatCanvasView.applySeatStyleJSONConfig(styleJsonData)
```

## Android

**Main Files**:
- `src/SeatCanvas/platform/android/` — C++ platform code
- `android/` — Kotlin sample app, JNI bridging
- `src/SeatCanvas/platform/android/renderer/AndroidPlatformView.cpp` — OpenGL ES context

**Usage**: JNI bridging, OpenGL ES rendering, TextureView render target, ValueAnimator render loop

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.loadBaseMap(svgData, BaseMapFormat.SVG)
```

## OHOS (HarmonyOS)

**Main Files**:
- `src/SeatCanvas/platform/ohos/` — C++ platform code
- `ohos/` — ArkTS sample app, NAPI bridging
- `src/SeatCanvas/platform/ohos/OHOSPlatformView.cpp` — OpenGL ES context

**Usage**: NAPI bridging, OpenGL ES rendering, XComponent render target

```typescript
@State controller: SeatCanvasViewController = new SeatCanvasViewController()
SeatCanvasView({ controller: this.controller })
```

## Troubleshooting

**GPU rendering issues**
→ Check `getFPS()` output, ensure PlatformView is properly initialized

**Seat selection not working**
→ Verify delegate is set via `setDelegate()`, `styleIdForSeat` returns a registered style id, and `didTapSeat` runs for hits

**Zoom animation stuttering**
→ Check `getFPS()`, ensure no blocking operations on main thread
