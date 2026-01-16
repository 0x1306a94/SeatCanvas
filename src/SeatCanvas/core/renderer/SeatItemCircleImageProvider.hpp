//
//  SeatItemCircleImageProvider.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#ifndef SeatItemCircleImageProvider_hpp
#define SeatItemCircleImageProvider_hpp

#include "SeatItemImageProvider.hpp"

#include <unordered_map>

namespace tgfx {
class Context;
}  // namespace tgfx
namespace kk::renderer {
class SeatItemCircleImageProvider : public SeatItemImageProvider {
  public:
    explicit SeatItemCircleImageProvider();
    virtual ~SeatItemCircleImageProvider() = default;
    virtual std::shared_ptr<tgfx::Image> lookup(const SeatItemImageKey &key) override;
    virtual void attachContext(tgfx::Context *context, float density) override;

  private:
    void generate();

  private:
    tgfx::Context *context = {nullptr};
    float density = 1.0f;
    std::unordered_map<kk::SeatItemImageKey, std::shared_ptr<tgfx::Image>> imageCache = {};
};

};  // namespace kk::renderer

#endif /* SeatItemCircleImageProvider_hpp */
