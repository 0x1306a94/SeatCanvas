# SeatCanvas：跨平台高性能座位图渲染引擎架构解析

## 1. 引言

SeatCanvas 是一个面向 iOS、Android 和 HarmonyOS（OHOS）的跨平台座位图渲染库，核心 C++ 代码约 180+ 源文件，三端共享同一套渲染核心。它基于 tgfx 图形库（类 Skia 的 2D 渲染引擎），解决了大型场馆座位图在移动设备上的核心难题：如何在有限的 GPU 资源下，以 60fps（高刷设备可达 120fps，如 iPad Pro、Mate 60 系列）流畅渲染上万个可交互的座位元素，同时支持手势驱动的缩放、平移和弹性动画。

本文面向有图形/渲染背景的工程师，从渲染管线、手势系统、纹理图集、跨平台抽象和性能优化五个维度，深入剖析 SeatCanvas 的架构设计。文中所有代码路径均为项目实际文件路径，可直接在仓库中查阅对应源码。

## 2. 整体架构

SeatCanvas 采用经典的分层架构，自底向上分为**图形抽象层**、**渲染核心层**和**平台接入层**：

```
┌────────────────────────────────────────────────────────────┐
│ iOS(Swift/Metal) │ Android(Kotlin/GLES) │ OHOS(ArkTS/GLES) │
├────────────────────────────────────────────────────────────┤
│                  Swift Bridge / JNI / NAPI                 │
├────────────────────────────────────────────────────────────┤
│                  SeatCanvasCoreRenderer                    │
│   ┌──────────────┬──────────────┬──────────────────────┐   │
│   │  Viewport    │RenderPasses  │ Style & Atlas Manager│   │
│   │  Controller  │(BaseMap/Seat)│                      │   │
│   ├──────────────┼──────────────┼──────────────────────┤   │
│   │  Gesture     │ Data Manager │   Layer & Drawer     │   │
│   │  Controller  │              │   System             │   │
│   └──────────────┴──────────────┴──────────────────────┘   │
├────────────────────────────────────────────────────────────┤
│                    tgfx (Graphics Library)                 │
├────────────────────────────────────────────────────────────┤
│        Metal (iOS)      │      OpenGL ES (Android/OHOS)    │
└────────────────────────────────────────────────────────────┘
```

### 2.1 核心类：SeatCanvasCoreRenderer

`SeatCanvasCoreRenderer`（`src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.{hpp,cpp}`）是整个系统的调度中心。它持有以下内部组件：


| 组件      | 类名                         | 职责                |
| ------- | -------------------------- | ----------------- |
| 视口控制器   | `ViewportController`       | 管理缩放/偏移状态、动画、坐标转换 |
| 底图渲染通道  | `CustomBaseMapPass`        | GPU 渲染 SVG 底图区域网格 |
| 座位渲染通道  | `CustomSeatPass`           | GPU 实例化渲染座位       |
| 纹理图集管理器 | `SeatStyleAtlasManager`    | 动态生成座位样式纹理图集      |
| 座位数据管理器 | `SeatDataManager`          | 座位实例数据的增删改查与选取    |
| 手势控制器   | `ElasticZoomPanController` | 弹性缩放/平移 + 惯性动画    |
| 底图网格构建器 | `BaseMapMeshBuilder`       | 管理三角化后的区域顶点数据     |
| 覆盖图层    | `SeatOverlayLayerTree`     | 小地图、返回按钮等 UI 叠加层  |
| 区域名称图层  | `SeatZoneNameLayerTree`    | 区域标签文本            |
| 动画系统    | `Animator`                 | 动画生命周期管理、插值器      |


### 2.2 Delegate 模式

SeatCanvasCoreRenderer 通过 `SeatCanvasCoreRendererDelegate` 接口（`src/SeatCanvas/core/renderer/SeatCanvasCoreRendererDelegate.hpp:18`）与平台层通信：

```cpp
class SeatCanvasCoreRendererDelegate {
public:
    // 底图生命周期
    virtual void didLoadBaseMap(uint32_t) = 0;
    virtual void didUnloadBaseMap(uint32_t) = 0;
    virtual void didUpdateZoomLevelConfig(uint32_t, const SeatCanvasZoomLevelConfigEvent &) = 0;

    // 交互回调
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId) = 0;
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) = 0;

    // 视口事件（7个）
    virtual void viewportWillBeginDragging(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidScroll(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndDragging(uint32_t, const SeatCanvasViewportEvent &, bool) = 0;
    virtual void viewportDidEndDecelerating(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportWillBeginZooming(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidZoom(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndZooming(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndScrollingAnimation(uint32_t, const SeatCanvasViewportEvent &) = 0;
};
```

**关键约束**：`setDelegate()` 必须在 `loadBaseMap()` 之前调用。底图生命周期回调（`didLoadBaseMap`、`didUnloadBaseMap`、`didUpdateZoomLevelConfig`）在 `handleBaseMapChanged()` 中同步触发，不会因 delegate 后来设置而"重放"已发生的事件。Tap 和 Viewport 回调则由对应的事件处理方法按需分发。

