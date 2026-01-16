# SeatCanvas AGENTS.md

## 项目概述

SeatCanvas 是一个跨平台的座位图渲染库，支持 iOS、Android 和 OHOS（鸿蒙）平台。该库提供了高性能的座位图渲染、交互手势处理、自定义样式配置等功能。

### 核心特性

- **跨平台支持**: iOS、Android、OHOS
- **高性能渲染**: 基于 tgfx 图形库的 GPU 加速渲染，使用自定义渲染通道（Custom Render Pass）
- **SVG 底图支持**: 支持 SVG 格式的场馆底图解析和渲染
- **手势交互**: 支持点击、平移、缩放等手势操作，带有弹性回弹效果
- **自定义样式**: 支持圆形和 SVG 两种座位样式配置，支持动态更新
- **渲染模式**: 支持两种渲染模式（点击进入模式、缩放级别模式）
- **Minimap 支持**: 支持小地图显示和动画
- **动画系统**: 内置动画系统支持平滑的缩放和平移动画

## 架构设计

### 目录结构

```
SeatCanvas/
├── src/SeatCanvas/              # C++ 核心代码
│   ├── core/                    # 核心功能模块
│   │   ├── renderer/            # 渲染器相关
│   │   │   ├── pass/            # 自定义渲染通道
│   │   │   └── SeatCanvasCoreRenderer.hpp/cpp
│   │   ├── gesture/             # 手势处理
│   │   ├── style/               # 样式配置
│   │   ├── layers/              # 图层管理
│   │   ├── parser/              # 底图解析器
│   │   ├── animation/           # 动画系统
│   │   └── drawers/             # 绘制器
│   ├── platform/                # 平台特定实现
│   │   ├── ios/                 # iOS 平台代码
│   │   ├── android/             # Android 平台代码
│   │   └── ohos/                # OHOS 平台代码
│   └── swift/                   # Swift 桥接代码
├── ios/                     # iOS 示例和资源
├── android/                 # Android 示例和资源
├── ohos/                    # OHOS 示例和资源
├── resources/               # 资源文件
│   └── SVGBaseMap.bundle/  # 测试资源文件（SVG 底图示例）
├── third_party/             # 第三方依赖（通过 depsync 同步）
└── CMakeLists.txt          # CMake 构建配置
```

### 核心组件

#### 1. SeatCanvasCoreRenderer

