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

本项目使用 **tgfx 的依赖管理方式**，通过 `DEPS` 文件和 [depctl](https://github.com/0x1306a94/depctl) 工具来管理第三方依赖。

### 安装依赖

首次克隆项目后，需要同步依赖：

```bash
./sync_deps.sh
```

或者手动执行：

```bash
# 安装 depctl 工具
brew install 0x1306a94/tap/depctl

# 同步依赖
depctl
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
├── third_party/             # 第三方依赖（通过 depsync 同步）
└── CMakeLists.txt          # CMake 构建配置
```

### SeatCanvasSample.bundle 目录结构

示例应用使用的资源包 `SeatCanvasSample.bundle` 采用以下目录结构：

```
SeatCanvasSample.bundle/
└── default/                  # 默认资源作用域
    ├── basemap/             # 底图文件目录
    │   ├── performbg.svg
    │   ├── performbg_2.svg
    │   ├── 73807.svg
    │   └── 73808.svg
    ├── seatstyle/           # 座位样式 SVG 图标目录
    │   ├── icon_seat_selectable.svg      # 可选座位图标
    │   ├── icon_seat_selected.svg        # 已选座位图标
    │   └── icon_seat_nonselectable.svg   # 不可选座位图标
    ├── zonedata/            # 区域数据 JSON 文件目录
    │   ├── performbg.json
    │   └── performbg_2.json
    └── seatdata/            # 座位数据 JSON 文件目录
        ├── performbg.json
        └── performbg_2.json
```

**资源作用域说明：**
- `default/`: 默认资源作用域，包含示例应用的基础资源
- `customized/`: 自定义资源作用域（可选），用于存放用户自定义的资源文件. 需和 `default` 结构保持一致。用于快速查看自己的场馆渲染效果

**文件命名规则：**
- 底图文件：SVG 格式，文件名对应 `zonedata/` 和 `seatdata/` 目录中的 JSON 文件名（不含扩展名）
- 区域数据：JSON 格式，文件名与对应的底图文件名一致（`.svg` 替换为 `.json`）
- 座位数据：JSON 格式，文件名与对应的底图文件名一致（`.svg` 替换为 `.json`）

**JSON 数据格式：**

区域数据 (`zonedata/*.json`) 格式：
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

座位数据 (`seatdata/*.json`) 格式：
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

**平台资源路径：**
- 通过软连复用 `SeatCanvasSample.bundle`

## 座位数据、样式键与渲染代理

- **字符串样式键**：座位样式 JSON 为若干条记录，每条含字符串 `key` 与 `config`（圆形或 SVG）。用 `SeatStyleConfigBuilder` 注册键名；渲染时由 `styleIdForSeat`（或各平台等价回调）返回相同样式 ID，才会按该配置绘制。未注册或空键则跳过该座位。
- **每座位的 styleId**：`SeatData` 可带可选 `styleId`；若实现了代理，最终以代理返回的样式为准。
- **更新与清空**：使用 `updateSeatDatas`（iOS）/ `updateSeats`（Android、鸿蒙）按区域写入几何信息；调用视图或控制器上的 `clearSeatData` 可清空全部座位数据并重绘。
- **圆形样式**：在支持的 Builder 上，`overlay` 与 `checkmark` 为可选，仅 `fill` 必填。
- **代理**：实现 `styleIdForSeat`、`didTapSeat`，可选 `didTapZone`，用于样式解析、点击与选座逻辑（详见各端类型定义与示例）。

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
builder.addCircleStyle(styleId: "selectable_unselected", fill: .red, overlay: .black, checkmark: .white)
builder.addSVGStyle(styleId: "custom_svg", content: svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

#### 座位数据与清空

```swift
seatCanvasView.updateSeatDatas(zoneId: "37492", seats: seatDataArray)
seatCanvasView.clearSeatData()
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
builder.addCircleStyle("selectable_unselected", Color.RED, Color.BLACK, Color.WHITE)
builder.addSVGStyle("custom_svg", svgContent)

seatCanvasView.applySeatStyleJSONConfig(builder.toJSONData())
```

#### 座位数据与清空

```kotlin
seatCanvasView.updateSeats("37492", seatDataArray)
seatCanvasView.clearSeatData()
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
builder.addCircleStyle('selectable_unselected', '#FFFF0000', '#B2000000', '#FFFFFFFF');
builder.addCircleStyle('selectable_selected', '#FFFF0000', '#B2000000', '#FFFFFFFF');
builder.addCircleStyle('sold', '#FFAAAAAA');
builder.addCircleStyle('locked', '#FF999999');
builder.addCircleStyle('disabled', '#FF666666');

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

`SeatStyleBuilder.BuildSVGSeatStyleConfig()` 会自动从示例资源文件加载以下 SVG 图标：
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_canSelected.svg` - 可选座位
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_selected.svg` - 已选座位
- `SeatCanvasSample.bundle/default/seatstyle/icon_chooseSeat_noSelected.svg` - 不可选座位（已售/锁定/禁用）

#### 渲染代理与清空座位

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

### Web (计划中)

Web 平台支持正在开发中，将通过 WebAssembly 实现渲染。

## 许可证

详见 [LICENSE](LICENSE) 文件。