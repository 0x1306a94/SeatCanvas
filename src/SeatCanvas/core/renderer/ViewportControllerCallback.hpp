//
//  ViewportControllerCallback.hpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#ifndef ViewportControllerCallback_hpp
#define ViewportControllerCallback_hpp

#include "core/BaseMapColorState.h"

namespace kk::renderer {

/// ViewportController 回调接口，由 SeatCanvasCoreRenderer 实现
class ViewportControllerCallback {
  public:
    virtual ~ViewportControllerCallback() = default;
    virtual void onInvalidateContent() = 0;
    virtual void onApplyBaseMapColorState(kk::BaseMapColorState state) = 0;
    virtual void onHideMinimapWithoutAnimation() = 0;
    virtual void onSetOverlayBackVisible(bool visible) = 0;
    virtual void onSetOverlayBackAlpha(float alpha) = 0;
    virtual bool onIsOverlayBackVisible() const = 0;
    virtual void onUpdateZoomPanControllerState(bool notify) = 0;
};

};  // namespace kk::renderer

#endif /* ViewportControllerCallback_hpp */
