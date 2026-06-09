#include "SeatCanvasTestFixture.hpp"

#include "Baseline.hpp"
#include "TestFileUtils.hpp"
#include "core/DeviceLockGuard.hpp"
#include "core/parser/BaseMapLoadResult.hpp"
#include "core/parser/BaseMapParserFactory.hpp"
#include "platform/apple/renderer/ApplePlatformView.h"

#include <tgfx/gpu/Window.h>
#include <tgfx/layers/TextLayer.h>

#import <MetalKit/MTKView.h>

namespace kk::test {

void SeatCanvasTestFixture::SetUpTestSuite() {
}

void SeatCanvasTestFixture::TearDownTestSuite() {
}

void SeatCanvasTestFixture::SetUp() {
    delegate = std::make_shared<NullRendererDelegate>();
    renderer = makeRenderer();
}

void SeatCanvasTestFixture::TearDown() {
    if (::testing::Test::HasFailure()) {
        Baseline::RecordFailure();
    }
    renderer = nullptr;
    platformView = nullptr;
    metalView = nullptr;
    delegate = nullptr;
}

std::unique_ptr<kk::renderer::SeatCanvasCoreRenderer> SeatCanvasTestFixture::makeRenderer(int width, int height, float density) {
    metalView = std::make_unique<TestMetalView>(width, height, density);
    auto *mtkView = (__bridge MTKView *)metalView->rawView();
    auto view = std::make_unique<kk::renderer::ApplePlatformView>(mtkView);
    platformView = view.get();
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();
    auto coreRenderer = std::make_unique<kk::renderer::SeatCanvasCoreRenderer>(
        std::move(view),
        std::move(zoomPanController));
    coreRenderer->setDelegate(delegate);
    coreRenderer->setDebugHUDEnabled(false);
    coreRenderer->setBackgroundColor(tgfx::Color::White());
    return coreRenderer;
}

bool SeatCanvasTestFixture::loadSVGBaseMap(kk::renderer::SeatCanvasCoreRenderer &targetRenderer, const std::string &relativePath) {
    auto data = ReadFile(relativePath);
    if (data == nullptr) {
        return false;
    }
    auto result = kk::parser::BaseMapParserFactory::parse(data, kk::parser::BaseMapFormat::SVG, nullptr);
    if (result == nullptr) {
        return false;
    }
    kk::parser::BaseMapLoadResult loadResult(std::move(result));
    targetRenderer.setBaseMapConfig(loadResult.makeBaseMapConfig());
    targetRenderer.updateSize();
    targetRenderer.invalidateContent();
    if (platformView != nullptr) {
        platformView->invalidSize();
        targetRenderer.updateSize();
    }
    return true;
}

void SeatCanvasTestFixture::renderFrame(kk::renderer::SeatCanvasCoreRenderer &targetRenderer) {
    targetRenderer.invalidateContent();
    targetRenderer.draw(true);
    waitForGPU(platformView);
    targetRenderer.draw(true);
    waitForGPU(platformView);
}

bool SeatCanvasTestFixture::compareBaseline(const std::string &key) {
    if (platformView == nullptr) {
        return false;
    }
    auto window = platformView->getWindow();
    if (window == nullptr) {
        return false;
    }
    auto device = window->getDevice();
    if (device == nullptr) {
        return false;
    }
    kk::DeviceLockGuard lockGuard(device.get());
    if (!lockGuard) {
        return false;
    }
    auto surface = platformView->getSurface(lockGuard.context());
    if (surface == nullptr) {
        return false;
    }
    return Baseline::Compare(surface, key);
}

};  // namespace kk::test
