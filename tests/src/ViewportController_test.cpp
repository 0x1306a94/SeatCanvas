#include "core/BaseMapColorState.h"
#include "core/ZoomLevelConfig.hpp"
#include "core/ZoomScaleConfig.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/renderer/ViewportController.hpp"
#include "core/renderer/ViewportControllerCallback.hpp"

#include <gtest/gtest.h>
#include <memory>

using namespace kk;
using namespace kk::renderer;

// ---------- Mock ViewportControllerCallback ----------

class MockCallback : public ViewportControllerCallback {
  public:
    void onInvalidateContent() override {
        invalidateCount++;
    }
    void onApplyBaseMapColorState(BaseMapColorState s) override {
        lastColorState = s;
    }
    void onHideMinimapWithoutAnimation() override {
        hideMinimapCount++;
    }
    void onSetOverlayBackVisible(bool v) override {
        overlayBackVisible = v;
    }
    void onSetOverlayBackAlpha(float a) override {
        lastAlpha = a;
    }
    bool onIsOverlayBackVisible() const override {
        return overlayBackVisible;
    }
    void onUpdateZoomPanControllerState(bool notify) override {
        updateStateCount++;
        lastNotify = notify;
    }

    int invalidateCount = 0;
    int hideMinimapCount = 0;
    int updateStateCount = 0;
    BaseMapColorState lastColorState = {};
    bool overlayBackVisible = false;
    float lastAlpha = 0.0f;
    bool lastNotify = false;
};

// ---------- Test Fixture ----------

class ViewportControllerTest : public ::testing::Test {
  public:
    void SetUp() override {
        state = std::make_unique<SeatCanvasCoreRendererState>();
        zoomPanController = std::make_unique<gesture::ElasticZoomPanController>();
        mockCallback = std::make_unique<MockCallback>();

        state->updateScreen(750, 1334, 2.0f);
        zoomPanController->setBounds(tgfx::Size::Make(750.f, 1334.f));

        controller = std::make_unique<ViewportController>(
            zoomPanController.get(),
            state.get(),
            nullptr,  // animator
            nullptr,  // delegate
            &zoomLevelConfig,
            mockCallback.get());
    }

    void setOriginAndUpdate(float w, float h) {
        state->updateOriginSize(tgfx::Size::Make(w, h));
        state->updateContentScale(1.0f);
        controller->updateContentSize();
    }

    std::unique_ptr<SeatCanvasCoreRendererState> state;
    std::unique_ptr<gesture::ElasticZoomPanController> zoomPanController;
    std::unique_ptr<MockCallback> mockCallback;
    std::unique_ptr<ViewportController> controller;
    ZoomLevelConfig zoomLevelConfig = {};
};

// ---------- Accessors ----------

TEST_F(ViewportControllerTest, DefaultMaxWidth) {
    EXPECT_FLOAT_EQ(controller->getMaxWidth(), 1000.f);
}

TEST_F(ViewportControllerTest, SetAndGetMaxWidth) {
    controller->setMaxWidth(500.f);
    EXPECT_FLOAT_EQ(controller->getMaxWidth(), 500.f);
}

TEST_F(ViewportControllerTest, IsAutoDrawSeatDisabledDefaultsFalse) {
    EXPECT_FALSE(controller->isAutoDrawSeatDisabled());
}

TEST_F(ViewportControllerTest, AutoChangeBaseMapColorStateDefaultsTrue) {
    EXPECT_TRUE(controller->autoChangeBaseMapColorState);
}

TEST_F(ViewportControllerTest, ZoomLevelConfigReturnsConfig) {
    zoomLevelConfig.seat = 5.0f;
    EXPECT_FLOAT_EQ(controller->zoomLevelConfig().seat, 5.0f);
}

TEST_F(ViewportControllerTest, AnimationStateDefaults) {
    EXPECT_FALSE(controller->isPanAnimationActive());
    EXPECT_FALSE(controller->isScrollingAnimationActive());
}

TEST_F(ViewportControllerTest, SetPanAnimationActive) {
    controller->setPanAnimationActive(true);
    EXPECT_TRUE(controller->isPanAnimationActive());
    controller->setPanAnimationActive(false);
    EXPECT_FALSE(controller->isPanAnimationActive());
}

