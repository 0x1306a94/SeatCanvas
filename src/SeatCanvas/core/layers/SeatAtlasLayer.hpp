//
//  SeatAtlasLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatAtlasLayer_hpp
#define SeatAtlasLayer_hpp

#include <functional>

#include <tgfx/layers/ShapeLayer.h>

#include "CustomLayerType.hpp"

namespace kk::layer {
class SeatItemLayer;
class SeatAtlasLayer : public tgfx::ShapeLayer {
  public:
    static std::shared_ptr<SeatAtlasLayer> Make();

    virtual ~SeatAtlasLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::SeatAtlas);
    }

    std::shared_ptr<SeatItemLayer> getSeatLayerOrCreate(const std::string &seatId, std::function<void(SeatItemLayer *)> onCreate = nullptr);

    void removeSeat(const std::string &seatId);

    void hiddenAllSeat();

  protected:
    SeatAtlasLayer() = default;

  private:
    std::unordered_map<std::string, std::shared_ptr<SeatItemLayer>> _seatLayers{};
};
};  // namespace kk::layer

#endif /* SeatAtlasLayer_hpp */
