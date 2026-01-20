//
//  SeatTextLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatTextLayer_hpp
#define SeatTextLayer_hpp

#include <optional>

#include <tgfx/core/Stroke.h>
#include <tgfx/layers/Layer.h>
#include <tgfx/layers/TextAlign.h>

#include "CustomLayerType.hpp"

namespace tgfx {
class TextBlob;
};

namespace kk::layer {
class SeatTextLayer : public tgfx::Layer {
  public:
    static std::shared_ptr<SeatTextLayer> Make();

    virtual ~SeatTextLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::ZoneName);
    }

    const std::shared_ptr<tgfx::TextBlob> textBlob() const {
        return _textBlob;
    }

    void setTextBlob(std::shared_ptr<tgfx::TextBlob> textBlob);

    const tgfx::Color &textColor() const {
        return _textColor;
    }

    void setTextColor(const tgfx::Color &color);

    const std::optional<tgfx::Color> &strokeColor() const {
        return _strokeColor;
    }

    void setStrokeColor(const tgfx::Color &color);

    tgfx::TextAlign textAlign() const {
        return _textAlign;
    }

    void setTextAlign(tgfx::TextAlign align);

    tgfx::LineCap lineCap() const {
        return _stroke.cap;
    }

    void setLineCap(tgfx::LineCap cap);

    tgfx::LineJoin lineJoin() const {
        return _stroke.join;
    }

    void setLineJoin(tgfx::LineJoin join);

    float miterLimit() const {
        return _stroke.miterLimit;
    }

    void setMiterLimit(float limit);

    float lineWidth() const {
        return _stroke.width;
    }

    void setLineWidth(float width);

  protected:
    SeatTextLayer() = default;

    float getAlignmentFactor() const;

    void onUpdateContent(tgfx::LayerRecorder *recorder) override;

  private:
    std::shared_ptr<tgfx::TextBlob> _textBlob = {nullptr};
    tgfx::Color _textColor = {tgfx::Color::White()};
    tgfx::TextAlign _textAlign = {tgfx::TextAlign::Left};
    tgfx::Stroke _stroke = {};
    std::optional<tgfx::Color> _strokeColor = {std::nullopt};
};
};  // namespace kk::layer

#endif /* SeatTextLayer_hpp */
