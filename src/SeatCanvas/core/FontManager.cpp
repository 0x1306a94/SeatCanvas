//
//  FontManager.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#include "FontManager.hpp"

#include "core/Platform.hpp"

#include <tgfx/core/Typeface.h>
namespace kk {
static FontManager fontManager = {};
FontManager::FontManager() {
}

FontManager::~FontManager() {
}

bool FontManager::hasFallbackFonts() {
    std::lock_guard<std::mutex> autoLock(locker);
    return !fallbackFontList.empty();
}

Font FontManager::registerFont(const std::string &fontPath, int ttcIndex, const std::string &fontFamily, const std::string &fontStyle) {
    std::lock_guard<std::mutex> autoLock(locker);
    auto typeface = tgfx::Typeface::MakeFromPath(fontPath, ttcIndex);
    return registerFont(typeface, fontFamily, fontStyle);
}

Font FontManager::registerFont(const void *data, size_t length, int ttcIndex, const std::string &fontFamily, const std::string &fontStyle) {
    std::lock_guard<std::mutex> autoLock(locker);
    auto typeface = tgfx::Typeface::MakeFromBytes(data, length, ttcIndex);
    return registerFont(typeface, fontFamily, fontStyle);
}

static std::string FontRegisterKey(const std::string &fontFamily, const std::string &fontStyle) {
    return fontFamily + "|" + fontStyle;
}

Font FontManager::registerFont(std::shared_ptr<tgfx::Typeface> typeface, const std::string &fontFamily, const std::string &fontStyle) {
    if (typeface == nullptr) {
        return {"", ""};
    }
    std::string family = typeface->fontFamily();
    std::string style = typeface->fontStyle();
    if (!fontFamily.empty()) {
        family = fontFamily;
        style = fontStyle;
    }
    auto key = FontRegisterKey(family, style);
    auto iter = registeredFontMap.find(key);
    if (iter != registeredFontMap.end()) {
        registeredFontMap.erase(iter);
    }
    registeredFontMap[key] = std::move(typeface);
    return {family, style};
}

void FontManager::unregisterFont(const Font &font) {
    if (font.fontFamily.empty()) {
        return;
    }
    std::lock_guard<std::mutex> autoLock(locker);
    auto iter = registeredFontMap.find(FontRegisterKey(font.fontFamily, font.fontStyle));
    if (iter == registeredFontMap.end()) {
        return;
    }
    registeredFontMap.erase(iter);
}

static bool RegisterFallbackFonts() {
    if (fontManager.hasFallbackFonts()) {
        return false;
    }
    return Platform::Current()->registerFallbackFonts();
}

Font FontManager::RegisterFont(const std::string &fontPath, int ttcIndex, const std::string &fontFamily, const std::string &fontStyle) {
    return fontManager.registerFont(fontPath, ttcIndex, fontFamily, fontStyle);
}

Font FontManager::RegisterFont(const void *data, size_t length, int ttcIndex, const std::string &fontFamily, const std::string &fontStyle) {
    return fontManager.registerFont(data, length, ttcIndex, fontFamily, fontStyle);
}

void FontManager::UnregisterFont(const Font &font) {
    return fontManager.unregisterFont(font);
}

std::vector<std::shared_ptr<tgfx::Typeface>> FontManager::GetFallbackTypefaces() {
    static auto registered = RegisterFallbackFonts();
    return fontManager.getFallbackTypefaces();
}

std::vector<std::shared_ptr<tgfx::Typeface>> FontManager::GetFallbackTypefacesDirect() {
    return fontManager.getFallbackTypefaces();
}

bool FontManager::HasFallbackFonts() {
    return fontManager.hasFallbackFonts();
}

void FontManager::SetFallbackFontNames(const std::vector<std::string> &fontNames) {
    fontManager.setFallbackFontNames(fontNames);
}

void FontManager::SetFallbackFontPaths(const std::vector<std::string> &fontPaths, const std::vector<int> &ttcIndices) {
    fontManager.setFallbackFontPaths(fontPaths, ttcIndices);
}

void FontManager::AppendFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces) {
    fontManager.appendFallbackTypefaces(typefaces);
}

void FontManager::PrependFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces) {
    fontManager.prependFallbackTypefaces(typefaces);
}

std::vector<std::shared_ptr<tgfx::Typeface>> FontManager::getFallbackTypefaces() {
    std::lock_guard<std::mutex> autoLock(locker);
    return fallbackFontList;
}

void FontManager::setFallbackFontNames(const std::vector<std::string> &fontNames) {
    std::lock_guard<std::mutex> autoLock(locker);
    fallbackFontList.clear();
    for (auto &fontFamily : fontNames) {
        auto face = tgfx::Typeface::MakeFromName(fontFamily, "");
        if (!face) {
            continue;
        }
        fallbackFontList.push_back(face);
    }
}

void FontManager::setFallbackFontPaths(const std::vector<std::string> &fontPaths, const std::vector<int> &ttcIndices) {
    std::lock_guard<std::mutex> autoLock(locker);
    fallbackFontList.clear();
    int index = 0;
    for (auto &fontPath : fontPaths) {
        auto face = tgfx::Typeface::MakeFromPath(fontPath, ttcIndices[index]);
        index++;
        if (!face) {
            continue;
        }
        fallbackFontList.push_back(face);
    }
}

void FontManager::appendFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces) {
    std::lock_guard<std::mutex> autoLock(locker);
    for (const auto &typeface : typefaces) {
        if (typeface == nullptr) {
            continue;
        }
        fallbackFontList.push_back(typeface);
    }
}

void FontManager::prependFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces) {
    std::lock_guard<std::mutex> autoLock(locker);
    std::vector<std::shared_ptr<tgfx::Typeface>> updatedList = {};
    updatedList.reserve(typefaces.size() + fallbackFontList.size());
    for (const auto &typeface : typefaces) {
        if (typeface == nullptr) {
            continue;
        }
        updatedList.push_back(typeface);
    }
    updatedList.insert(updatedList.end(), fallbackFontList.begin(), fallbackFontList.end());
    fallbackFontList = std::move(updatedList);
}
};  // namespace kk
