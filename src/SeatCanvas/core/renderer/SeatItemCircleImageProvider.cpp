//
//  SeatItemCircleImageProvider.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#include "SeatItemCircleImageProvider.hpp"

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Color.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/Context.h>

namespace kk::renderer {
SeatItemCircleImageProvider::SeatItemCircleImageProvider()
    : context(nullptr)
    , density(1.0f) {
}

std::shared_ptr<tgfx::Image> SeatItemCircleImageProvider::lookup(const SeatItemImageKey &key) {
    if (imageCache.empty()) {
        generate();
    }
    auto iter = imageCache.find(key);
    if (iter == imageCache.end()) {
        return nullptr;
    }
    return iter->second;
}

void SeatItemCircleImageProvider::attachContext(tgfx::Context *context, float density) {
    if (this->context == context && this->density == density) {
        return;
    }
    this->context = context;
    this->density = density;
    generate();
}

void SeatItemCircleImageProvider::generate() {
    if (context == nullptr) {
        return;
    }

    imageCache.clear();

    std::vector<std::pair<kk::SeatItemImageKey, tgfx::Color>> colors{
        {kk::SeatItemImageKey{kk::SeatStatus::Available, false}, tgfx::Color::Red()},
        {kk::SeatItemImageKey{kk::SeatStatus::Available, true}, tgfx::Color::Red()},
        {kk::SeatItemImageKey{kk::SeatStatus::Sold, false}, tgfx::Color::FromRGBA(0xaa, 0xaa, 0xaa, 0xff)},
        {kk::SeatItemImageKey{kk::SeatStatus::Locked, false}, tgfx::Color::FromRGBA(0x99, 0x99, 0x99, 0xff)},
        {kk::SeatItemImageKey{kk::SeatStatus::Disabled, false}, tgfx::Color::FromRGBA(0x66, 0x66, 0x66, 0xff)},
        {kk::SeatItemImageKey{kk::SeatStatus::Sold, true}, tgfx::Color::FromRGBA(0xaa, 0xaa, 0xaa, 0xff)},
        {kk::SeatItemImageKey{kk::SeatStatus::Locked, true}, tgfx::Color::FromRGBA(0x99, 0x99, 0x99, 0xff)},
        {kk::SeatItemImageKey{kk::SeatStatus::Disabled, true}, tgfx::Color::FromRGBA(0x66, 0x66, 0x66, 0xff)},
    };

    auto columns = 4;
    auto rows = static_cast<int>(std::ceil(static_cast<float>(colors.size()) / static_cast<float>(columns)));

    int lineSpacing = 10;
    int itemSpacing = 10;

    auto baseSize = 36.0f;
    auto itemWidth = static_cast<int>(baseSize * density);
    auto itemHeight = static_cast<int>(baseSize * density);
    auto surfaceWidth = itemWidth * columns + (columns + 1) * itemSpacing;
    auto surfaceHeight = itemHeight * rows + (rows + 1) * lineSpacing;

    auto surface = tgfx::Surface::Make(context, surfaceWidth, surfaceHeight, tgfx::ColorType::RGBA_8888);
    if (!surface) {
        return;
    }

    auto canvas = surface->getCanvas();
    if (!canvas) {
        return;
    }

    auto startX = static_cast<float>(itemSpacing), startY = static_cast<float>(lineSpacing);

    std::unordered_map<kk::SeatItemImageKey, tgfx::Rect> rectsMap{};

    canvas->clear();

    for (const auto &pair : colors) {
        canvas->save();
        canvas->setMatrix(tgfx::Matrix::MakeTrans(startX, startY));
        tgfx::Paint paint;
        paint.setColor(pair.second);
        canvas->drawOval(tgfx::Rect::MakeWH(itemWidth, itemHeight), paint);

        if (pair.first.isSelected()) {
            paint.setColor(tgfx::Color{0.0f, 0.0f, 0.0f, 0.7f});
            canvas->drawOval(tgfx::Rect::MakeWH(itemWidth, itemHeight), paint);

            tgfx::Path strokePath;
            strokePath.moveTo(11.0f, 17.0f);
            strokePath.lineTo(16.0f, 22.0f);
            strokePath.lineTo(24.0f, 13.0f);
            strokePath.transform(tgfx::Matrix::MakeScale(static_cast<float>(itemWidth) / baseSize, static_cast<float>(itemHeight) / baseSize));
            paint.setColor(tgfx::Color::White());
            paint.setStyle(tgfx::PaintStyle::Stroke);
            paint.setStroke(tgfx::Stroke{4.0f * density, tgfx::LineCap::Round, tgfx::LineJoin::Round});
            canvas->drawPath(strokePath, paint);
        }
        canvas->restore();

        rectsMap.insert_or_assign(pair.first, tgfx::Rect::MakeXYWH(startX, startY, static_cast<float>(itemWidth), static_cast<float>(itemHeight)));

        startX += static_cast<float>(itemWidth + itemSpacing);

        if (startX + static_cast<float>(itemWidth) > static_cast<float>(surfaceWidth - itemSpacing)) {
            startX = static_cast<float>(itemSpacing);
            startY += static_cast<float>(itemHeight + lineSpacing);
        }
    }

    if (rectsMap.empty()) {
        return;
    }

    auto image = surface->makeImageSnapshot();
    for (const auto &[key, rect] : rectsMap) {
        auto subimage = image->makeSubset(rect);
        imageCache.emplace(key, subimage);
    }
}

};  // namespace kk::renderer