**位置**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.hpp`

核心渲染器，负责渲染逻辑和生命周期管理。

**关键方法**: `setBaseMapConfig()`, `setStyleKeyToConfigFromJSON()`, `handleTap/Pan/Pinch()`, `zoomToRect()`, `start/stop()`, `draw()`

**内部组件**: `CustomBaseMapPass`, `CustomSeatPass`, `SeatRegionMeshManager`, `SeatStyleAtlasManager`, `ElasticZoomPanController`, `Animator`

#### 2. PlatformView

**位置**: `src/SeatCanvas/core/renderer/PlatformView.hpp`

平台视图抽象接口，提供 GPU 上下文和窗口。各平台实现：iOS (`IOSPlatformView.mm`), Android (`AndroidPlatformView.cpp`), OHOS (`OHOSPlatformView.cpp`)

#### 3. 自定义渲染通道

**位置**: `src/SeatCanvas/core/renderer/pass/`

**CustomBaseMapPass**: 底图 GPU 渲染，使用 BaseMapMeshBuilder 构建网格

**CustomSeatPass**: 座位 GPU 渲染，实例化渲染，纹理图集管理样式，UV 偏移更新

**渲染流程**: CommandEncoder → CustomBaseMapPass → CustomSeatPass → Canvas 绘制 → 图层绘制 → 提交呈现

#### 4. 手势处理

**位置**: `src/SeatCanvas/core/gesture/`

**核心类**: `ElasticZoomPanController`（弹性缩放平移，边界回弹），`Scroller`（滚动动画，惯性滚动），`VelocityTracker`（速度计算），`SpringBack`（弹性回弹）

**手势状态**: `Began`, `Changed`, `Ended`, `Cancelled`

#### 5. 座位样式系统

**位置**: `src/SeatCanvas/core/style/`

**样式类型**: `CircleSeatStyleConfig`（圆形样式），`SVGSeatStyleConfig`（SVG 样式），`SeatStyleKey`（regionId + styleId）

**样式渲染**: `SeatStyleRenderer`（基类）→ `CanvasSeatStyleRenderer`（渲染到纹理图集）

**图集管理**: `SeatStyleAtlasManager`（动态生成/更新图集，UV 偏移，多密度适配）

#### 6. 底图解析器

**位置**: `src/SeatCanvas/core/parser/`

**接口**: `IBaseMapParser` → `BaseMapParserFactory`（工厂模式）→ `SVGBaseMapParser`（SVG 实现）

**解析结果**: `BaseMapParseResult`（Layer + RegionInfo + SeatInfo）

#### 7. 渲染模式

**位置**: `src/SeatCanvas/core/SeatRenderMode.hpp`

**模式**: `ClickToEnter`（点击进入区域视图，座位坐标相对区域），`ZoomBased`（缩放级别自动显示，座位坐标相对画布）

#### 8. 图层系统

**位置**: `src/SeatCanvas/core/layers/`

**核心图层**: `BaseMapRootLayer`（底图图层树），`SeatRegionLayer`（区域图层），`SeatTextLayer`（文本图层）

#### 9. 绘制器

**位置**: `src/SeatCanvas/core/drawers/`

**绘制器**: `SeatRegionNameLayerTree`（区域名称），`SeatOverlayLayerTree`（Minimap，透明度动画）

#### 10. 动画系统

**位置**: `src/SeatCanvas/core/animation/`

**核心类**: `Animator`（动画生命周期，插值动画，回调通知）

#### 11. 代理模式

**位置**: `src/SeatCanvas/core/renderer/SeatCanvasCoreRendererDelegate.hpp`

**接口**: `shouldSelectSeat()`（可阻止选择），`didSelectSeat()`，`didDeselectSeat()`

## 平台集成

### iOS

**主要文件**: `platform/ios/ui/SeatCanvasView.swift`, `platform/ios/renderer/IOSPlatformView.mm`

**使用**: Swift/Objective-C++ 桥接，OpenGL ES 渲染，CADisplayLink 渲染循环

```swift
let seatCanvasView = SeatCanvasView(frame: view.bounds)
seatCanvasView.loadBaseMap(svgData, format: .svg)
seatCanvasView.applySeatStyleJSONConfig(styleJsonData)
```

### Android

**主要文件**: `android/.../SeatCanvasView.kt`, `platform/android/renderer/AndroidPlatformView.cpp`

**使用**: JNI 桥接，OpenGL ES 渲染，TextureView 渲染目标，ValueAnimator 渲染循环

```kotlin
val seatCanvasView = SeatCanvasView(context)
seatCanvasView.loadBaseMap(svgData, BaseMapFormat.SVG)
```

### OHOS

**主要文件**: `ohos/.../SeatCanvasView.ets`, `platform/ohos/OHOSPlatformView.cpp`

**使用**: NAPI 桥接，OpenGL ES 渲染，XComponent 渲染目标

```typescript
@State controller: SeatCanvasViewController = new SeatCanvasViewController()
SeatCanvasView({ controller: this.controller })
```

## 底图格式

**位置**: `src/SeatCanvas/core/parser/BaseMapFormat.hpp`

**格式**: SVG（通过 `SVGBaseMapParser` 解析）

**配置**: `BaseMapConfig`（BaseMapMeshBuilder + Layer + BaseMapRootLayer + 原始尺寸）

## 渲染流程

1. **初始化**: 创建 `SeatCanvasCoreRenderer` → 设置 `PlatformView` → 创建 `ElasticZoomPanController` → 初始化样式
2. **加载底图**: `setBaseMapConfig()` → `BaseMapParserFactory` 解析 → 提取图层和区域信息
3. **设置样式**: `setStyleKeyToConfigFromJSON()` → `SeatStyleAtlasManager` 生成纹理图集
4. **渲染循环**: `DisplayLink` 驱动 → `draw()` → CustomBaseMapPass → CustomSeatPass → 图层绘制 → 提交
5. **手势处理**: 平台层传递 → `ElasticZoomPanController` 处理 → 更新状态 → 重渲染
6. **数据更新**: 更新网格/图集 → `invalidateContent()` 标记重绘

## 关键 API

**底图配置**: `setBaseMapConfig(baseMapConfig, renderMode)`

**样式配置**: `setStyleKeyToConfig(styleKeyToConfig)`, `setStyleKeyToConfigFromJSON(bytes, len)`

**缩放控制**: `getZoomScale()`, `setZoomScale()`, `getMinimumZoomScale()`, `getMaximumZoomScale()`, `getContentOffset()`, `setContentOffset()`

**手势处理**: `handleTap(location)`, `handlePan(state, translation, timestampMs)`, `handlePinch(state, scale, center)`

**区域操作**: `zoomToRect(rect, animated, padding, durationMs)`, `setSelectedRegionId(regionId)`, `getSeatRegionDataByPoint(x, y)`

**坐标转换**: `convertScreenToOriginal()`, `convertOriginalToScreen()`, `getVisibleOriginalRect()`, `isPointInContentArea()`

**渲染控制**: `start()`, `stop()`, `draw(force)`, `invalidateContent()`, `getFPS()`

**代理**: `setDelegate(delegate)`

**缩放级别配置**: `ZoomLevelConfig`（zoomScale9/18/30/50，控制 ZoomBased 模式座位渲染时机）

## 开发指南

### 添加新的座位样式

继承 `SeatStyleConfig` → 实现 `SeatStyleRenderer` → 在 `SeatStyleKey` 中添加样式键 → 在 `SeatStyleConfigJSONHelper` 中添加 JSON 支持 → 注册到渲染器

### 添加新的底图格式

实现 `IBaseMapParser` → 在 `BaseMapFormat` 中添加枚举 → 在 `BaseMapParserFactory` 中注册 → 实现解析逻辑返回 `BaseMapParseResult`

### 自定义渲染通道

继承 `CustomRenderPass` → 实现 `onDraw()` → 实现着色器方法 → 实现 `outputImage()` → 在 `SeatCanvasCoreRenderer` 中集成

## 依赖与构建

**依赖**: tgfx（图形渲染库，SVG 解析和图层支持），json（nlohmann/json，样式配置解析）

**构建**: CMake，C++17，支持 iOS/Android/OHOS，启用 tgfx SVG 和 Layers 支持

## 编码规范

### 开发流程规范

* **对话语言**: 对话用中文，代码和注释用英语
* **新需求流程**: 禁止直接编码，先输出关键接口和伪代码，穷尽所有疑问向用户提问，方案确认后才可编码
* **文档生成**: 不生成说明文件，除非明确要求
* **代码复用**: 复用项目已有功能，保持变更简洁，避免重复代码
* **重构原则**: 重构时审查关联代码合理性，顺带清理冗余，不考虑向后兼容

### C++ 代码规范

#### 代码格式化

使用 **clang-format 14.x** 格式化，配置文件 `.clang-format`，运行 `./codeformat.sh` 格式化所有代码。

#### 命名规范

* **类名**: PascalCase（`SeatCanvasCoreRenderer`）
* **函数名**: camelCase（`setBaseMapConfig`, `getZoomScale`）
  - 类静态方法、全局函数/变量：大写开头
  - 成员方法/变量、局部变量：小写开头
* **成员变量**: camelCase
* **常量**: 全大写下划线（`MINIMAP_FADE_DURATION_MS`）
* **命名空间**: 小写，支持嵌套（`namespace kk::renderer`）
* **枚举**: PascalCase（`enum class GestureState`）
* **变量命名**: 避免缩写，简短且语义明确

#### 代码风格

* **变量初始化**: 声明时一律赋初始值（即使是 `= {}`），智能指针初始值使用 `nullptr`
* **函数顺序**: CPP 文件里的函数实现顺序与头文件中定义的顺序尽可能一致
* **智能指针**: 优先使用智能指针管理内存
* **const 正确性**: 尽可能使用 const
* **移动语义**: 大对象优先使用移动语义
* **内存优化**: vector 能预知大小时，使用 `reserve` 提前分配内存

#### 注释规范

* **include/ 目录 API**: 需详细注释含参数描述
* **其他公开方法**: 一段话描述主要功能
* **私有方法**: 不加注释
* **函数内代码**: 不加行注释，除非只看代码无法理解设计意图
* **文档注释**: 使用 Doxygen 风格（`///` 或 `/** */`）

