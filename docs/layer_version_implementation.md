# SeatCanvas Layer 版本实现方案

## 概述

SeatCanvas Layer 版本是基于 `tgfx::Layer` 架构实现的座位图渲染方案。该方案充分利用了 tgfx 图形库的 Layer 系统和 DisplayList 渲染机制，实现了高性能、可扩展的座位图渲染引擎。

## 架构设计

### 核心组件

#### 1. SeatLayerTree（座位图层树）

`SeatLayerTree` 是 Layer 版本的核心渲染组件，负责管理整个座位图的图层树结构。

**位置**: `src/SeatCanvas/core/drawers/SeatLayerTree.hpp`

**主要职责**:
- 管理底图图层（BaseMapRootLayer）
- 管理座位图层树（SeatRootLayer）
- 管理座位区域容器（SeatAtlasLayer）
- 使用 DisplayList 进行渲染
- 处理坐标转换和命中测试

**关键特性**:
- **分块渲染支持**: 通过 `enableTiled()` 启用分块渲染模式，提升大场景渲染性能
- **缩放模糊优化**: 通过 `enableZoomBlur()` 启用缩放时的模糊效果优化
- **按需加载**: 支持按需加载座位数据，减少内存占用

#### 2. DisplayList（显示列表）

`DisplayList` 是 tgfx 提供的渲染优化机制，用于记录和优化图层渲染。

**使用方式**:
```cpp
_displayList->setRenderMode(_enableTiled ? tgfx::RenderMode::Tiled : tgfx::RenderMode::Partial);
_displayList->setZoomScale(zoomScale);
_displayList->setContentOffset(contentOffset.x, contentOffset.y);
_displayList->render(surface, false);
```

**渲染模式**:
- `RenderMode::Tiled`: 分块渲染模式，适合大场景
- `RenderMode::Partial`: 部分渲染模式，只渲染变化区域
- `RenderMode::Direct`: 直接渲染模式，不使用优化

### 图层层次结构

```
Root Layer (tgfx::Layer)
├── BaseMapRootLayer (底图根节点)
│   ├── SVG 图层树
│   └── SeatRegionLayer (区域图层)
│       └── SeatTextLayer (区域文本图层)
└── SeatRootLayer (座位根容器)
    └── SeatAtlasLayer (座位区域容器)
        └── SeatItemLayer (座位项图层)
```

### 自定义图层类型

项目定义了以下自定义图层类型（`CustomLayerType`）：

```cpp
enum class CustomLayerType {
    BaseMapRoot = static_cast<int>(tgfx::LayerType::Solid) + 100,  // 底图根节点
    Region = static_cast<int>(tgfx::LayerType::Solid) + 101,        // 区域
    RegionName = static_cast<int>(tgfx::LayerType::Solid) + 102,   // 区域名称
    SeatRoot = static_cast<int>(tgfx::LayerType::Solid) + 200,     // 座位根容器
    SeatAtlas = static_cast<int>(tgfx::LayerType::Solid) + 201,    // 座位区域容器
    Seat = static_cast<int>(tgfx::LayerType::Solid) + 202,         // 座位
};
```

## 核心图层实现

### 1. BaseMapRootLayer（底图根图层）

**位置**: `src/SeatCanvas/core/layers/BaseMapRootLayer.hpp`

**继承**: `tgfx::ShapeLayer`

**功能**:
- 作为 SVG 底图的根节点
- 包含整个底图的路径和变换矩阵
- 支持区域信息的查询和命中测试

**创建方式**:
```cpp
auto baseMapLayer = kk::layer::BaseMapRootLayer::Make();
```

### 2. SeatRootLayer（座位根图层）

**位置**: `src/SeatCanvas/core/layers/SeatRootLayer.hpp`

**继承**: `tgfx::ShapeLayer`

**功能**:
- 作为所有座位图层的根容器
- 继承底图的路径和变换矩阵，确保座位与底图对齐
- 管理所有 SeatAtlasLayer 子图层

**创建方式**:
```cpp
_seatRootLayer = kk::layer::SeatRootLayer::Make();
_seatRootLayer->setPath(baseMapLayer->path());
_seatRootLayer->setMatrix(baseMapLayer->matrix());
```