### 2.3 典型加载流程

```
setDelegate()
  → loadBaseMap() / setBaseMapConfig()
    → didLoadBaseMap:
        registerPricecodes()
        → applySeatStyleJSONConfig() / setStyleKeyToConfigFromJSON()
        → per zone: setSeatData()
        → updateSeatStatusesForZone()
        → setSelectedSeatIds()
    → didUpdateZoomLevelConfig:
        seatRenderZoomThreshold = zoomLevels.venue
    → didUnloadBaseMap: 停止轮询，清理缓存状态
```

## 3. 渲染管线

### 3.1 绘制流程

`SeatCanvasCoreRenderer::draw(bool force)` 的完整执行顺序如下（`SeatCanvasCoreRenderer.cpp:439`）：

```
1. Prepare（数据准备阶段）
   ├── _seatZoneNameLayer->prepare()
   ├── _overlayLayer->prepare()
   ├── 脏检查：无内容变化 && !force && !_invalidate → 跳过本帧
   ├── _seatAtlasManager->update()     ← 按需重建图集
   └── prepareSeatIfNeeded()          ← 收集可见座位实例数据

2. canvas->clear(backgroundColor)

3. executeCustomRenderPass（GPU Pass 阶段，两个 Pass 共用一个 CommandEncoder）
   ├── CustomBaseMapPass::onDraw()  → 底图离屏纹理
   └── CustomSeatPass::onDraw()     → 座位离屏纹理
   └── encoder->finish() → queue->submit()

4. Canvas 合成（由底向上逐层覆盖）
   ├── canvas->drawImage(baseMapImage)      ← 第1层：底图
   ├── _seatZoneNameLayer->draw()           ← 第2层：区域名称标签
   ├── canvas->drawImage(seatImage)         ← 第3层：座位
   ├── _overlayLayer->draw()                ← 第4层：小地图 + 半透明返回层
   └── drawDebugHUD()                       ← 第5层：调试信息

5. context->flush() → context->submit() → 呈现到屏幕
```

关键设计点：
- 两个 RenderPass **共享一个 CommandEncoder**，在一次 `executeCustomRenderPass` 中依次执行后统一 submit
- 座位（第3层）绘制在区域名称（第2层）之上，小地图（第4层）在最顶层
- 在 Pass 阶段之前有**脏检查优化**：如果 zone name 和 overlay 的 DisplayList 都没有内容变化，且没有 `force` 或 `invalidate` 标记，则跳过整帧
- 座位实例数据在 Pass 阶段之前的 `prepareSeatIfNeeded()` 中收集，确保 GPU 命令提交前数据已就绪

两个 Pass 均继承自 `CustomRenderPass`（`src/SeatCanvas/core/renderer/pass/CustomRenderPass.hpp:25`），它定义了标准模板方法：

```cpp
class CustomRenderPass : public IContextAware {
public:
    virtual bool onDraw(tgfx::CommandEncoder *encoder,
                        const SeatCanvasCoreRendererState *state) = 0;
    virtual std::shared_ptr<tgfx::Image> outputImage() = 0;

protected:
    // 子类必须实现
    virtual std::string onBuildVertexShader() const = 0;
    virtual std::string onBuildFragmentShader() const = 0;

    // 子类可覆写
    virtual std::vector<tgfx::VertexBufferLayout> vertexBufferLayouts() const;
    virtual std::vector<tgfx::BindingEntry> uniformBlocks() const;
    virtual std::vector<tgfx::BindingEntry> textureSamplers() const;
    virtual tgfx::MultisampleDescriptor multisample() const;
    virtual std::vector<tgfx::PipelineColorAttachment> colorAttachments() const;

    // 管线创建（基类实现）
    std::shared_ptr<tgfx::RenderPipeline> createPipeline(tgfx::GPU *gpu) const;
};
```

这种模板方法模式意味着：每个子类只需提供 Shader 源码和 Buffer Layout，而管线创建、版本前缀注入（自动检测 OpenGL ES vs Desktop GL 并插入 `#version 300 es` 或 `#version 150`）、Shader 编译和错误处理全部由基类 `CreatePipeline` 统一完成（`CustomRenderPass.cpp:16-51`）。

### 3.2 CustomBaseMapPass：底图渲染

底图渲染的核心任务是将 SVG 解析产出的三角化区域网格渲染到纹理。

**网格构建器**（`BaseMapMeshBuilder`, `src/SeatCanvas/core/renderer/BaseMapMeshBuilder.hpp:36`）：

所有区域的顶点**按顺序追加到单个 VBO** 中，每个区域占据一段连续范围。这是一个关键的不变量设计：

