//
//  Platform.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#include "Platform.hpp"

#include <chrono>
namespace kk {

bool Platform::registerFallbackFonts() const {
    return false;
}

double Platform::currentMediaTime() const {
    static const auto START_TIME = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - START_TIME);
    return static_cast<double>(ms.count());
}

std::shared_ptr<DisplayLink> Platform::createDisplayLink(std::function<void()>) const {
    return nullptr;
}

};  // namespace kk
