# SeatCanvas

SeatCanvas 是一个跨平台的座位图渲染库，支持 iOS、Android 和 OHOS（鸿蒙）平台，Web 平台也在计划中。该库提供了座位图渲染、交互手势处理、座位状态管理等功能。

## 核心特性

- **跨平台支持**: iOS、Android、OHOS，Web 平台（计划中）
- **高性能渲染**: 基于 tgfx 图形库的 GPU 加速渲染
- **SVG 底图支持**: 支持 SVG 格式的场馆底图
- **手势交互**: 支持点击、平移、缩放等手势操作，采用 iOS 风格的惯性滚动和弹性回弹动画
- **自定义样式**: 支持圆形和 SVG 两种座位样式配置，支持动态切换

## 支持的平台

- iOS 15.0+
- Android (通过 JNI)
- OHOS (鸿蒙系统)
- Web (计划中，将通过 WebAssembly 实现)

## 依赖管理

本项目使用 **tgfx 的依赖管理方式**，通过 `DEPS` 文件和 `depsync` 工具来管理第三方依赖。

### 安装依赖

首次克隆项目后，需要同步依赖：

```bash
./sync_deps.sh
```

或者手动执行：

```bash
# 安装 depsync 工具
npm install -g depsync

# 同步依赖
depsync
```

### 依赖说明

主要依赖项：
- **tgfx**: 图形渲染库（包含 SVG 解析和图层支持）
- **json**: JSON 解析库（nlohmann/json，通过 tgfx 引入）

依赖配置位于 `DEPS` 文件中，使用 tgfx 项目相同的依赖管理格式。

## 技术实现

### 手势惯性/弹性动画算法

本项目的手势惯性滚动和弹性回弹动画算法移植自 [fluid-scroll](https://github.com/ktiays/fluid-scroll) 项目。该算法实现了类似 iOS `UIScrollView` 的滚动效果，包括：

- **滚动惯性**: 支持平滑的惯性滚动，提供自然的减速效果
- **边缘回弹**: 实现边缘弹性回弹动画，提供流畅的边界反馈
- **速度跟踪**: 精确的手势速度计算，支持快速滑动后的惯性滚动

该算法在 C++ 核心层实现，确保跨平台一致的手势体验。

## 构建系统

项目使用 CMake 构建系统，支持：
- iOS (Xcode)
- Android (Gradle + CMake)
- OHOS (DevEco Studio + CMake)
- Web (计划中，通过 Emscripten)

### 构建要求

- CMake 3.22 或更高版本
- C++17 标准
- 平台特定的构建工具（Xcode、Android NDK、DevEco Studio）

### VS Code CMake 配置示例

`settings.json` 配置示例：

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

## 项目结构

```
SeatCanvas/
├── src/SeatCanvas/          # C++ 核心代码
│   ├── core/                # 核心功能模块
│   │   ├── renderer/            # 渲染器相关
│   │   │   ├── pass/            # 自定义渲染通道
│   │   ├── gesture/             # 手势处理
│   │   ├── style/               # 样式配置
│   │   ├── layers/              # 图层管理
│   │   ├── parser/              # 底图解析器
│   │   ├── animation/           # 动画系统
│   │   └── drawers/             # 绘制器
│   └── platform/            # 平台特定实现
├── ios/                     # iOS 示例和资源
├── android/                 # Android 示例和资源
├── ohos/                    # OHOS 示例和资源
├── resources/               # 资源文件
│   └── SVGBaseMap.bundle/  # 测试资源文件（SVG 底图示例）
├── third_party/             # 第三方依赖（通过 depsync 同步）
└── CMakeLists.txt          # CMake 构建配置
```

## 使用示例

### iOS

#### 基本使用

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.delegate = self
seatCanvasView.loadBaseMap(svgData)
```

#### 应用样式配置

```swift
let builder = SeatStyleConfigBuilder()
builder.addCircleStyle(status: 0, selected: false, fill: .red, overlay: .black, checkmark: .white)
builder.addSVGStyle(status: 0, selected: false, content: svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

### Android

#### 基本使用

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.loadBaseMap(svgData)
```

#### 应用样式配置

```kotlin
val builder = SeatStyleConfigBuilder()
builder.addCircleStyle(0U, false, Color.RED, Color.BLACK, Color.WHITE)
builder.addSVGStyle(0U, false, svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

### OHOS

#### 基本使用

```typescript
import { SeatCanvasView, SeatCanvasViewController, BaseMapFormat } from 'libseatcanvas';

@State controller: SeatCanvasViewController = new SeatCanvasViewController()

SeatCanvasView({ controller: this.controller })
  .width('100%')
  .height('100%')

// 加载底图
let manager = getContext(this).resourceManager;
await this.controller.loadFromAssets(manager, `svg/${basemapName}.svg`, BaseMapFormat.SVG);
```

#### 应用样式配置

SeatCanvas 支持两种座位样式：**圆形样式**和 **SVG 样式**。

**圆形样式配置示例：**

```typescript
import { SeatStyleConfigBuilder } from 'libseatcanvas';

let builder = new SeatStyleConfigBuilder();
builder.addCircleStyle(0, false, '#FFFF0000', '#B2000000', '#FFFFFFFF'); // 可选座位，未选中
builder.addCircleStyle(0, true, '#FFFF0000', '#B2000000', '#FFFFFFFF');   // 可选座位，已选中
builder.addCircleStyle(1, false, '#FFAAAAAA', '#B2000000', '#FFFFFFFF');   // 已售座位
builder.addCircleStyle(2, false, '#FF999999', '#B2000000', '#FFFFFFFF');   // 锁定座位
builder.addCircleStyle(3, false, '#FF666666', '#B2000000', '#FFFFFFFF');   // 禁用座位

let config = builder.toJSONString();
controller.applySeatStyleJSONConfig(config);
```

**SVG 样式配置示例：**

```typescript
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { resourceManager } from '@kit.LocalizationKit';

// 使用 SeatStyleBuilder 构建 SVG 样式配置
let manager = getContext(this).resourceManager;
let config = SeatStyleBuilder.BuildSVGSeatStyleConfig(manager);
controller.applySeatStyleJSONConfig(config);
```

`SeatStyleBuilder.BuildSVGSeatStyleConfig()` 会自动从资源文件加载以下 SVG 图标：
- `svg/icon_chooseSeat_canSelected.svg` - 可选座位
- `svg/icon_chooseSeat_selected.svg` - 已选座位
- `svg/icon_chooseSeat_noSelected.svg` - 不可选座位（已售/锁定/禁用）

#### 设置座位选择代理

```typescript
import { SeatCanvasRendererDelegate } from 'libseatcanvas';

let delegate: SeatCanvasRendererDelegate = {
  shouldSelectSeat: (regionId: string, seatId: string) => {
    // 返回 true 表示可以选中，false 表示不能选中
    return true;
  },
  didSelectSeat: (regionId: string, seatId: string) => {
    console.log(`座位已选中: regionId=${regionId}, seatId=${seatId}`);
  },
  didDeselectSeat: (regionId: string, seatId: string) => {
    console.log(`座位已取消选中: regionId=${regionId}, seatId=${seatId}`);
  },
};

controller.setDelegate(delegate);
```

### Web (计划中)

Web 平台支持正在开发中，将通过 WebAssembly 实现渲染。

## 许可证

详见 [LICENSE](LICENSE) 文件。