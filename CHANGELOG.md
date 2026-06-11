# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-06-11

First stable release. SeatCanvas is a cross-platform GPU-accelerated seat map rendering library for ticketing and venue seating applications.

### Highlights

- **Four platforms**: iOS (Metal), Android (OpenGL ES), OHOS (OpenGL ES), Web (WebAssembly + WebGL 2.0)
- **Push-based seat state**: Apps push pricecode, availability, and selection; C++ resolves style keys at render time
- **SVG basemap**: Parse venue SVG, instanced seat drawing, dynamic style atlas (circle + SVG icons)
- **Gestures**: Tap, pan, zoom with iOS-style inertial scrolling and elastic edge bounce (ported from [fluid-scroll](https://github.com/ktiays/fluid-scroll))
- **Web sample (online)**: https://preview.seatcanvas-demo.pages.dev — deployed `web/demo` (same as local `npm start` after `npm run build:deploy`)

### Added

#### Platforms & rendering

- iOS 15+ with Metal and `MTKView` ([#22](https://github.com/0x1306a94/SeatCanvas/pull/22))
- Web platform with Emscripten, TypeScript bindings, and demo ([#39](https://github.com/0x1306a94/SeatCanvas/pull/39))
- macOS platform and shared Apple rendering code (framework build + headless Metal for tests) ([#42](https://github.com/0x1306a94/SeatCanvas/pull/42))
- Android (JNI + OpenGL ES) and OHOS (NAPI + OpenGL ES) sample integrations
- Optional MSAA for basemap pass; MV/MVP matrix split for canvas and pass reuse ([#7](https://github.com/0x1306a94/SeatCanvas/pull/7), [#40](https://github.com/0x1306a94/SeatCanvas/pull/40))
- Per-seat rotation around seat center ([#6](https://github.com/0x1306a94/SeatCanvas/pull/6))
- Style atlas UV capacity expanded to 100 rects ([#38](https://github.com/0x1306a94/SeatCanvas/pull/38))

#### APIs & lifecycle

- Push-based seat state: `registerPricecodes`, `updateSeatStatusesForZone`, `setSelectedSeatIds`, `updateSelectedSeatIds` ([#30](https://github.com/0x1306a94/SeatCanvas/pull/30))
- String-based style keys and `SeatRenderStyleId.compose(pricecode, status, selected)` across platforms ([#24](https://github.com/0x1306a94/SeatCanvas/pull/24))
- Basemap lifecycle delegates: `didLoadBaseMap`, `didUnloadBaseMap`, `didUpdateZoomLevelConfig` ([#26](https://github.com/0x1306a94/SeatCanvas/pull/26), [#33](https://github.com/0x1306a94/SeatCanvas/pull/33))
- Zone APIs: tap callback, alternate minimap color, visible zone query, zoom-to-zone with `contentInset` ([#18](https://github.com/0x1306a94/SeatCanvas/pull/18), [#19](https://github.com/0x1306a94/SeatCanvas/pull/19), [#27](https://github.com/0x1306a94/SeatCanvas/pull/27), [#29](https://github.com/0x1306a94/SeatCanvas/pull/29))
- `getZoomScale` / zoom level APIs, configurable seat render zoom threshold, `setCanvasColor`
- Configurable basemap parse config ([#16](https://github.com/0x1306a94/SeatCanvas/pull/16))
- Debug HUD toggle ([#31](https://github.com/0x1306a94/SeatCanvas/pull/31))
- `clearSeatData` on all platforms ([#24](https://github.com/0x1306a94/SeatCanvas/pull/24))

#### Web-specific

- `SeatCanvasInit` integrated into the web library for simpler WASM startup
- FreeType font registration via `SeatCanvasFont.registerFonts()` ([#41](https://github.com/0x1306a94/SeatCanvas/pull/41))
- Seat status push uses raw `uint32` binary on Web (aligned with native) ([#34](https://github.com/0x1306a94/SeatCanvas/pull/34))

#### Testing & CI

- macOS autotest: unit tests + Metal rendering snapshot baselines ([#43](https://github.com/0x1306a94/SeatCanvas/pull/43))
- Headless Metal platform view; renderer decoupled from window ([#44](https://github.com/0x1306a94/SeatCanvas/pull/44))
- Broad unit test coverage (renderer, gesture, parser, style, data) ([#46](https://github.com/0x1306a94/SeatCanvas/pull/46), [#47](https://github.com/0x1306a94/SeatCanvas/pull/47))
- Per-GPU baseline cache for CI and local Metal GPUs ([#49](https://github.com/0x1306a94/SeatCanvas/pull/49))
- GitHub Actions: iOS, Android, Web, OHOS build workflows; dedicated autotest workflow
- Code coverage support via `./autotest.sh coverage`

#### Documentation

- English README; Chinese version in `README_zh.md` ([#12](https://github.com/0x1306a94/SeatCanvas/pull/12))
- Agent/developer docs under `docs/agents/`
- Architecture overview in `docs/SeatCanvas技术架构解析.md`

### Changed

- Core renderer refactor: `SeatDataManager`, `ViewportController` ([#32](https://github.com/0x1306a94/SeatCanvas/pull/32))
- Zone styling simplified to `alternateColor` only; `rainbowColor` renamed to `alternateColor` ([#3](https://github.com/0x1306a94/SeatCanvas/pull/3), [#4](https://github.com/0x1306a94/SeatCanvas/pull/4))
- Region renamed to Zone across layers and APIs
- Seat rendering uses instanced drawing for performance
- iOS Swift 6 concurrency cleanup ([#36](https://github.com/0x1306a94/SeatCanvas/pull/36))
- Dependency management migrated to depctl + `DEPS` (tgfx-style)

### Removed

- `SeatRenderMode` — seat visibility is zoom-threshold based only
- Zone highlight feature and `additionalAlpha` on zone mesh ([#4](https://github.com/0x1306a94/SeatCanvas/pull/4))
- Delegate `styleIdForSeat` — replaced by push-based seat state ([#30](https://github.com/0x1306a94/SeatCanvas/pull/30))
- Web `registerFallbackFontNames` — use `SeatCanvasFont.registerFonts()` with font file bytes instead

### Fixed

- SVG group transforms in basemap parsing ([#13](https://github.com/0x1306a94/SeatCanvas/pull/13))
- SVG text transforms preserved when building `SeatTextLayer` tree ([#2](https://github.com/0x1306a94/SeatCanvas/pull/2))
- OHOS release crash (deferred stop, null renderer guards) ([#35](https://github.com/0x1306a94/SeatCanvas/pull/35), [#37](https://github.com/0x1306a94/SeatCanvas/pull/37))
- DisplayLink start/stop idempotency ([#35](https://github.com/0x1306a94/SeatCanvas/pull/35))
- Zone color not applied when `setZoneData` called before `setBaseMapConfig`
- Stroke color alpha handling; dirty fill color refresh during zoom

### Known limitations (1.0.0)

- **Automated rendering tests** run on macOS Metal only; Android GLES, OHOS GLES, and Web WebGL are not covered by CI snapshot tests.
- **iOS shader warmup** is exposed as a temporary API; integration into renderer startup is planned (see TODO in `SeatCanvasCoreRendererBridge`).
- **Web fonts**: The npm package does not ship font files. Integrators must fetch and register fonts via `SeatCanvasFont.registerFonts()` before `app.init()`.
- **Experimental**: Zone name Picture rendering lives on branch `experimental/zone-name-picture-ab-test` and is **not** included in 1.0.0.

### Migration notes

If you followed early development commits:

1. Replace delegate style lookup with push APIs: `registerPricecodes` → `applySeatStyleJSONConfig` → `setSeatData` / `updateSeatStatusesForZone` → `setSelectedSeatIds`.
2. Use `SeatRenderStyleId.compose(pricecode, status, selected)` for style key format (string pricecode on native; Web passes pricecode string, C++ maps to index).
3. Call `setDelegate` **before** `loadBaseMap`. Apply styles and seat data in `didLoadBaseMap`; set `seatRenderZoomThreshold` in `didUpdateZoomLevelConfig`.
4. Web: load font bytes and call `SeatCanvasFont.registerFonts(textFont, emojiFont?)` after `SeatCanvasInit()` and before `app.init()`.

See [README](README.md) and [docs/agents/platform.md](docs/agents/platform.md) for integration examples.

[1.0.0]: https://github.com/0x1306a94/SeatCanvas/releases/tag/v1.0.0