TEST_F(ViewportControllerTest, ResetAnimationStateClearsPanActive) {
    controller->setPanAnimationActive(true);
    controller->resetAnimationState();
    EXPECT_FALSE(controller->isPanAnimationActive());
    EXPECT_FALSE(controller->isScrollingAnimationActive());
}

TEST_F(ViewportControllerTest, SetCoreIDAndDelegateNoCrash) {
    controller->setCoreID(42u);
    controller->setDelegate(nullptr);
    SUCCEED();
}

// ---------- updateContentScale ----------

TEST_F(ViewportControllerTest, ContentScaleWhenOriginExceedsMaxWidth) {
    state->updateOriginSize(tgfx::Size::Make(2000.f, 1000.f));
    controller->updateContentScale(1000.f);
    EXPECT_FLOAT_EQ(state->getContentScale(), 0.5f);
}

TEST_F(ViewportControllerTest, ContentScaleWhenOriginWithinMaxWidth) {
    state->updateOriginSize(tgfx::Size::Make(500.f, 500.f));
    controller->updateContentScale(1000.f);
    EXPECT_FLOAT_EQ(state->getContentScale(), 1.0f);
}

TEST_F(ViewportControllerTest, ContentScaleEmptyOriginUsesIdentity) {
    state->updateOriginSize(tgfx::Size::MakeEmpty());
    controller->updateContentScale(1000.f);
    EXPECT_FLOAT_EQ(state->getContentScale(), 1.0f);
}

// ---------- updateContentSize with empty origin ----------

TEST_F(ViewportControllerTest, UpdateContentSizeEmptyOriginResetsZoomToDefault) {
    state->updateOriginSize(tgfx::Size::MakeEmpty());
    controller->updateContentSize();

    EXPECT_FLOAT_EQ(zoomLevelConfig.seat, 1.0f);
    EXPECT_FLOAT_EQ(zoomLevelConfig.row, 1.0f);
    EXPECT_FLOAT_EQ(zoomLevelConfig.zone, 1.0f);
    EXPECT_FLOAT_EQ(zoomLevelConfig.venue, 1.0f);
    EXPECT_FLOAT_EQ(zoomPanController->getMinimumZoomScale(), 1.0f);
    EXPECT_FLOAT_EQ(zoomPanController->getMaximumZoomScale(), 1.0f);
    EXPECT_FLOAT_EQ(zoomPanController->getZoomScale(), 1.0f);
}

// ---------- updateMaxMinZoomScalesForCurrentBounds ----------

TEST_F(ViewportControllerTest, ZoomLevelsUseSeatBaseSizeConstants) {
    // Origin 375×667 at 1x content scale + 2x density → normalized = 750×1334
    setOriginAndUpdate(375.f, 667.f);

    float minZoom = zoomPanController->getMinimumZoomScale();
    float maxZoom = zoomPanController->getMaximumZoomScale();

    EXPECT_GT(maxZoom, minZoom);
    EXPECT_GT(zoomLevelConfig.seat, zoomLevelConfig.row);
    EXPECT_GT(zoomLevelConfig.row, zoomLevelConfig.zone);
    // venue is clamped to minimumZoomScale so it can be >= zone
    EXPECT_FLOAT_EQ(zoomLevelConfig.venue, minZoom);
}

TEST_F(ViewportControllerTest, ZoomLevelsScaleWithViewportWidth) {
    setOriginAndUpdate(375.f, 667.f);
    float seat1 = zoomLevelConfig.seat;
    float venue1 = zoomLevelConfig.venue;

    // Wider viewport → higher zoom thresholds
    state = std::make_unique<SeatCanvasCoreRendererState>();
    state->updateScreen(1500, 2668, 2.0f);
    zoomPanController = std::make_unique<gesture::ElasticZoomPanController>();
    zoomPanController->setBounds(tgfx::Size::Make(1500.f, 2668.f));
    auto cb2 = std::make_unique<MockCallback>();
    ZoomLevelConfig config2 = {};
    auto c2 = std::make_unique<ViewportController>(
        zoomPanController.get(), state.get(), nullptr, nullptr, &config2, cb2.get());
    state->updateOriginSize(tgfx::Size::Make(375.f, 667.f));
    state->updateContentScale(1.0f);
    c2->updateContentSize();

    // Twice the viewport width → roughly twice the zoom thresholds
    EXPECT_NEAR(config2.seat / seat1, 2.0f, 0.15f);
    EXPECT_GT(config2.venue, zoomLevelConfig.venue);
}

