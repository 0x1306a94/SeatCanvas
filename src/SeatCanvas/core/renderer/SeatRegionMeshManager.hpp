//
//  SeatRegionMeshManager.hpp
//  SeatCanvas
//
//  Created by KK on 2025/12/21.
//

#ifndef SeatRegionMeshManager_hpp
#define SeatRegionMeshManager_hpp

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "IContextAware.hpp"

namespace tgfx {
class Context;
};

namespace kk::renderer {
struct SeatRegionMesh;
class SeatRegionMeshManager : public IContextAware, public std::enable_shared_from_this<SeatRegionMeshManager> {
  public:
    using SeatRegionMeshBuilder = std::function<std::shared_ptr<SeatRegionMesh>(tgfx::Context *context, const std::string &regionId)>;
    explicit SeatRegionMeshManager(SeatRegionMeshBuilder meshBuilder);

    std::vector<std::shared_ptr<SeatRegionMesh>> getMesh(const std::unordered_set<std::string> &regionIds);

    std::shared_ptr<SeatRegionMesh> getMeshOrCreate(const std::string &regionId);

    std::shared_ptr<SeatRegionMesh> getMesh(const std::string &regionId) const;

    void clear(const std::unordered_set<std::string> &regionIds);
    void clearAll();

  protected:
    void onResetGPUResources() override;

  private:
    std::shared_ptr<SeatRegionMesh> buildMesh(tgfx::Context *context, const std::string &regionId);

  private:
    SeatRegionMeshBuilder meshBuilder = {nullptr};
    std::unordered_map<std::string, std::shared_ptr<SeatRegionMesh>> meshMap = {};
};
};  // namespace kk::renderer

#endif /* SeatRegionMeshManager_hpp */
