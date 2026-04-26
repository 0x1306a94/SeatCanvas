//
//  SeatStyleAtlasManager.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatStyleAtlasManager_hpp
#define SeatStyleAtlasManager_hpp

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include "SeatStyleConfig.hpp"
#include "SeatStyleRenderer.hpp"
#include "core/renderer/IContextAware.hpp"

namespace tgfx {
class Context;
class Surface;
class Texture;
};  // namespace tgfx

namespace kk::renderer {

class SeatStyleAtlasManager : public IContextAware {
  public:
    using AtlasGeneratedCallback = std::function<void(const SeatStyleAtlasManager *)>;

    explicit SeatStyleAtlasManager(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleConfigs = {});
    ~SeatStyleAtlasManager();

    void update(float density, const tgfx::Size &seatSize);

    std::shared_ptr<tgfx::Texture> getAtlasTexture() const {
        return atlasTexture;
    }

    bool getUVCoords(const std::string &styleId, tgfx::Point &uvMin, tgfx::Point &uvMax) const;

    int32_t getUVOffsetIndex(const std::string &styleId) const;

    const std::vector<float> &getUVOffsets() const {
        return uvOffsets;
    }

    size_t getUVOffsetCount() const {
        return uvOffsets.size() / 4;
    }

    int getAtlasWidth() const {
        return atlasWidth;
    }

    int getAtlasHeight() const {
        return atlasHeight;
    }

    bool setStyleIdToConfigs(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleConfigs);

    void setOnAtlasGenerated(AtlasGeneratedCallback callback) {
        onAtlasGenerated = std::move(callback);
    }

    void clear();

  private:
    void generateAtlas();
    std::shared_ptr<SeatStyleRenderer> getStyleRenderer();

  protected:
    void onResetGPUResources() override;

  private:
    float density = 1.0f;
    tgfx::Size seatSize = {};

    std::shared_ptr<tgfx::Texture> atlasTexture = {nullptr};

    std::unordered_map<std::string, tgfx::Rect> uvRects = {};
    std::vector<float> uvOffsets = {};
    std::unordered_map<std::string, int32_t> styleIdToIndexMap = {};

    int atlasWidth = 0;
    int atlasHeight = 0;
    int itemWidth = 0;
    int itemHeight = 0;
    int itemSpacing = 2;

    std::shared_ptr<SeatStyleRenderer> styleRenderer = {nullptr};

    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> styleConfigs = {};
    AtlasGeneratedCallback onAtlasGenerated = {nullptr};
};

};  // namespace kk::renderer

#endif /* SeatStyleAtlasManager_hpp */
