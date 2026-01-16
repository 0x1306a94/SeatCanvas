//
//  Font.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "Font.h"

#include "FontManager.hpp"

namespace kk {
Font Font::RegisterFont(const std::string &fontPath, int ttcIndex,
                        const std::string &fontFamily, const std::string &fontStyle) {
    return FontManager::RegisterFont(fontPath, ttcIndex, fontFamily, fontStyle);
}

Font Font::RegisterFont(const void *data, size_t length, int ttcIndex,
                        const std::string &fontFamily, const std::string &fontStyle) {
    return FontManager::RegisterFont(data, length, ttcIndex, fontFamily, fontStyle);
}

void Font::UnregisterFont(const Font &font) {
    return FontManager::UnregisterFont(font);
}

void Font::SetFallbackFontNames(const std::vector<std::string> &fontNames) {
    FontManager::SetFallbackFontNames(fontNames);
}

void Font::SetFallbackFontPaths(const std::vector<std::string> &fontPaths,
                                const std::vector<int> &ttcIndices) {
    FontManager::SetFallbackFontPaths(fontPaths, ttcIndices);
}

};  // namespace kk