//
//  Drawer.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "Drawer.hpp"

#include "core/renderer/SeatCanvasCoreRendererState.hpp"

#include <tgfx/core/Canvas.h>
#include <tgfx/platform/Print.h>

#include <utility>

namespace kk::drawers {

Drawer::Drawer(std::string name)
    : _name(std::move(name)) {
}

void Drawer::setVisible(bool visible) {
    if (_visible == visible) {
        return;
    }
    _visible = visible;
    onVisible(visible);
}

void Drawer::draw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (canvas == nullptr) {
        tgfx::PrintError("Drawer::draw() canvas is nullptr!");
        return;
    }

    if (state == nullptr) {
        tgfx::PrintError("Drawer::draw() state is nullptr!");
        return;
    }
    tgfx::AutoCanvasRestore autoRestore(canvas);
    onDraw(canvas, state);
}
};  // namespace kk::drawers
