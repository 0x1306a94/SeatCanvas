//
//  FontManager.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef FontManager_hpp
#define FontManager_hpp

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "Font.h"

namespace tgfx {
class Typeface;
};

namespace kk {
class FontManager {
  public:
    FontManager();
    ~FontManager();

    static Font RegisterFont(const std::string &fontPath, int ttcIndex,
                             const std::string &fontFamily, const std::string &fontStyle);

    static Font RegisterFont(const void *data, size_t length, int ttcIndex,
                             const std::string &fontFamily, const std::string &fontStyle);

    static void UnregisterFont(const Font &font);

    static void SetFallbackFontNames(const std::vector<std::string> &fontNames);

    static void SetFallbackFontPaths(const std::vector<std::string> &fontPaths, const std::vector<int> &ttcIndices);

    static void AppendFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces);

    static void PrependFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces);

    static std::vector<std::shared_ptr<tgfx::Typeface>> GetFallbackTypefaces();
    static std::vector<std::shared_ptr<tgfx::Typeface>> GetFallbackTypefacesDirect();
    static bool HasFallbackFonts();
    bool hasFallbackFonts();

  private:
    Font registerFont(const std::string &fontPath, int ttcIndex = 0, const std::string &fontFamily = "", const std::string &fontStyle = "");

    Font registerFont(const void *data, size_t length, int ttcIndex = 0, const std::string &fontFamily = "", const std::string &fontStyle = "");

    Font registerFont(std::shared_ptr<tgfx::Typeface> typeface, const std::string &fontFamily = "", const std::string &fontStyle = "");

    void unregisterFont(const Font &font);

    std::vector<std::shared_ptr<tgfx::Typeface>> getFallbackTypefaces();

    void setFallbackFontNames(const std::vector<std::string> &fontNames);

    void setFallbackFontPaths(const std::vector<std::string> &fontPaths, const std::vector<int> &ttcIndices);

    void appendFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces);

    void prependFallbackTypefaces(const std::vector<std::shared_ptr<tgfx::Typeface>> &typefaces);

  private:
    std::unordered_map<std::string, std::shared_ptr<tgfx::Typeface>> registeredFontMap;
    std::vector<std::shared_ptr<tgfx::Typeface>> fallbackFontList;
    std::mutex locker = {};
};
};  // namespace kk

#endif /* FontManager_hpp */
