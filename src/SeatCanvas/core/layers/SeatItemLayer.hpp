//
//  SeatItemLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatItemLayer_hpp
#define SeatItemLayer_hpp

#include <tgfx/core/Image.h>
#include <tgfx/layers/Layer.h>

#include "CustomLayerType.hpp"
#include "core/SeatShapeStyle.h"
#include "core/SeatStatus.h"

namespace kk::layer {
class SeatAtlasLayer;
class SeatItemLayer : public tgfx::Layer {
  public:
    static std::shared_ptr<SeatItemLayer> Make();

    virtual ~SeatItemLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::Seat);
    }

    void setSeatSize(const tgfx::Size &size);

    void setSeatImage(std::shared_ptr<tgfx::Image> image);
    void setSeatStyle(kk::SeatShapeStyle style);
    void setSeatSatus(kk::SeatStatus status);
    kk::SeatStatus seatSatus() const;
    bool selected() const;
    void setSelected(bool selected);

    void attachAtlasLayer(std::weak_ptr<SeatAtlasLayer> atlasLayer);
    void dettachAtlasLayer();

  protected:
    SeatItemLayer() = default;

    void onUpdateContent(tgfx::LayerRecorder *recorder) override;

    void onUpdateImage(tgfx::LayerRecorder *recorder);

    void onUpdateShape(tgfx::LayerRecorder *recorder);

  private:
    std::weak_ptr<SeatAtlasLayer> _attachAtlasLayer = {};
    std::shared_ptr<tgfx::Image> _image = {};
    kk::SeatStatus _seatSatus = kk::SeatStatus::Available;
    kk::SeatShapeStyle _shapeStyle = kk::SeatShapeStyle::Circle;
    tgfx::Size _seatSize = {};
    bool _selected = false;
};
};  // namespace kk::layer

#endif /* SeatItemLayer_hpp */
