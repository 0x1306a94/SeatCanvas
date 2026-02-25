//
//  SeatTextLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatTextLayer.hpp"

#include <tgfx/core/Canvas.h>
#include <tgfx/core/TextBlob.h>
#include <tgfx/layers/LayerRecorder.h>

namespace kk::layer {
std::shared_ptr<SeatTextLayer> SeatTextLayer::Make() {
    return std::shared_ptr<SeatTextLayer>(new SeatTextLayer());
}

void SeatTextLayer::setTextBlob(std::shared_ptr<tgfx::TextBlob> textBlob) {
    if (_textBlob == textBlob) {
        return;
    }
    _textBlob = textBlob;
    invalidateContent();
}

void SeatTextLayer::setTextColor(const tgfx::Color &color) {
    if (_textColor == color) {
        return;
    }
    _textColor = color;
    invalidateContent();
}

void SeatTextLayer::setStrokeColor(const tgfx::Color &color) {
    if (_strokeColor == color) {
        return;
    }
    _strokeColor = color;
    invalidateContent();
}

void SeatTextLayer::setTextAlign(tgfx::TextAlign align) {
    if (_textAlign == align) {
        return;
    }
    _textAlign = align;
    invalidateContent();
}

void SeatTextLayer::setLineCap(tgfx::LineCap cap) {
    if (_stroke.cap == cap) {
        return;
    }
    _stroke.cap = cap;
    invalidateContent();
}

void SeatTextLayer::setLineJoin(tgfx::LineJoin join) {
    if (_stroke.join == join) {
        return;
    }
    _stroke.join = join;
    invalidateContent();
}

void SeatTextLayer::setMiterLimit(float limit) {
    if (_stroke.miterLimit == limit) {
        return;
    }
    _stroke.miterLimit = limit;
    invalidateContent();
}

void SeatTextLayer::setLineWidth(float width) {
    if (_stroke.width == width) {
        return;
    }
    _stroke.width = width;
    invalidateContent();
}

float SeatTextLayer::getAlignmentFactor() const {
    switch (_textAlign) {
        case tgfx::TextAlign::Start:
            return 0.0f;
        case tgfx::TextAlign::Center:
            return -0.5f;
        case tgfx::TextAlign::End:
            return -1.0f;
        case tgfx::TextAlign::Justify:
            return 0.0f;
    }
}

void SeatTextLayer::onUpdateContent(tgfx::LayerRecorder *recorder) {
    if (_textBlob == nullptr) {
        return;
    }

    auto bounds = _textBlob->getTightBounds();
    auto tx = getAlignmentFactor() * bounds.width();

    tgfx::LayerPaint paint{_textColor};
    recorder->addTextBlob(_textBlob, paint, tx, 0);

    if (_stroke.width > 0.0f && _strokeColor) {
        tgfx::LayerPaint paint{_strokeColor.value()};
        paint.style = tgfx::PaintStyle::Stroke;
        paint.stroke = _stroke;
        recorder->addTextBlob(_textBlob, paint, tx, 0);
    }
}

};  // namespace kk::layer
