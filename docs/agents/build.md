# Build Guide

## Build Commands

| Platform | Command | Notes |
|----------|---------|-------|
| iOS Device | `./ios/gen_ios && cd ios && xcodebuild -workspace ios/SeatCanvas.xcworkspace -scheme SeatCanvasSample -configuration Release -sdk iphoneos -arch arm64 CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO` | Requires Xcode |
| iOS (custom flags) | `./ios/gen_ios -DENABLE_TIME_PROFILER=ON` | Custom CMake flags |
| Android (all arch) | `cd android/SeatCanvasSample && ./gradlew assembleRelease` | All architectures |
| Android (arm64) | `cd android/SeatCanvasSample && ./gradlew assembleRelease -Parm64-only` | Faster builds |
| OHOS | `cd ohos && hvigorw assembleHar --mode module -p module=libseatcanvas@default -p buildMode=release -p product=default --no-daemon` | Requires HarmonyOS SDK |

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

**Build system**: CMake, C++17, supports iOS/Android/OHOS, enables tgfx SVG and Layers support.

## Troubleshooting

**Build fails with missing dependencies**
→ Run `./sync_deps.sh`

**Code formatting hook fails**
→ `brew install clang-format@14`

**iOS build fails with signing error**
→ Add `CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO` to xcodebuild command

**OHOS dependencies fail to install**
→ `ohpm config set registry https://ohpm.openharmony.cn/ohpm/`

**Pre-commit hook reformats code unexpectedly**
→ Expected behavior — commit will succeed after auto-formatting