```cpp
class BaseMapMeshBuilder {
    struct ZoneDrawRange {
        uint32_t vertexOffset = 0;
        uint32_t vertexCount = 0;
    };

    std::vector<std::shared_ptr<ZoneMeshInfo>> zoneMeshInfos;
    std::vector<ZoneDrawRange> drawRanges;
    std::unordered_map<std::string, std::shared_ptr<ZoneMeshInfo>> zoneIdToMeshInfo;
    std::unordered_map<std::string, size_t> zoneIdToIndex;
    uint32_t totalVertexCount = 0;
};
```

它还提供空间查询能力——`findZoneIntersectingRect`（包围盒相交查询）和 `findZoneContainingPoint`（命中测试），用于可见性裁剪和点击检测。

**顶点结构**（`BaseMapZoneVertex.hpp`）包含四个属性：

- `x, y`：2D 位置（底图原始坐标空间）
- `coverage`：AA 覆盖率（用于边缘抗锯齿）
- `colorIndex`：int32 类型，用于颜色纹理查表

**Fragment Shader 的颜色查找**：底图的每个区域可以有"原始色"（Original）和"交替色"（Alternate）两种状态。Fragment Shader 将 `colorIndex` 按纹理宽度拆分为行/列坐标（`row = index / width, col = index % width`），通过 `texelFetch` 从一张 2D 颜色纹理中进行点采样获取实际颜色，再乘以 `vCoverage` 实现边缘抗锯齿。这种设计使得区域颜色切换（如点击高亮）无需重建网格——只需更新颜色纹理即可。

**CustomBaseMapPass::onDraw 执行流程**（`src/SeatCanvas/core/renderer/pass/CustomBaseMapPass.cpp:111`）：

1. **CPU 可见性裁剪**：`getVisibleZoneMesheIndices(state)` 根据 MVP 矩阵对每个区域做包围盒-视口相交测试，筛选可见区域的索引列表
2. **MSAA（可选）**：如果启用多重采样，创建 MSAA 纹理作为中间颜色附件
3. **准备 GPU 资源**：
  - 创建 RenderTexture（与视口同尺寸）
  - 创建/获取 FillPipeline
  - 更新 FillVBO（将全量区域顶点写入单个缓冲区）
  - 更新 UBO（MVP 矩阵 + 颜色纹理尺寸）
  - 更新颜色纹理
4. **绘制**：`renderFill()` 遍历可见区域索引，将连续的区域段合并为一次 `draw(Triangles, vertexCount, 1, vertexOffset)`，通过顶点偏移量直接从全量 VBO 中读取对应区域数据，减少 draw call 次数
5. **输出**：`outputImage()` 返回渲染完成的底图纹理

### 3.3 CustomSeatPass：实例化座位渲染

座位渲染是 SeatCanvas 的性能关键路径——一场馆可能包含上万个座位，逐个 Draw Call 不可行。`CustomSeatPass` 采用 **GPU 实例化渲染（Instanced Rendering）** 来解决这个问题。

**设计思路**：

- 一个座位的基本几何体是固定的 Quad（两个三角形组成矩形），作为顶点缓冲区 slot 0（`baseQuadVBO`）
- 每个座位的差异化数据（位置偏移、旋转、UV 索引）存储在实例缓冲区 slot 1（`instanceBuffer`）中
- 一次 `draw(TriangleStrip, 4, seatCount)` 调用即可渲染所有座位

**实例数据结构**（`SeatInstanceData`, `src/SeatCanvas/core/renderer/SeatInstanceData.hpp`）：

每个座位实例携带：位置偏移（`positionOffset`）、旋转角度（`rotation`）、和纹理坐标索引（`textureCoordIndex`）——后者用于从图集中选取正确的样式。

**CustomSeatPass 的顶点属性布局**（`src/SeatCanvas/core/renderer/pass/CustomSeatPass.hpp:78`）：

```cpp
// Slot 0: 基础几何顶点属性（per-vertex）
tgfx::Attribute position;          // Quad 的本地坐标
tgfx::Attribute textureCoord;      // Quad 的本地 UV

// Slot 1: 实例属性（per-instance）
tgfx::Attribute positionOffset;    // 世界空间位置偏移
tgfx::Attribute rotation;          // 旋转角度
tgfx::Attribute textureCoordIndex; // UV 图集索引（用于从纹理数组中选取对应样式）
```

**Vertex Shader 的核心逻辑**：

1. 从实例缓冲区读取 `positionOffset`、`rotation`、`textureCoordIndex`
2. 通过 `textureCoordIndex` 从 UBO 中的 UV 数组查表，获得该座位样式在图集中的实际 UV 范围
3. 将 Quad 顶点的本地 UV 重新映射到图集 UV
4. 应用旋转矩阵和位置偏移
5. 通过 MVP 矩阵变换到裁剪空间

**UBO 结构**（`layout(std140)`）：`uMVP`（mat3）、`uSeatHalfSize`（vec2，座位半尺寸，用于将 Quad 顶点平移到几何中心后做旋转）、`uTextureCoordRects`（vec4 动态数组，每个样式存 uMin/vMin/uMax/vMax）。

