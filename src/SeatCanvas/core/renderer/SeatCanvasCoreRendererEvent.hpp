//
//  SeatCanvasCoreRendererEvent.hpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#ifndef SeatCanvasCoreRendererEvent_hpp
#define SeatCanvasCoreRendererEvent_hpp

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Size.h>

#include "core/ZoomLevelConfig.hpp"

namespace kk::renderer {
struct SeatCanvasViewportEvent {
    float zoomScale = 1.0f;
    tgfx::Point contentOffset = {};
    tgfx::Rect visibleOriginalRect = {};
};

struct SeatCanvasZoomLevelConfigEvent {
    kk::ZoomLevelConfig zoomLevels = {};
    float minimumZoomScale = 1.0f;
    float maximumZoomScale = 1.0f;
    float zoomScale = 1.0f;
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererEvent_hpp */
