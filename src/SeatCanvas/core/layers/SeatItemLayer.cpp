//
//  SeatItemLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatItemLayer.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/core/Image.h>
#include <tgfx/core/Shader.h>

namespace kk::layer {
std::shared_ptr<SeatItemLayer> SeatItemLayer::Make() {
    return std::shared_ptr<SeatItemLayer>(new SeatItemLayer());
}

void SeatItemLayer::setSeatSize(const tgfx::Size &size) {
    if (_seatSize == size) {
        return;
    }
    _seatSize = size;
    invalidateContent();
}

void SeatItemLayer::setSeatImage(std::shared_ptr<tgfx::Image> image) {
    if (_image == image) {
        return;
    }

    _image = std::move(image);
    invalidateContent();
}

void SeatItemLayer::setSeatStyle(kk::SeatShapeStyle style) {
    if (_shapeStyle == style) {
        return;
    }

    _shapeStyle = style;
    invalidateContent();
}

void SeatItemLayer::setSeatSatus(kk::SeatStatus status) {
    if (_seatSatus == status) {
        return;
    }

    _seatSatus = status;
    invalidateContent();
}

kk::SeatStatus SeatItemLayer::seatSatus() const {
    return _seatSatus;
}

bool SeatItemLayer::selected() const {
    return _selected;
}

void SeatItemLayer::setSelected(bool selected) {
    if (_selected == selected) {
        return;
    }
    _selected = selected;
    invalidateContent();
}

void SeatItemLayer::attachAtlasLayer(std::weak_ptr<SeatAtlasLayer> atlasLayer) {
    _attachAtlasLayer = std::move(atlasLayer);
}

void SeatItemLayer::dettachAtlasLayer() {
    _attachAtlasLayer.reset();
}

void SeatItemLayer::onUpdateContent(tgfx::LayerRecorder *recorder) {
    switch (_shapeStyle) {
        case kk::SeatShapeStyle::Circle:
            onUpdateShape(recorder);
            break;
        case kk::SeatShapeStyle::Image:
            onUpdateImage(recorder);
            break;
        default:
            break;
    }

    // 必须添加一个形状，否则 hitTest 无法检测
    //    auto rect = tgfx::Rect::MakeWH(_seatSize.width, _seatSize.height);
    //    tgfx::Path path;
    //    path.addRect(rect);
    //    auto shape = tgfx::Shape::MakeFrom(path);
    //    recorder->addShape(shape, tgfx::LayerPaint(tgfx::Color::Transparent()));
    //    onUpdateImage(recorder);
}

void SeatItemLayer::onUpdateImage(tgfx::LayerRecorder *recorder) {
    if (!_image || (_seatSize.width <= 0.0 || _seatSize.height <= 0.0)) {
        return;
    }

    auto scaleX = _seatSize.width / static_cast<float>(_image->width());
    auto scaleY = _seatSize.height / static_cast<float>(_image->height());

    tgfx::SamplingOptions sampling{tgfx::FilterMode::Linear, tgfx::MipmapMode::Linear};
    auto shader = tgfx::Shader::MakeImageShader(_image, tgfx::TileMode::Clamp, tgfx::TileMode::Clamp, sampling);
    if (!shader) {
        return;
    }
    shader = shader->makeWithMatrix(tgfx::Matrix::MakeScale(scaleX, scaleY));
    auto rect = tgfx::Rect::MakeWH(_seatSize.width, _seatSize.height);
    recorder->addRect(rect, tgfx::LayerPaint(std::move(shader)));
}

void SeatItemLayer::onUpdateShape(tgfx::LayerRecorder *recorder) {
    if (_seatSize.width <= 0.0 || _seatSize.height <= 0.0) {
        return;
    }

    tgfx::LayerPaint paint;
    switch (_seatSatus) {
        case kk::SeatStatus::Available: {
            paint.color = tgfx::Color::Red();
            break;
        }
        case kk::SeatStatus::Locked: {
            paint.color = tgfx::Color::FromRGBA(0x99, 0x99, 0x99, 0xff);
            break;
        }
        case kk::SeatStatus::Disabled: {
            paint.color = tgfx::Color::FromRGBA(0x66, 0x66, 0x66, 0xff);
            break;
        }
        case kk::SeatStatus::Sold: {
            paint.color = tgfx::Color::FromRGBA(0xaa, 0xaa, 0xaa, 0xff);
            break;
        }
        default:
            break;
    }

    tgfx::Path ovalPath;
    ovalPath.addOval(tgfx::Rect::MakeWH(_seatSize.width, _seatSize.height));
    recorder->addPath(ovalPath, paint);

    if (_selected) {
        paint.color = tgfx::Color{0.0f, 0.0f, 0.0f, 0.7f};
        recorder->addPath(ovalPath, paint);

        tgfx::Path strokePath;
        strokePath.moveTo(11.0f, 17.0f);
        strokePath.lineTo(16.0f, 22.0f);
        strokePath.lineTo(24.0f, 13.0f);

        paint.color = tgfx::Color::White();
        paint.style = tgfx::PaintStyle::Stroke;
        paint.stroke = tgfx::Stroke{4.0f, tgfx::LineCap::Round, tgfx::LineJoin::Round};
        recorder->addPath(strokePath, paint);
    }
}
};  // namespace kk::layer
