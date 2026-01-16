//
//  Font.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef Font_h
#define Font_h

#include <string>
#include <vector>

namespace kk {
class Font {
  public:
    const std::string fontFamily;
    const std::string fontStyle;

    static Font RegisterFont(const std::string &fontPath, int ttcIndex, const std::string &fontFamily = "", const std::string &fontStyle = "");

    static Font RegisterFont(const void *data, size_t length, int ttcIndex, const std::string &fontFamily = "", const std::string &fontStyle = "");

    static void UnregisterFont(const Font &font);

    static void SetFallbackFontNames(const std::vector<std::string> &fontNames);

    static void SetFallbackFontPaths(const std::vector<std::string> &fontPaths, const std::vector<int> &ttcIndices);

    Font(std::string fontFamily, std::string fontStyle)
        : fontFamily(std::move(fontFamily))
        , fontStyle(std::move(fontStyle)) {
    }
};
};  // namespace kk

#endif  // Font_h