### 3. SeatAtlasLayer（座位区域容器图层）

**位置**: `src/SeatCanvas/core/layers/SeatAtlasLayer.hpp`

**继承**: `tgfx::ShapeLayer`

**功能**:
- 按区域组织座位图层
- 管理同一区域内的所有 SeatItemLayer
- 支持按需创建和移除座位图层

**关键方法**:
```cpp
// 获取或创建座位图层
std::shared_ptr<SeatItemLayer> getSeatLayerOrCreate(
    const std::string &seatId, 
    std::function<void(SeatItemLayer *)> onCreate = nullptr
);

// 移除座位图层
void removeSeat(const std::string &seatId);

// 隐藏所有座位
void hiddenAllSeat();
```

### 4. SeatItemLayer（座位项图层）

**位置**: `src/SeatCanvas/core/layers/SeatItemLayer.hpp`

**继承**: `tgfx::Layer`

**功能**:
- 表示单个座位的图层
- 支持圆形和图片两种渲染样式
- 支持座位状态（可用、锁定、禁用、已售等）
- 支持选中状态显示

**座位样式**:
- `SeatShapeStyle::Circle`: 圆形座位，使用颜色填充
- `SeatShapeStyle::Image`: 图片座位，使用 SVG 或位图

**座位状态**:
- `SeatStatus::Available`: 可用（红色）
- `SeatStatus::Locked`: 锁定（灰色）
- `SeatStatus::Disabled`: 禁用（深灰色）
- `SeatStatus::Sold`: 已售（浅灰色）

**内容更新**:
```cpp
void SeatItemLayer::onUpdateContent(tgfx::LayerRecorder *recorder) {
    switch (_shapeStyle) {
        case kk::SeatShapeStyle::Circle:
            onUpdateShape(recorder);  // 绘制圆形
            break;
        case kk::SeatShapeStyle::Image:
            onUpdateImage(recorder);   // 绘制图片
            break;
    }
}
```

**圆形座位渲染**:
```cpp
void SeatItemLayer::onUpdateShape(tgfx::LayerRecorder *recorder) {
    tgfx::LayerPaint paint;
    paint.color = getColorByStatus(_seatSatus);  // 根据状态设置颜色
    
    tgfx::Path ovalPath;
    ovalPath.addOval(tgfx::Rect::MakeWH(_seatSize.width, _seatSize.height));
    recorder->addPath(ovalPath, paint);
    
    // 选中状态：绘制半透明覆盖层和勾选标记
    if (_selected) {
        // ... 绘制选中效果
    }
}
```

**图片座位渲染**:
```cpp
void SeatItemLayer::onUpdateImage(tgfx::LayerRecorder *recorder) {
    auto scaleX = _seatSize.width / static_cast<float>(_image->width());
    auto scaleY = _seatSize.height / static_cast<float>(_image->height());
    
    tgfx::SamplingOptions sampling{tgfx::FilterMode::Linear, tgfx::MipmapMode::Linear};
    auto shader = tgfx::Shader::MakeImageShader(_image, ...);
    shader = shader->makeWithMatrix(tgfx::Matrix::MakeScale(scaleX, scaleY));
    
    auto rect = tgfx::Rect::MakeWH(_seatSize.width, _seatSize.height);
    recorder->addRect(rect, tgfx::LayerPaint(std::move(shader)));
}
```

### 5. SeatRegionLayer（区域图层）

**位置**: `src/SeatCanvas/core/layers/SeatRegionLayer.hpp`

**功能**:
- 表示底图中的区域（如座位区、舞台区等）
- 支持区域名称显示
- 用于区域查询和命中测试

### 6. SeatTextLayer（文本图层）

**位置**: `src/SeatCanvas/core/layers/SeatTextLayer.hpp`

**功能**:
- 显示区域名称或其他文本信息
- 基于 tgfx::TextLayer 实现

## SVG 底图转换

### ConvertSVGLayer

**位置**: `src/SeatCanvas/core/svg/ConvertSVGLayer.hpp`

