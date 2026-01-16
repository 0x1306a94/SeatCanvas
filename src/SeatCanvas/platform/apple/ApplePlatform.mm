//
//  ApplePlatform.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#import "ApplePlatform.h"

#import "core/FontManager.hpp"

#import <QuartzCore/CABase.h>
#import <tgfx/layers/TextLayer.h>
namespace kk {

ApplePlatform::ApplePlatform() {
}

bool ApplePlatform::registerFallbackFonts() const {
    std::vector<std::string> fallbackList = {
        "PingFang SC",
        "Apple SD Gothic Neo",
        "Apple Color Emoji",
        "Helvetica",
        "Myanmar Sangam MN",
        "Thonburi",
        "Mishafi",
        "Menlo",
        "Kailasa",
        "Kefa",
        "Kohinoor Telugu",
        "Hiragino Maru Gothic ProN",
    };
    FontManager::SetFallbackFontNames(fallbackList);

    // 使用 GetFallbackTypefacesDirect() 避免静态初始化循环依赖
    std::vector<std::shared_ptr<tgfx::Typeface>> fallbackTypefaces = FontManager::GetFallbackTypefacesDirect();
    tgfx::TextLayer::SetFallbackTypefaces(std::move(fallbackTypefaces));
    return true;
}

double ApplePlatform::currentMediaTime() const {
    const auto t = CACurrentMediaTime() * 1000.0;
    return t;
}

};  // namespace kk
