//
//  SeatAtlasLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatAtlasLayer.hpp"

#include "SeatItemLayer.hpp"
namespace kk::layer {
std::shared_ptr<SeatAtlasLayer> SeatAtlasLayer::Make() {
    return std::shared_ptr<SeatAtlasLayer>(new SeatAtlasLayer());
}

std::shared_ptr<SeatItemLayer> SeatAtlasLayer::getSeatLayerOrCreate(const std::string &seatId, std::function<void(SeatItemLayer *)> onCreate) {
    if (seatId.empty()) {
        return nullptr;
    }

    auto iter = _seatLayers.find(seatId);
    std::shared_ptr<SeatItemLayer> layer = nullptr;

    if (iter != _seatLayers.end()) {
        layer = iter->second;
    }

    if (!layer) {
        layer = SeatItemLayer::Make();
        if (onCreate) {
            onCreate(layer.get());
        }
        _seatLayers.insert_or_assign(seatId, layer);
    }

    if (layer->parent() == nullptr) {
        layer->attachAtlasLayer(std::static_pointer_cast<SeatAtlasLayer>(shared_from_this()));
        this->addChild(layer);
    }

    return layer;
}

void SeatAtlasLayer::removeSeat(const std::string &seatId) {
    if (seatId.empty()) {
        return;
    }

    auto iter = _seatLayers.find(seatId);
    if (iter != _seatLayers.end()) {
        return;
    }

    if (iter->second) {
        iter->second->dettachAtlasLayer();
        iter->second->removeFromParent();
    }

    _seatLayers.erase(iter);
}

void SeatAtlasLayer::hiddenAllSeat() {
    for (auto &[key, value] : _seatLayers) {
        if (value) {
            value->setVisible(false);
        }
    }
}

};  // namespace kk::layer