**CustomSeatPass::onDraw 执行流程**（`src/SeatCanvas/core/renderer/pass/CustomSeatPass.cpp:124`）：

1. 检查前置条件（encoder、图集纹理、座位数据非空）
2. 准备 GPU 资源：
  - RenderTexture（与视口同尺寸）
  - Pipeline + Sampler
  - BaseQuadVBO（4 个顶点，TriangleStrip 布局）
  - InstanceBuffer（动态更新，映射写入）
  - UBO
3. 开始 RenderPass → 设置 Viewport、Pipeline、纹理、UBO、两个顶点缓冲区
4. **一次 Draw Call**：`renderPass->draw(TriangleStrip, 4, seats.size())`
5. 结束 RenderPass

通过实例化，上万个座位只需一个 Draw Call，大幅降低 CPU 端的命令提交开销。

### 3.4 覆盖图层（Overlay & Minimap）

除两个核心 RenderPass 外，`SeatCanvasCoreRenderer` 还管理两个 Drawer：

- **SeatOverlayLayerTree**（`src/SeatCanvas/core/drawers/SeatOverlayLayerTree.hpp:34`）：绘制小地图（minimap）和半透明返回层（back layer），支持淡入淡出动画
- **SeatZoneNameLayerTree**（`src/SeatCanvas/core/drawers/SeatZoneNameLayerTree.hpp`）：绘制区域名称标签

两者均继承自 `Drawer` 抽象类（`src/SeatCanvas/core/drawers/Drawer.hpp:22`），遵循 `prepare() → draw()` 的两阶段模式。`SeatOverlayLayerTree` 内部使用 `tgfx::DisplayList` 的 `RenderMode::Direct` 模式进行图层管理和脏区域追踪。

## 4. 手势系统

手势系统是 SeatCanvas 交互体验的核心，由 `ElasticZoomPanController`（`src/SeatCanvas/core/gesture/ElasticZoomPanController.{hpp,cpp}`，约 159 行头文件）统一管理。

### 4.1 整体架构

```
平台手势事件
  ↓
handlePan(state, translation, timestampMs)
handlePinch(state, scale, center)
  ↓
ElasticZoomPanController
  ├── PanProxy: 手势代理，追踪 Pan 状态、位移和速度（内置 VelocityTracker）
  ├── VelocityTracker: 速度追踪（用于计算惯性滑动初速度）
  ├── Scroller: 衰减曲线（惯性滑动）
  ├── SpringBack: 弹性回弹（临界阻尼弹簧模型）
  └── ScrollProperties: 每个轴的动画状态（X / Y / Zoom）
```

### 4.2 弹性边界

`ElasticZoomPanController` 的核心行为是弹性边界——当用户拖动超出内容边界时，内容会跟随手指但带有 rubber-band 阻尼；松手后以临界阻尼弹簧动画回弹到合法范围。

边界约束通过 `contentInset`（`EdgeInsets`, 默认 40px 四边）扩展，提供合理的 overscroll 空间。`rubberBandForOffset` 方法实现非线性阻尼（`ElasticZoomPanController.cpp:445`）：

1. 若 offset 在 [minOffset, maxOffset] 范围内 → 直接返回，不做处理
2. 否则取最近的边界值作为 target，计算超出距离 `distance = |offset - target|`
3. 使用 `CalculateRubberBandOffset` 计算阻尼后的距离：

```
damped = (1 - 1 / (distance / range * 0.55 + 1)) * range
```

4. 返回 `target + damped * sign(offset - target)`

其中 `RubberBandCoefficient = 0.55`（`GestureConstant.hpp:21`），`range` 为视口宽度或高度。这个公式确保小幅度超出时阻尼较弱（接近 1:1 跟随），大幅度超出时 damped 渐近趋近于 range，不会无限漂移。

### 4.3 惯性滑动与回弹

手势结束时，`VelocityTracker` 计算出手指离开屏幕时的即时速度。速度和剩余距离通过 `Scroller` 产生衰减曲线，或通过 `SpringBack` 产生回弹曲线。

`SpringBack` 使用**临界阻尼弹簧模型**：

```cpp
class SpringBack {
    void absorb(float velocity, float distance);              // 初始化：吸收初速度和距离
    void absorbWithResponse(float velocity, float distance, float response); // 可指定响应时间
    std::optional<float> value(float time) const;             // 查询任意时刻的位移
};
```

数学上，临界阻尼弹簧的位移方程为：

```
x(t) = (c₁ + c₂·t) · e^(-λt)
```

其中 λ 由响应时间推导，c₁ 和 c₂ 由初始位移和初速度确定。临界阻尼保证以最快速度回到平衡位置且不产生振荡，这对移动端体验至关重要。

### 4.4 缩放处理