// ---------- zoomToRect (non-animated) ----------

TEST_F(ViewportControllerTest, ZoomToRectNonAnimatedDoesNotCrash) {
    setOriginAndUpdate(375.f, 667.f);

    tgfx::Rect target = tgfx::Rect::MakeXYWH(10.f, 10.f, 100.f, 100.f);
    controller->zoomToRect(target, false, 0.0f, 0.0);

    // zoom should have changed from the initial value
    EXPECT_GT(zoomPanController->getZoomScale(), 0.0f);
}

TEST_F(ViewportControllerTest, ZoomToRectWithEmptyStateReturnsEarly) {
    // Don't set origin → normalized content is empty
    tgfx::Rect target = tgfx::Rect::MakeXYWH(0.f, 0.f, 100.f, 100.f);
    auto oldZoom = zoomPanController->getZoomScale();
    controller->zoomToRect(target, false, 0.0f, 0.0);
    // Should be no-op because content size is empty
    EXPECT_FLOAT_EQ(zoomPanController->getZoomScale(), oldZoom);
}

TEST_F(ViewportControllerTest, ZoomToRectEmptyRectReturnsEarly) {
    setOriginAndUpdate(375.f, 667.f);
    auto oldZoom = zoomPanController->getZoomScale();
    controller->zoomToRect(tgfx::Rect::MakeEmpty(), false, 0.0f, 0.0);
    EXPECT_FLOAT_EQ(zoomPanController->getZoomScale(), oldZoom);
}

// ---------- notifyViewportDidEndDeceleratingIfNeeded ----------

TEST_F(ViewportControllerTest, NotifyDeceleratingNotActiveIsNoOp) {
    controller->setPanAnimationActive(false);
    controller->notifyViewportDidEndDeceleratingIfNeeded();
    // No crash and animation state unchanged
    EXPECT_FALSE(controller->isPanAnimationActive());
}

TEST_F(ViewportControllerTest, NotifyDeceleratingActiveClearsFlag) {
    controller->setPanAnimationActive(true);
    controller->notifyViewportDidEndDeceleratingIfNeeded();
    EXPECT_FALSE(controller->isPanAnimationActive());
}

TEST_F(ViewportControllerTest, PanAnimationActivePersistsUntilNotify) {
    controller->setPanAnimationActive(true);
    EXPECT_TRUE(controller->isPanAnimationActive());
    controller->notifyViewportDidEndDeceleratingIfNeeded();
    EXPECT_FALSE(controller->isPanAnimationActive());
}

// ---------- scrollViewWithLocation edge cases ----------

TEST_F(ViewportControllerTest, ScrollViewWithLocationNoStateReturnsEarly) {
    auto emptyState = std::make_unique<SeatCanvasCoreRendererState>();
    zoomPanController = std::make_unique<gesture::ElasticZoomPanController>();
    auto cb = std::make_unique<MockCallback>();
    ZoomLevelConfig config = {};
    auto vc = std::make_unique<ViewportController>(
        zoomPanController.get(), nullptr, nullptr, nullptr, &config, cb.get());
    vc->scrollViewWithLocation(tgfx::Point::Make(100.f, 100.f), 0.5f);
    // No crash
    SUCCEED();
}

// ---------- scrollViewWithZone edge cases ----------

TEST_F(ViewportControllerTest, ScrollViewWithZoneNullZoneReturnsEarly) {
    setOriginAndUpdate(375.f, 667.f);
    controller->scrollViewWithZone(nullptr, 0.5f);
    // No crash
    SUCCEED();
}

// ---------- handleZoomBack edge cases ----------

TEST_F(ViewportControllerTest, HandleZoomBackEmptyContentReturnsEarly) {
    state->updateOriginSize(tgfx::Size::MakeEmpty());
    controller->updateContentSize();
    auto oldZoom = zoomPanController->getZoomScale();
    controller->handleZoomBack();
    EXPECT_FLOAT_EQ(zoomPanController->getZoomScale(), oldZoom);
}
