//
//  SeatStyleAtlasManager.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatStyleAtlasManager_hpp
#define SeatStyleAtlasManager_hpp

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include "SeatStyleConfig.hpp"
#include "SeatStyleKey.hpp"
#include "SeatStyleRenderer.hpp"
#include "core/renderer/IContextAware.hpp"

#include <cstdint>

namespace tgfx {
class Context;
class Surface;
class Texture;
};  // namespace tgfx

namespace kk::renderer {

/**
 * 座位样式 Atlas 管理器
 * 用于提前预渲染所有座位的状态的样式到一个大的图片中
 */
class SeatStyleAtlasManager : public IContextAware {
  public:
    using AtlasGeneratedCallback = std::function<void(const SeatStyleAtlasManager *)>;

    /**
     * 创建 Atlas 管理器
     * @param styleConfigs 样式配置
     */
    explicit SeatStyleAtlasManager(const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleConfigs = {});
    ~SeatStyleAtlasManager();

    /**
     * 更新
     * @param density 设备密度
     * @param seatSize 座位大小（用于渲染）
     */
    void update(float density, const tgfx::Size &seatSize);

    /**
     * 获取 Atlas 的 Image，用于在着色器中使用
     * 调用者可以使用 Image::makeTextureImage() 来获取纹理
     * @return Image，如果未生成则返回 nullptr
     */
    std::shared_ptr<tgfx::Texture> getAtlasTexture() const {
        return atlasTexture;
    }

    /**
     * 根据座位状态和选中状态获取在 Atlas 中的 UV 坐标
     * @param status 座位状态（由业务层定义）
     * @param selected 是否选中
     * @param uvMin 输出的最小 UV 坐标（左下角）
     * @param uvMax 输出的最大 UV 坐标（右上角）
     * @return 是否成功获取
     */
    bool getUVCoords(uint32_t status, bool selected, tgfx::Point &uvMin, tgfx::Point &uvMax) const;

    /**
     * 根据座位状态和选中状态获取在 UV offset 数组中的索引
     * @param status 座位状态（由业务层定义）
     * @param selected 是否选中
     * @return 索引，如果未找到则返回 -1
     */
    int32_t getUVOffsetIndex(uint32_t status, bool selected) const;

    /**
     * 获取 UV offset 数组（Float4 格式：uvMin.x, uvMin.y, uvMax.x, uvMax.y）
     * @return UV offset 数组的引用
     */
    const std::vector<float> &getUVOffsets() const {
        return uvOffsets;
    }

    /**
     * 获取 UV offset 数组的大小（元素数量，每个样式占 4 个 float）
     */
    size_t getUVOffsetCount() const {
        return uvOffsets.size() / 4;
    }

    /**
     * 获取 Atlas 的宽度
     */
    int getAtlasWidth() const {
        return atlasWidth;
    }

    /**
     * 获取 Atlas 的高度
     */
    int getAtlasHeight() const {
        return atlasHeight;
    }

    /**
     * 设置样式键到配置的映射
     * 业务层通过此映射为不同状态+选中组合提供不同的样式配置（填充颜色、覆盖层颜色、勾选颜色等）
     * @param styleConfigs 样式键到配置的映射
     * @return 是否有变化
     */
    bool setStyleKeyToConfigs(const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleConfigs);

    /**
     * 设置 Atlas 生成成功回调
     * @param callback 回调函数，当 Atlas 成功生成时调用
     */
    void setOnAtlasGenerated(AtlasGeneratedCallback callback) {
        onAtlasGenerated = std::move(callback);
    }

    /**
     * 清除所有缓存，强制重新生成
     */
    void clear();

  private:
    /**
     * 生成 Atlas 纹理
     */
    void generateAtlas();

    /**
     * 获取座位样式渲染器（如果未设置则创建默认的 Canvas 渲染器）
     */
    std::shared_ptr<SeatStyleRenderer> getStyleRenderer();

  protected:
    void onResetGPUResources() override;

  private:
    float density = 1.0f;
    tgfx::Size seatSize = {};

    std::shared_ptr<tgfx::Texture> atlasTexture = {nullptr};

    // 存储每个状态的 UV 坐标（在 Atlas 中的位置）
    std::unordered_map<kk::SeatStyleKey, tgfx::Rect> uvRects = {};

    // UV offset 数组（Float4 格式：每个样式占 4 个 float: uvMin.x, uvMin.y, uvMax.x, uvMax.y）
    std::vector<float> uvOffsets = {};

    // 从 SeatStyleKey 到索引的映射
    std::unordered_map<kk::SeatStyleKey, int32_t> keyToIndexMap = {};

    int atlasWidth = 0;
    int atlasHeight = 0;
    int itemWidth = 0;
    int itemHeight = 0;
    int itemSpacing = 2;  // 座位之间的间距

    std::shared_ptr<SeatStyleRenderer> styleRenderer = {nullptr};

    std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> styleConfigs = {};
    AtlasGeneratedCallback onAtlasGenerated = {nullptr};
};

};  // namespace kk::renderer

#endif /* SeatStyleAtlasManager_hpp */