Pinch 手势的处理比 Pan 更复杂——缩放必须以手指中心（`center` 参数，在 viewport 坐标系中）为锚点，同时保持该锚点对应的内容点在缩放前后不变。

关键实现是 `handlePinch`（`ElasticZoomPanController.cpp:132`）：

1. **Began**：记录起始缩放比例 `_pinchStartZoomScale = _zoomScale`，清除所有进行中的滚动/回弹动画
2. **Changed**：`logicalScale = _pinchStartZoomScale * scale`（无阻尼的理想缩放），再通过 `zoomScaleForRubberBandScale` 施加 rubber-band 得到视觉缩放比例；然后以手势中心为锚点，重新计算 `contentOffset` 使锚点对应的内容位置在缩放前后不变
3. **Ended**：将当前 zoom 钳制到 [minZoom, maxZoom] 范围；若超出边界则调用 `prepareBouncingForZoomScale` 以临界阻尼弹簧动画回弹到合法值

### 4.5 DisplayLink 驱动

`handleDisplayLinkFire()` 是每帧调用的动画推进方法（由平台层的 DisplayLink/CADisplayLink/ValueAnimator 触发）。它遍历三个动画轴（X 偏移、Y 偏移、Zoom 缩放），在每个轴上检查是否有未完成的惯性滑动或回弹动画，有则推进一帧并返回 `true`（需要继续刷新），无则返回 `false`（可以停止 DisplayLink 以省电）。

```cpp
bool ElasticZoomPanController::handleDisplayLinkFire() {
    bool needUpdate = false;
    needUpdate |= handleDisplayLinkFireForAxis(_scrollPropertiesX.get(), true);   // X轴
    needUpdate |= handleDisplayLinkFireForAxis(_scrollPropertiesY.get(), false);  // Y轴
    needUpdate |= handleDisplayLinkFireForZoomScale();                             // 缩放
    return needUpdate;
}
```

### 4.6 手势状态机

手势识别通过 `GestureState` 枚举（`Began` → `Changed` → `Ended` / `Cancelled`）传达：

- **Began**：记录初始状态，停止所有进行中的动画
- **Changed**：实时更新缩放/偏移
- **Ended**：计算速度，启动惯性滑动或弹性回弹
- **Cancelled**：直接回弹到合法范围

## 5. 纹理图集系统

SeatCanvas 的每个座位可以有独立的视觉样式（圆形颜色、SVG 图标等），为每个座位单独渲染一张纹理显然不现实。解决方案是**纹理图集（Texture Atlas）**。

图集采用等宽网格布局，最多 10 列，行数按样式数量自动增长。每个格位在 GPU 上对应 `RGBA_8888` 纹理的一个矩形区域，通过归一化 UV 坐标寻址：

```
    col=0          col=1          col=2        ...   col=9
  ┌──────┐      ┌──────┐      ┌──────┐            ┌──────┐
  │idx=0 │      │idx=1 │      │idx=2 │            │idx=9 │
  │style │      │style │      │style │    ...     │style │
  │  A   │      │  B   │      │  C   │            │  J   │
  └──────┘      └──────┘      └──────┘            └──────┘
  ┌──────┐      ┌──────┐      ┌──────┐            ┌──────┐
  │idx=10│      │idx=11│      │idx=12│            │idx=19│
  │style │      │style │      │style │    ...     │style │
  │  K   │      │  L   │      │  M   │            │  T   │
  └──────┘      └──────┘      └──────┘            └──────┘
  ┌──────┐      ┌──────┐      ┌──────┐            ┌──────┐
  │ idx= │      │ idx= │      │ idx= │            │ idx= │
  │N*10  │      │ ...  │      │ ...  │    ...     │ ...  │
  │      │      │      │      │      │            │      │
  └──────┘      └──────┘      └──────┘            └──────┘
```

**UV 坐标计算**：格位 `(row, col)` 对应索引 `idx = row × 10 + col`，其归一化 UV 为：

```
uvMin = (col × cellW / atlasW,  row × cellH / atlasH)
uvMax = ((col+1) × cellW / atlasW,  (row+1) × cellH / atlasH)
```

**扁平化 UV Offsets 数组**：`[uMin₀, vMin₀, uMax₀, vMax₀,  uMin₁, vMin₁, uMax₁, vMax₁,  ...]`，每 4 个 float 对应一个样式。Shader 端通过实例属性 `textureCoordIndex` 索引这个数组，完成图集采样。

### 5.1 SeatStyleAtlasManager

`SeatStyleAtlasManager`（`src/SeatCanvas/core/style/SeatStyleAtlasManager.hpp:33`）负责图集的生成和 UV 坐标管理：

```cpp
class SeatStyleAtlasManager : public IContextAware {
public:
    void update(float density, const tgfx::Size &seatSize);
    std::shared_ptr<tgfx::Texture> getAtlasTexture() const;
    bool getUVCoords(const std::string &styleId, tgfx::Point &uvMin, tgfx::Point &uvMax) const;
    int32_t getUVOffsetIndex(const std::string &styleId) const;
    const std::vector<float> &getUVOffsets() const;  // [uMin, vMin, uMax, vMax] * N
    bool setStyleIdToConfigs(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleConfigs);
};
```

