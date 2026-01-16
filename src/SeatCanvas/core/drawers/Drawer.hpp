//
//  Drawer.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef Drawer_hpp
#define Drawer_hpp

#include <string>

namespace tgfx {
class Canvas;
};

namespace kk::renderer {
class SeatCanvasCoreRendererState;
};

namespace kk::drawers {
class Drawer {
  public:
    explicit Drawer(std::string name);

    virtual ~Drawer() = default;

    std::string name() const {
        return _name;
    }

    void setVisible(bool visible);

    bool visible() const {
        return _visible;
    }

    virtual void prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) = 0;

    void draw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);

  protected:
    virtual void onVisible(bool visible) = 0;

    virtual void onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) = 0;

  private:
    std::string _name;
    bool _visible{true};
};
};  // namespace kk::drawers

#endif /* Drawer_hpp */
