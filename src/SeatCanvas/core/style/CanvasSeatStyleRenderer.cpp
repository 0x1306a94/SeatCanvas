//
//  CanvasSeatStyleRenderer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "CanvasSeatStyleRenderer.hpp"

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Color.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Stream.h>
#include <tgfx/gpu/Context.h>
#include <tgfx/svg/SVGDOM.h>

#include "CircleSeatStyleConfig.hpp"
#include "SVGSeatStyleConfig.hpp"
#include "SeatStyleType.hpp"

namespace kk::renderer {

CanvasSeatStyleRenderer::CanvasSeatStyleRenderer() {
}

std::unordered_map<std::string, tgfx::Rect> CanvasSeatStyleRenderer::renderAllSeatStyles(
    tgfx::Canvas *canvas,
    const tgfx::Size &itemSize,
    int columns,
    int itemSpacing,
    float density,
    const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) {
    std::unordered_map<std::string, tgfx::Rect> result = {};

    if (!canvas || itemSize.width <= 0 || itemSize.height <= 0 || styleIdToConfig.empty()) {
        return result;
    }

    float startX = static_cast<float>(itemSpacing);
    float startY = static_cast<float>(itemSpacing);

    for (const auto &[styleId, config] : styleIdToConfig) {
        tgfx::Rect itemRect = tgfx::Rect::MakeXYWH(startX, startY, itemSize.width, itemSize.height);

        if (config) {
            tgfx::AutoCanvasRestore autoRestore(canvas);
            canvas->translate(itemRect.x(), itemRect.y());

            auto type = config->getType();
            switch (type) {
                case SeatStyleType::Circle: {
                    auto circleConfig = std::static_pointer_cast<CircleSeatStyleConfig>(config);
                    if (circleConfig) {
                        renderCircleStyle(canvas, itemSize, *circleConfig, density);
                    }
                    break;
                }
                case SeatStyleType::SVG: {
                    auto svgConfig = std::static_pointer_cast<SVGSeatStyleConfig>(config);
                    if (svgConfig) {
                        renderSVGStyle(canvas, itemSize, *svgConfig, density);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        result[styleId] = itemRect;

        startX += itemSize.width + static_cast<float>(itemSpacing);
        if (startX + itemSize.width > static_cast<float>(columns * (itemSize.width + itemSpacing) + itemSpacing)) {
            startX = static_cast<float>(itemSpacing);
            startY += itemSize.height + static_cast<float>(itemSpacing);
        }
    }

    return result;
}

void CanvasSeatStyleRenderer::renderCircleStyle(tgfx::Canvas *canvas,
                                                const tgfx::Size &itemSize,
                                                const CircleSeatStyleConfig &config,
                                                float density) {
    tgfx::Rect ovalRect = tgfx::Rect::MakeWH(itemSize.width, itemSize.height);
    tgfx::Path ovalPath = {};
    ovalPath.addOval(ovalRect);

    tgfx::Paint paint = {};

    paint.setColor(config.getFillColor());
    canvas->drawPath(ovalPath, paint);

    const auto &overlayColor = config.getOverlayColor();
    if (overlayColor.has_value()) {
        paint.setColor(*overlayColor);
        canvas->drawPath(ovalPath, paint);
    }

    const auto &checkmarkColor = config.getCheckmarkColor();
    if (checkmarkColor.has_value()) {
        tgfx::Path checkmarkPath = {};
        float scaleX = itemSize.width / 36.0f;
        float scaleY = itemSize.height / 36.0f;
        checkmarkPath.moveTo(11.0f * scaleX, 17.0f * scaleY);
        checkmarkPath.lineTo(16.0f * scaleX, 22.0f * scaleY);
        checkmarkPath.lineTo(24.0f * scaleX, 13.0f * scaleY);

        paint.setColor(*checkmarkColor);
        paint.setStyle(tgfx::PaintStyle::Stroke);
        paint.setStroke(tgfx::Stroke{4.0f * density, tgfx::LineCap::Round, tgfx::LineJoin::Round});
        canvas->drawPath(checkmarkPath, paint);
    }
}

void CanvasSeatStyleRenderer::renderSVGStyle(tgfx::Canvas *canvas,
                                             const tgfx::Size &itemSize,
                                             const SVGSeatStyleConfig &config,
                                             float density) {
    (void)density;
    const auto &content = config.getContent();
    auto data = tgfx::Data::MakeWithoutCopy(content.data(), content.size());
    auto stream = tgfx::Stream::MakeFromData(data);
    auto dom = tgfx::SVGDOM::Make(*stream);
    if (!dom) {
        return;
    }

    auto domSize = dom->getContainerSize();
    auto scaleX = itemSize.width / domSize.width;
    auto scaleY = itemSize.height / domSize.height;
    canvas->scale(scaleX, scaleY);
    dom->render(canvas);
}

};  // namespace kk::renderer