**生成策略**（`generateAtlas()`, `src/SeatCanvas/core/style/SeatStyleAtlasManager.cpp:48`）：

1. 根据座位尺寸 × density 计算每个样式项的像素尺寸
2. 按最多 10 列排列所有样式，计算图集总尺寸
3. 创建 GPU 纹理（`RGBA_8888`, `RENDER_ATTACHMENT | TEXTURE_BINDING`）
4. 创建临时 Surface → Canvas
5. 遍历所有 `styleId → config` 映射，依次渲染到 Canvas 的对应格位
6. 计算每个格位的归一化 UV 坐标，填充 `uvRects`、`uvOffsets`、`styleIdToIndexMap`
7. `context->flushAndSubmit()` 提交 GPU 命令
8. 通过 `onAtlasGenerated` 回调通知外部（触发 CustomSeatPass 更新 UV Offset 数组）

**增量更新**：当 `setStyleIdToConfigs` 检测到样式配置实际发生变化时，会清空图集并触发重新生成。如果样式配置未变化（数量和内容相同），则不触发重建。

### 5.2 SeatStyleRenderer

图集的实际绘制由 `SeatStyleRenderer` 接口完成：

```cpp
class SeatStyleRenderer {
public:
    virtual std::unordered_map<std::string, tgfx::Rect> renderAllSeatStyles(
        tgfx::Canvas *canvas,
        const tgfx::Size &itemSize,
        int columns, int itemSpacing, float density,
        const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig
    ) = 0;
};
```

`CanvasSeatStyleRenderer`（`src/SeatCanvas/core/style/CanvasSeatStyleRenderer.cpp`）是当前唯一实现，支持两种样式类型：

- **CircleSeatStyleConfig**：纯色圆形填充 → `renderCircleStyle()` 使用 `canvas->drawPath(ovalPath, paint)` 绘制圆形；可选叠加层颜色（overlayColor）和勾选标记（checkmarkPath stroke）
- **SVGSeatStyleConfig**：SVG 图片 → `renderSVGStyle()` 解析 SVG 内容为 `tgfx::SVGDOM`，缩放到 itemSize 后 `dom->render(canvas)` 渲染

### 5.3 图集到 Shader 的数据流

1. `SeatStyleAtlasManager::getUVOffsets()` 返回扁平化的 UV 数组（每个样式 4 个 float）
2. `CustomSeatPass::updateUVOffset()` 接收此数组
3. 在 `updateUBOBuffer()` 中，UV 数组被打包进 UBO
4. Vertex Shader 中，`textureCoordIndex` 实例属性被用作索引，从 UBO 的 UV 数组中查找对应范围的偏移
5. Fragment Shader 使用重映射后的 UV 对图集纹理采样

这种**数组化 UV** 的方式避免了为每个样式绑定独立纹理或使用纹理数组（Texture Array），在 OpenGL ES 3.0 环境下有更好的兼容性。

## 6. 坐标系统与 Viewport 管理

### 6.1 三层坐标空间

SeatCanvas 维护三个坐标空间，通过 `SeatCanvasCoreRendererState` 统一管理：

```
原始坐标 (Original)  →  规范化内容坐标 (Normalized Content)  →  屏幕坐标 (Screen)
```

- **原始坐标**：底图 SVG 数据的原始坐标系（`originSize`）
- **规范化内容坐标**：按固定宽度基准缩放后的大小（`normalizedContentSize`），避免超大 SVG 导致过小的缩放级别
- **屏幕坐标**：最终像素坐标（`width × height × density`）

`getMVPMatrix()` 计算从原始坐标到 NDC（归一化设备坐标）的完整变换链：

```cpp
tgfx::Matrix getMVPMatrix() const;  // 原始坐标 → 规范化内容坐标 → 屏幕坐标 → NDC
```

**坐标转换工具函数**（`SeatCanvasCoreRendererState.hpp:74-84`）：

```cpp
inline tgfx::Point ConvertScreenToContent(const tgfx::Point &location,
                                           const tgfx::Point &contentOffset, float scale) {
    return tgfx::Point::Make(
        (location.x - contentOffset.x) / scale,
        (location.y - contentOffset.y) / scale);
}

inline tgfx::Point ConvertContentToScreen(const tgfx::Point &location,
                                           const tgfx::Point &contentOffset, float scale) {
    return tgfx::Point::Make(
        location.x * scale + contentOffset.x,
        location.y * scale + contentOffset.y);
}
```

### 6.2 ViewportController

`ViewportController`（`src/SeatCanvas/core/renderer/ViewportController.{hpp,cpp}`）在 `ElasticZoomPanController` 和 `SeatCanvasCoreRendererState` 之间充当中间层，负责：