**功能**: 将 SVG DOM 转换为 Layer 树结构

**转换流程**:
1. 解析 SVG DOM
2. 遍历 SVG 节点树
3. 将 SVG 元素转换为对应的 Layer：
   - `SVGCircle` → `SeatRegionLayer`
   - `SVGEllipse` → `SeatRegionLayer`
   - `SVGPath` → `SeatRegionLayer`
   - `SVGRect` → `SeatRegionLayer`
   - `SVGPoly` → `SeatRegionLayer`
   - `SVGText` → `SeatTextLayer`
   - `SVGGroup` → `tgfx::Layer`（容器图层）

**转换选项**:
```cpp
struct ConvertSVGLayerOptions {
    bool collectRegionInfo{false};  // 是否收集区域信息
    bool supportText{false};         // 是否支持文本图层
};
```

**转换结果**:
```cpp
struct ConvertSVGLayerResult {
    std::shared_ptr<BaseMapRootLayer> layer;      // 底图根图层
    tgfx::Size size;                               // 底图尺寸
    std::shared_ptr<BaseMapLayerManager> layerManager;  // 图层管理器
};
```

## 渲染流程

### 1. 初始化阶段

```cpp
// 创建 SeatLayerTree
_seatLayer = std::make_unique<kk::drawers::SeatLayerTree>();

// 设置底图图层
_seatLayer->setBaseMapLayer(baseMapLayer, baseMapSize);
```

### 2. 准备阶段（prepare）

```cpp
void SeatLayerTree::prepare(
    tgfx::Canvas *canvas, 
    const SeatCanvasCoreRendererState *state, 
    bool force
) {
    // 预构建座位状态图片（如果需要）
    prebuildSeatStatusImage(canvas, state);
    
    // 更新内容尺寸或强制重建时，重新构建图层树
    if (updateContentSize(state) || force || _rebuildLayer) {
        buildLayerTree(canvas, state);
    }
    
    // 更新 DisplayList 的缩放和偏移
    auto zoomScale = state->zoomScale();
    auto contentOffset = state->contentOffset();
    _displayList->setZoomScale(zoomScale);
    _displayList->setContentOffset(contentOffset.x, contentOffset.y);
}
```

### 3. 构建图层树（buildLayerTree）

```cpp
void SeatLayerTree::buildLayerTree(
    tgfx::Canvas *canvas, 
    const SeatCanvasCoreRendererState *state
) {
    // 创建根图层
    _root = tgfx::Layer::Make();
    
    // 创建座位根图层
    _seatRootLayer = kk::layer::SeatRootLayer::Make();
    _seatRootLayer->setPath(_baseMapLayer->path());
    _seatRootLayer->setMatrix(_baseMapLayer->matrix());
    
    // 添加底图图层（图片或 SVG）
    if (_baseMapImage) {
        auto imageLayer = tgfx::ImageLayer::Make();
        imageLayer->setImage(_baseMapImage);
        imageLayer->setMatrix(calculateFitScale());
        _root->addChild(imageLayer);
    } else {
        _root->addChild(_baseMapLayer);
    }
    
    // 添加座位根图层
    _root->addChild(_seatRootLayer);
    
    // 更新根图层的变换矩阵
    updateRootMatrix(canvas, state);
    
    // 添加到 DisplayList
    _displayList->root()->addChild(_root);
}
```

### 4. 绘制阶段（draw）

```cpp
void SeatLayerTree::onDraw(
    tgfx::Canvas *canvas, 
    const SeatCanvasCoreRendererState *state
) {
    // 按需加载座位数据
    loadSeatIfneeded(canvas, state);
    
    // 使用 DisplayList 渲染
    auto surface = canvas->getSurface();
    _displayList->render(surface, false);
}
```

## 座位数据管理

### 按需加载机制

座位数据采用按需加载机制，只加载可见区域的座位：

