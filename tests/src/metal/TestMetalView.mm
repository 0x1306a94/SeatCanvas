#import "TestMetalView.h"

#import "platform/apple/renderer/ApplePlatformView.h"

#import <tgfx/gpu/GPU.h>
#import <tgfx/gpu/Window.h>

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>
#import <QuartzCore/QuartzCore.h>

namespace kk::test {

struct TestMetalView::Impl {
    NSWindow *window = nil;
    MTKView *view = nil;
};

TestMetalView::TestMetalView(int width, int height, float density)
    : _impl(std::make_unique<Impl>()) {
    CGFloat pointWidth = static_cast<CGFloat>(width) / static_cast<CGFloat>(density);
    CGFloat pointHeight = static_cast<CGFloat>(height) / static_cast<CGFloat>(density);
    NSRect frame = NSMakeRect(0.0, 0.0, pointWidth, pointHeight);

    auto *window = [[NSWindow alloc] initWithContentRect:frame
                                               styleMask:NSWindowStyleMaskBorderless
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];
    window.releasedWhenClosed = NO;
    [window orderOut:nil];

    auto *view = [[MTKView alloc] initWithFrame:frame];
    view.device = MTLCreateSystemDefaultDevice();
    view.paused = NO;
    view.enableSetNeedsDisplay = YES;
    view.framebufferOnly = NO;
    view.autoResizeDrawable = NO;

    auto *metalLayer = static_cast<CAMetalLayer *>(view.layer);
    metalLayer.contentsScale = static_cast<CGFloat>(density);
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.drawableSize = CGSizeMake(static_cast<CGFloat>(width), static_cast<CGFloat>(height));

    window.contentView = view;
    _impl->window = window;
    _impl->view = view;
}

TestMetalView::~TestMetalView() = default;

void *TestMetalView::rawView() const {
    return (__bridge void *)_impl->view;
}

void waitForGPU(kk::renderer::ApplePlatformView *platformView) {
    if (platformView == nullptr) {
        return;
    }
    auto window = platformView->getWindow();
    if (window == nullptr) {
        return;
    }
    auto device = window->getDevice();
    if (device == nullptr) {
        return;
    }
    auto context = device->lockContext();
    if (context == nullptr) {
        return;
    }
    auto gpu = context->gpu();
    if (gpu != nullptr && gpu->queue() != nullptr) {
        gpu->queue()->waitUntilCompleted();
    }
    device->unlock();
}

};  // namespace kk::test