- **zoomToRect**：以动画方式缩放到指定矩形区域（EaseInOut 曲线，可配置 durationMs 和 padding）
- **视口事件生成**：`makeViewportEvent()` 快照当前状态为 `SeatCanvasViewportEvent`
- **zoomLevel 管理**：`ZoomLevelConfig` 控制座位、行标签、区域标签、场馆标签分别在什么缩放级别下可见

### 6.3 Seat 可见性控制

座位的显示/隐藏受两级条件控制：

- **shouldAutoDrawSeat()**：首先检查业务方是否通过 `isAutoDrawSeatDisabled` 主动禁用了自动绘制；其次判断 `zoomScale > seatRenderZoomThreshold`
- **seatRenderZoomThreshold**：当 `zoomScale >= threshold` 时座位才被渲染（可通过 API 动态设置）
- **ZoomLevelConfig.seat**：系统内部计算的基础阈值，由底图尺寸和视口尺寸推导

当缩放比例低于阈值时，`prepareSeatIfNeeded` 不会收集任何座位数据，CustomSeatPass 的 `hasData()` 返回 false，从而跳过座位渲染，仅显示底图的"彩虹图"（区域颜色填充），节省大量 GPU 开销。

## 7. 座位数据管理

### 7.1 SeatDataManager

`SeatDataManager`（`src/SeatCanvas/core/renderer/SeatDataManager.{hpp,cpp}`）管理座位的全生命周期数据：

```cpp
class SeatDataManager {
    // 基础数据
    std::unordered_map<std::string, std::vector<kk::SeatData>> _seatDataMap;  // zoneId → seats
    std::unordered_map<std::string, ZoneSeatRuntimeState> _seatStateByZone;   // zoneId → statuses
    std::unordered_map<std::string, SeatLocation> _seatIndexById;             // seatId → (zoneId, index)

    // 价格编码（用于样式匹配）
    std::vector<std::string> _pricecodes;

    // 选中集合
    std::set<std::string> _selectedSeatIds;
};
```

**关键方法**：


| 方法                                            | 语义                                             |
| --------------------------------------------- | ---------------------------------------------- |
| `registerPricecodes(pricecodes)`              | 注册价格编码列表，数组索引即为 `pricecodeIndex`               |
| `setSeatData(zoneId, seats)`                  | 设置区域座位数据（几何 + pricecodeIndex），**重置该区域所有状态为 0** |
| `updateSeatStatuses(updates)`                 | 批量更新座位状态                                       |
| `updateSeatStatusesForZone(zoneId, statuses)` | 按区域更新状态                                        |
| `setSelectedSeatIds(seatIds)`                 | 全量设置选中集合                                       |
| `updateSelectedSeatIds(added, removed)`       | 增量更新选中                                         |


### 7.2 样式键匹配机制

关键约定：样式键由 `SeatRenderStyleId.compose(pricecode, status, selected)` 组成（C++ 侧 `composeSeatStyleId`），必须与通过 `setStyleKeyToConfigFromJSON` 注册的 JSON keys 精确匹配。未注册的 key 会导致对应座位不绘制。

这意味着座位最终的视觉呈现由三个因素决定：

- **pricecodeIndex**：座位所属价格编码（由业务方在 `setSeatData` 时指定）
- **status**：`uint32_t` 状态值，引擎不预设状态枚举，由业务层自定义含义，通过 `updateSeatStatuses` / `updateSeatStatusesForZone` 写入。`setSeatData` 时状态统一重置为 0
- **selected**：是否被选中

## 8. 跨平台设计

### 8.1 平台抽象层

SeatCanvas 的核心 C++ 代码（约 180+ 源文件）完全平台无关。平台差异通过以下抽象接口隔离：

**PlatformView**（`src/SeatCanvas/core/renderer/PlatformView.hpp`）：封装 GPU 窗口/表面的创建和管理：


| 平台      | 实现文件                      | GPU 后端                    |
| ------- | ------------------------- | ------------------------- |
| iOS     | `IOSPlatformView.mm`      | Metal（通过 MTKView）         |
| Android | `AndroidPlatformView.cpp` | OpenGL ES（通过 TextureView） |
| OHOS    | `OHOSPlatformView.cpp`    | OpenGL ES（通过 XComponent）  |


**DisplayLink**（`src/SeatCanvas/core/utils/DisplayLink.hpp`）：帧刷新驱动：


| 平台      | 实现                            | 文件                      |
| ------- | ----------------------------- | ----------------------- |
| iOS     | CADisplayLink                 | `NativeDisplayLink.h`   |
| Android | Choreographer / ValueAnimator | `NativeDisplayLink.cpp` |
| OHOS    | VSync callback                | `NativeDisplayLink.cpp` |


**Platform**（`src/SeatCanvas/core/Platform.{hpp,cpp}`）：平台工具方法（媒体时间戳、线程 ID 等）。

