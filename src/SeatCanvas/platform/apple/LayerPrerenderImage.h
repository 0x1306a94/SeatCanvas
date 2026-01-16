//
//  LayerPrerenderImage.h
//  SeatCanvas
//
//  Created by king on 2025/11/19.
//

#ifndef LayerPrerenderImage_h
#define LayerPrerenderImage_h

#import <CoreGraphics/CGImage.h>

#import <memory>

#import <tgfx/core/Size.h>

namespace tgfx {
class Layer;
class Image;
};  // namespace tgfx

namespace kk::renderer {
class LayerPrerenderImage {
  public:
    static CGImageRef Render(const std::shared_ptr<tgfx::Layer> &layer, const tgfx::Size &sourceSize, int targetWidth);
    static std::shared_ptr<tgfx::Image> RenderImage(const std::shared_ptr<tgfx::Layer> &layer, const tgfx::Size &sourceSize, int targetWidth);
};

};  // namespace kk::renderer

#endif /* LayerPrerenderImage_h */