```cpp
void SeatCanvasCoreRenderer::drawSeatIfNeeded(tgfx::Canvas *canvas) {
    // 1. 检查缩放级别，过小则隐藏所有座位
    if (zoomScale < _zoomLevelConfig.zoomScale50) {
        _seatLayer->hiddenSeatAtlas();
        return;
    }
    
    // 2. 获取可见内容区域
    auto visibleContentRect = getVisibleContentRect();
    visibleContentRect.outset(60, 60);  // 扩大一点，避免边缘闪烁
    
    // 3. 查找与可见区域相交的区域
    auto regions = layerManager->findRegionsIntersectingRect(visibleContentRect);
    
    // 4. 为每个区域创建或更新座位图层
    for (const auto &region : regions) {
        auto atlas = _seatLayer->getSeatAtlasLayerOrCreate(region->regionId);
        
        // 遍历该区域的座位数据
        for (const auto &seatInfo : seatInfos) {
            // 检查座位是否在可见区域内
            if (!visibleContentRect.contains(seatInfo.rect)) {
                continue;
            }
            
            // 创建或更新座位图层
            auto seatLayer = atlas->getSeatLayerOrCreate(seatInfo.seatId, [&](SeatItemLayer *layer) {
                layer->setPosition({seatInfo.rect.x(), seatInfo.rect.y()});
                layer->setSeatSize({seatInfo.rect.width(), seatInfo.rect.height()});
            });
            
            seatLayer->setSeatSatus(seatInfo.status);
        }
    }
    
    // 5. 更新可见性
    _seatLayer->hiddenSeatAtlas(seatAtlasVisibleMap);
}
```

### 座位图层创建

```cpp
std::shared_ptr<SeatAtlasLayer> SeatLayerTree::getSeatAtlasLayerOrCreate(
    const std::string &regionId,
    std::function<void(SeatAtlasLayer *)> onCreate
) {
    // 查找或创建 SeatAtlasLayer
    auto iter = _seatAtlasLayers.find(regionId);
    if (iter == _seatAtlasLayers.end()) {
        auto layer = kk::layer::SeatAtlasLayer::Make();
        if (onCreate) {
            onCreate(layer.get());
        }
        _seatAtlasLayers[regionId] = layer;
        
        // 添加到座位根图层
        if (_seatRootLayer) {
            _seatRootLayer->addChild(layer);
        }
    }
    return _seatAtlasLayers[regionId];
}
```

## 坐标转换

### 全局坐标 ↔ 本地坐标

```cpp
// 全局坐标转本地坐标
tgfx::Point SeatLayerTree::globalToLocal(const tgfx::Point &globalPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    return _root->globalToLocal(globalPoint);
}

// 本地坐标转全局坐标
tgfx::Point SeatLayerTree::localToGlobal(const tgfx::Point &localPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    return _root->localToGlobal(localPoint);
}
```

### 命中测试

```cpp
// 获取指定位置的座位区域ID
std::optional<std::string> SeatLayerTree::getSeatRegionIdAt(float x, float y) const {
    // 1. 粗略查找：使用 bounding box
    auto layers = _baseMapLayer->getLayersUnderPoint(x, y);
    
    // 2. 精确查找：使用 hitTest
    for (const auto &layer : layers) {
        if (layer->type() == CustomLayerType::Region) {
            if (layer->hitTestPoint(x, y, true)) {
                return static_cast<SeatRegionLayer*>(layer)->name();
            }
        }
    }
    return std::nullopt;
}

// 获取指定位置的座位图层
std::shared_ptr<SeatItemLayer> SeatLayerTree::getSeatItemAt(float x, float y) const {
    auto layers = _seatRootLayer->getLayersUnderPoint(x, y);
    for (const auto &layer : layers) {
        if (layer->type() == CustomLayerType::Seat) {
            return std::static_pointer_cast<SeatItemLayer>(layer);
        }
    }
    return nullptr;
}
```

## 性能优化

### 1. DisplayList 渲染优化

- **Partial 渲染模式**: 只渲染变化区域，减少不必要的绘制
- **Tiled 渲染模式**: 分块渲染，适合大场景
- **子树缓存**: 缓存不变的图层子树，避免重复渲染

```cpp
_displayList->setRenderMode(_enableTiled ? tgfx::RenderMode::Tiled : tgfx::RenderMode::Partial);
// _displayList->setSubtreeCacheMaxSize(1024);  // 可选：设置子树缓存大小
```

