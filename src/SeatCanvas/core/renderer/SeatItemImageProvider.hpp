//
//  SeatItemImageProvider.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#ifndef SeatItemImageProvider_hpp
#define SeatItemImageProvider_hpp

#include "SeatItemImageKey.hpp"

#include <memory>

namespace tgfx {
class Context;
class Image;
};  // namespace tgfx

namespace kk::renderer {
class SeatItemImageProvider {
  public:
    virtual ~SeatItemImageProvider() = default;
    virtual std::shared_ptr<tgfx::Image> lookup(const SeatItemImageKey &key) = 0;
    virtual void attachContext(tgfx::Context *context, float density) = 0;
};

};  // namespace kk::renderer

#endif /* SeatItemImageProvider_hpp */
