#include "core/gesture/PanProxy.hpp"
#include "core/gesture/VelocityTracker.hpp"

#include <gtest/gtest.h>

using namespace kk::gesture;

TEST(PanProxy, InitialStateIsPossible) {
    PanProxy proxy;
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
}

TEST(PanProxy, ConstructorWithCustomState) {
    PanProxy proxy(GestureState::BEGAN);
    EXPECT_EQ(proxy.state(), GestureState::BEGAN);
}

TEST(PanProxy, HandleBeganChangesState) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    EXPECT_EQ(proxy.state(), GestureState::BEGAN);
}

TEST(PanProxy, HandleChangedChangesState) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    EXPECT_EQ(proxy.state(), GestureState::CHANGED);
}

TEST(PanProxy, HandleChangedUpdatesTranslation) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(30.f, 40.f), 116.0);
    EXPECT_FLOAT_EQ(proxy.translation().x, 30.f);
    EXPECT_FLOAT_EQ(proxy.translation().y, 40.f);
}

TEST(PanProxy, HandleEndedResetsToPossible) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    proxy.handle(GestureState::ENDED, tgfx::Point::Make(10.f, 20.f), 132.0);
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
}

TEST(PanProxy, HandleCancelledResetsToPossible) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CANCELLED, tgfx::Point::Make(0.f, 0.f), 116.0);
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
}

TEST(PanProxy, DefaultStateIsPossible) {
    PanProxy proxy;
    // Handle with POSSIBLE should be a no-op
    proxy.handle(GestureState::POSSIBLE, tgfx::Point::Make(100.f, 200.f), 100.0);
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
    // Translation should still be zero since no gesture started
    EXPECT_FLOAT_EQ(proxy.translation().x, 0.f);
    EXPECT_FLOAT_EQ(proxy.translation().y, 0.f);
}

TEST(PanProxy, TranslationIsZeroAtBegin) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(50.f, 60.f), 100.0);
    // At begin, translation is reset to zero
    EXPECT_FLOAT_EQ(proxy.translation().x, 0.f);
    EXPECT_FLOAT_EQ(proxy.translation().y, 0.f);
}

TEST(PanProxy, VelocityDefaultsToZero) {
    PanProxy proxy;
    auto vel = proxy.velocity();
    EXPECT_FLOAT_EQ(vel.x, 0.f);
    EXPECT_FLOAT_EQ(vel.y, 0.f);
}

TEST(PanProxy, VelocityComputedAfterGesture) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(20.f, 40.f), 132.0);
    auto vel = proxy.velocity();
    // Should have non-zero velocity after movement
    EXPECT_GT(std::fabs(vel.x), 0.f);
    EXPECT_GT(std::fabs(vel.y), 0.f);
}

TEST(PanProxy, ResetClearsState) {
    PanProxy proxy;
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    proxy.reset();
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
    EXPECT_FLOAT_EQ(proxy.translation().x, 0.f);
    EXPECT_FLOAT_EQ(proxy.translation().y, 0.f);
}

TEST(PanProxy, CallbackIsInvokedOnHandle) {
    PanProxy proxy;
    int callCount = 0;
    proxy.setCallback([&]() {
        callCount++;
    });

    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    EXPECT_EQ(callCount, 1);

    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    EXPECT_EQ(callCount, 2);

    proxy.handle(GestureState::ENDED, tgfx::Point::Make(10.f, 20.f), 132.0);
    EXPECT_EQ(callCount, 3);
}

TEST(PanProxy, NoCallbackNoCrash) {
    PanProxy proxy;
    // Should not crash without callback set
    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 100.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(10.f, 20.f), 116.0);
    proxy.handle(GestureState::ENDED, tgfx::Point::Make(10.f, 20.f), 132.0);
}

TEST(PanProxy, FullGestureLifecycle) {
    PanProxy proxy;
    GestureState states[4] = {};
    int stateIndex = 0;
    proxy.setCallback([&]() {
        states[stateIndex++] = proxy.state();
    });

    proxy.handle(GestureState::BEGAN, tgfx::Point::Make(0.f, 0.f), 0.0);
    proxy.handle(GestureState::CHANGED, tgfx::Point::Make(5.f, 10.f), 16.0);
    proxy.handle(GestureState::ENDED, tgfx::Point::Make(5.f, 10.f), 32.0);

    // After ended, proxy resets to POSSIBLE internally
    EXPECT_EQ(proxy.state(), GestureState::POSSIBLE);
    EXPECT_GE(stateIndex, 2);  // at least began + changed callbacks fired
}
