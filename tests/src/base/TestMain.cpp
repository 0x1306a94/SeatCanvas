#include "TestEnvironment.hpp"

#include "core/Platform.hpp"

#include <gtest/gtest.h>

int main(int argc, char **argv) {

    kk::Platform::Current()->registerFallbackFonts();

    ::testing::AddGlobalTestEnvironment(new kk::test::TestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
