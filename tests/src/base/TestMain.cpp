#include "ProjectPath.hpp"
#include "TestEnvironment.hpp"

#include "core/FontManager.hpp"
#include "core/Log.hpp"
#include "core/Platform.hpp"

#include <gtest/gtest.h>
#include <tgfx/core/Typeface.h>
#include <tgfx/layers/TextLayer.h>

int main(int argc, char **argv) {
    // Pin a bundled CJK font for baseline tests so text rendering does not depend on
    // system fonts (CI runners vs local macOS). Noto must be prepended after
    // registerFallbackFonts(), which clears the fallback list via SetFallbackFontNames.
    const auto notoFontPath = kk::test::AbsolutePath("resources/fonts/NotoSansSC-Regular.otf");
    auto notoTypeface = tgfx::Typeface::MakeFromPath(notoFontPath, 0);

    kk::Platform::Current()->registerFallbackFonts();
    if (notoTypeface != nullptr) {
        kk::FontManager::PrependFallbackTypefaces({notoTypeface});
        tgfx::TextLayer::SetFallbackTypefaces(kk::FontManager::GetFallbackTypefacesDirect());
    }

    kk::Log::setLevel(kk::LogLevel::Info);

    ::testing::AddGlobalTestEnvironment(new kk::test::TestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