### 8.2 三端桥接方式


| 平台      | 桥接技术                     | 关键文件                                                                                    |
| ------- | ------------------------ | --------------------------------------------------------------------------------------- |
| iOS     | Swift + Objective-C++ 混合 | `swift/SeatCanvasCoreRendererBridge.h`, `swift/SwiftSeatCanvasCoreRendererDelegate.hpp` |
| Android | JNI                      | `android/jni/JNILoader.cpp`（81个 JNI 函数）                                                 |
| OHOS    | NAPI                     | `ohos/JRendererCore.cpp`（93个 NAPI 函数）                                                   |


每端的桥接层负责：

1. 将平台手势事件转换为 C++ 的 `GestureState` + 坐标
2. 将 C++ 的 delegate 回调转发回平台语言
3. 数据类型转换（如 Android 的 `JRect`、`JSeatData`、`JZoomLevel`）

### 8.3 Swift 端示例

iOS 端的 Swift API 通过内部可见的桥接模块暴露：

```swift
@MainActor
final class SeatCanvasCoreRenderer {
    func loadBaseMap(_ data: Data?, format: BaseMapFormat, parseConfigJSON: Data?)
    func applySeatStyleJSONConfig(_ json: Data?)
    func handTap(_ location: CGPoint)

    var seatRenderZoomThreshold: CGFloat { get set }
    var zoomScale: CGFloat { get }
    var minimumZoomScale: CGFloat { get }
    var maximumZoomScale: CGFloat { get }
    var zoomLevel: ZoomLevel { get }
    var debugHUDEnabled: Bool { get set }
}
```

## 9. 性能优化策略

### 9.1 实例化渲染

CustomSeatPass 的核心优化：一次 Draw Call 渲染所有同区域座位。每个座位的差异化数据（位置、旋转、UV）存储在实例缓冲区中，由 GPU 在 Vertex Shader 中并行处理。对于 10,000 个座位，这意味着 10,000 次传统 Draw Call → 1 次 Draw Call。

### 9.2 纹理图集

所有座位样式被合并到一张图集中，避免运行时纹理切换。Shader 通过 `textureCoordIndex` 从 UV 数组中查找对应范围，在一个 RenderPass 中绘制所有不同样式的座位。

### 9.3 按需加载

座位数据的加载受可见性驱动：

- CPU 侧：`getVisibleZoneMesheIndices()` 先做包围盒裁剪，只提交可见区域的顶点
- 缩放级别低于阈值时，直接隐藏所有座位，只显示底图
- `visibleOriginalRect.outset(seatSize * 2, seatSize * 2)` 提供小幅扩展，避免边缘座位闪烁

### 9.4 单 VBO 设计

BaseMapMeshBuilder 将所有区域的顶点合并到单个 VBO，通过 `drawRanges` 数组记录每个区域的偏移和数量。绘制时 `renderFill()` 通过 `draw(Triangles, vertexCount, 1, vertexOffset)` 直接从全量 VBO 中按偏移量读取子集，连续区域合并为一次调用，避免多次 VBO 绑定和 draw call 开销。

### 9.5 惰性图集重建

`SeatStyleAtlasManager::setStyleIdToConfigs()` 在设置新样式配置时会逐项比较，如果配置内容无变化则跳过重建。同样，`update()` 只在 seatSize 或 density 变化时才重新生成。

### 9.6 DisplayList 脏区域追踪

覆盖图层（SeatOverlayLayerTree）使用 `tgfx::DisplayList` 的脏区域追踪，只重绘变化部分。

### 9.7 DisplayLink 按需启停

`ElasticZoomPanController::handleDisplayLinkFire()` 返回是否需要继续刷新。当所有动画（惯性滑动、回弹）都结束时返回 `false`，平台层据此停止 DisplayLink，降低 CPU 占用和功耗。DisplayLink 的 start/stop 实现了幂等性保护，重复调用不会产生副作用。

## 10. 总结

SeatCanvas 的架构设计体现了移动端 GPU 渲染的几个关键原则：

1. **减少 Draw Call**：通过实例化渲染和单 VBO 合并，将大量座位的绘制开销降到最低
2. **纹理图集 + UV 数组**：用内存换纹理切换，用 Shader 中的索引查找替代动态纹理绑定
3. **CPU 侧裁剪先行**：在提交 GPU 命令前完成可见性判断，减少无用渲染
4. **弹性手势系统**：rubber-band 阻尼 + 临界阻尼弹簧 = 自然流畅的交互体验
5. **C++ 核心 + 薄桥接**：一套核心代码，三端复用的跨平台策略

这些设计使得 SeatCanvas 能够在移动设备有限的 GPU 资源下，流畅渲染包含上万个座位的场馆地图，同时保持 60fps 和自然的交互手感。

---

*本文基于 [SeatCanvas](https://github.com/0x1306a94/seatcanvas) 仓库 `develop` 分支代码（截止 2026-05-25）。*