### 2. 按需加载

- 只加载可见区域的座位数据
- 根据缩放级别动态显示/隐藏座位
- 使用可见性控制减少渲染负担

### 3. 图层复用

- SeatAtlasLayer 和 SeatItemLayer 采用对象池或缓存机制
- 避免频繁创建和销毁图层对象

### 4. 缩放优化

- 小缩放级别时隐藏座位，只显示底图
- 使用 DisplayList 的缩放模糊优化（`enableZoomBlur()`）

### 5. 脏区域标记

DisplayList 自动跟踪图层变化，只渲染脏区域：

```cpp
bool SeatLayerTree::hasContentChanged() const {
    return _displayList->hasContentChanged();
}
```

## 与渲染器的集成

### SeatCanvasCoreRenderer

`SeatCanvasCoreRenderer` 是 SeatCanvas 的核心渲染器，负责协调各个组件：

```cpp
class SeatCanvasCoreRenderer {
private:
    std::unique_ptr<SeatLayerTree> _seatLayer;           // 座位图层树
    std::unique_ptr<SeatOverlayLayerTree> _overlayLayer; // 覆盖图层树（如小地图）
    std::unique_ptr<SeatCanvasCoreRendererState> _state;  // 渲染状态
};
```

### 渲染循环

```cpp
void SeatCanvasCoreRenderer::draw(bool force) {
    auto canvas = surface->getCanvas();
    
    // 1. 准备座位图层
    _seatLayer->prepare(canvas, _state.get(), force);
    
    // 2. 准备覆盖图层
    _overlayLayer->prepare(canvas, _state.get(), force);
    
    // 3. 按需绘制座位数据
    if (_seatLayer->hasContentChanged() || force) {
        drawSeatIfNeeded(canvas);
    }
    
    // 4. 绘制图层
    _seatLayer->draw(canvas, _state.get());
    _overlayLayer->draw(canvas, _state.get());
}
```

## 优势与特点

### 1. 高性能

- 基于 GPU 加速的 tgfx 渲染引擎
- DisplayList 优化机制减少不必要的绘制
- 按需加载减少内存占用

### 2. 可扩展性

- 清晰的图层层次结构
- 易于添加新的图层类型
- 支持自定义渲染逻辑

### 3. 灵活性

- 支持多种座位样式（圆形、图片）
- 支持动态座位状态更新
- 支持区域级别的显示控制

### 4. 跨平台一致性

- 基于 C++ 核心实现
- 各平台使用相同的渲染逻辑
- 确保渲染效果一致

## 使用示例

### 初始化

```cpp
// 1. 创建渲染器
auto renderer = std::make_unique<SeatCanvasCoreRenderer>(
    std::move(backend),
    std::move(zoomPanController)
);

// 2. 加载 SVG 底图
auto svgDom = tgfx::SVGDOM::MakeFromData(svgData);
auto convertResult = kk::svg::convertSVGDomToLayer(svgDom);

// 3. 设置底图
renderer->setBaseMapConfig(baseMapConfig, SeatRenderMode::ZoomBased);

// 4. 启动渲染循环
renderer->start();
```

### 更新座位数据

```cpp
// 座位数据会自动按需加载
// 只需确保数据源提供正确的座位信息即可
```

### 处理交互

```cpp
// 点击处理
renderer->handleTap(location);

// 平移处理
renderer->handlePan(state, translation, timestampMs);

// 缩放处理
renderer->handlePinch(state, scale, center);
```

## 总结

SeatCanvas Layer 版本通过充分利用 tgfx::Layer 架构，实现了高性能、可扩展的座位图渲染方案。该方案具有以下核心优势：

1. **高性能渲染**: DisplayList 优化机制和按需加载策略
2. **清晰的架构**: 分层的图层结构，易于理解和维护
3. **灵活的扩展**: 支持自定义图层类型和渲染逻辑
4. **跨平台一致**: C++ 核心实现确保各平台渲染一致

该实现方案为 SeatCanvas 提供了坚实的基础，支持未来进一步的功能扩展和性能优化。
