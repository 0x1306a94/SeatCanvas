//
//  SeatRegionMeshManager.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/21.
//

#include "SeatRegionMeshManager.hpp"

namespace kk::renderer {

SeatRegionMeshManager::SeatRegionMeshManager(SeatRegionMeshBuilder meshBuilder)
    : meshBuilder(std::move(meshBuilder)) {
}

std::vector<std::shared_ptr<SeatRegionMesh>> SeatRegionMeshManager::getMesh(const std::unordered_set<std::string> &regionIds) {

    std::vector<std::shared_ptr<SeatRegionMesh>> result{};
    for (const auto &regionId : regionIds) {
        auto mesh = getMeshOrCreate(regionId);
        if (mesh) {
            result.push_back(mesh);
            meshMap.insert_or_assign(regionId, mesh);
        }
    }

    return result;
}

std::shared_ptr<SeatRegionMesh> SeatRegionMeshManager::getMeshOrCreate(const std::string &regionId) {
    if (regionId.empty()) {
        return nullptr;
    }
    auto iter = meshMap.find(regionId);
    if (iter != meshMap.end()) {
        return iter->second;
    }

    auto mesh = buildMesh(getContext(), regionId);
    return mesh;
}

std::shared_ptr<SeatRegionMesh> SeatRegionMeshManager::getMesh(const std::string &regionId) const {
    if (regionId.empty()) {
        return nullptr;
    }
    auto iter = meshMap.find(regionId);
    if (iter != meshMap.end()) {
        return iter->second;
    }

    return nullptr;
}

std::shared_ptr<SeatRegionMesh> SeatRegionMeshManager::buildMesh(tgfx::Context *context, const std::string &regionId) {
    if (!meshBuilder || context == nullptr) {
        return nullptr;
    }

    auto mesh = meshBuilder(context, regionId);
    return mesh;
}

void SeatRegionMeshManager::clear(const std::unordered_set<std::string> &regionIds) {
    for (const auto &regionId : regionIds) {
        auto iter = meshMap.find(regionId);
        if (iter != meshMap.end()) {
            meshMap.erase(iter);
        }
    }
}

void SeatRegionMeshManager::clearAll() {
    meshMap.clear();
}

void SeatRegionMeshManager::onResetGPUResources() {
    clearAll();
}
};  // namespace kk::renderer