#### Git 提交

项目配置了 pre-commit hook，提交前自动格式化代码（C++ 用 clang-format，Swift 用 swiftformat）。

## 性能优化

实例化渲染减少 Draw Call → 纹理图集（Atlas）管理样式 → 网格缓存管理 → 条件渲染（缩放级别） → DisplayLink 帧率控制 → 增量更新

## 注意事项

* 渲染操作在主线程执行
* 使用智能指针管理内存
* 平台视图销毁时正确释放 C++ 资源
* 区分屏幕坐标、内容坐标、原始坐标
* 处理手势识别器冲突和优先级
* 使用 `getFPS()` 监控性能
* 根据场景选择渲染模式

## 常见任务

### 加载座位图

准备 SVG 底图 → `setBaseMapConfig()` → `setStyleKeyToConfigFromJSON()` → 自动提取区域和座位信息

### 处理座位点击

实现 `SeatCanvasCoreRendererDelegate` → `setDelegate()` → `handleTap()` → `getSeatRegionDataByPoint()` 查找区域

### 自定义座位样式

准备样式 JSON → `setStyleKeyToConfigFromJSON()` → 自动更新纹理图集 → 根据 `SeatStyleKey` 应用样式

### 区域缩放

`zoomToRect()` 缩放到区域（可指定动画参数）→ `getVisibleOriginalRect()` 获取可见区域 → `setSelectedRegionId()` 设置选中区域（ClickToEnter 模式）

### 切换渲染模式

在 `setBaseMapConfig()` 时指定 `SeatRenderMode`：`ZoomBased`（缩放级别自动显示）或 `ClickToEnter`（点击进入区域视图）

## 相关文件

- **README.md**: 项目基本信息和构建说明
- **CMakeLists.txt**: CMake 构建配置
- **docs/layer_version_implementation.md**: 图层版本实现文档
- **include/**: 公共头文件
- **src/**: 源代码实现
- **platform/**: 平台特定代码
