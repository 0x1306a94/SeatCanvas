//
//  WebRendererCore.hpp
//  SeatCanvas
//
//  Created by KK on 2026/5/26.
//

#ifndef WebRendererCore_hpp
#define WebRendererCore_hpp

#include <memory>
#include <string>
#include <vector>

#include <emscripten/val.h>

namespace kk::renderer {
class SeatCanvasCoreRenderer;
};

namespace kk::web {
class WebSeatCanvasCoreRendererDelegate;
class WebRendererCore {
  public:
    static std::shared_ptr<WebRendererCore> MakeFrom(const std::string &canvasID);
    static void SetFallbackFontNames(const emscripten::val &fontNames);

    explicit WebRendererCore(const std::string &canvasID);
    ~WebRendererCore();

    void start();
    void stop();
    void draw(bool force = false);

    bool loadBaseMapFromSVG(const std::string &svgData, const std::string &parseConfigJSON = "");

    void handleTap(float x, float y);
    void handlePan(int state, float translationX, float translationY, double timestampMs);
    void handlePinch(int state, float scale, float centerX, float centerY);

    void setBackgroundColor(float r, float g, float b, float a);
    emscripten::val getBackgroundColor() const;

    void setSeatSize(float size);
    float getSeatSize() const;

    void setDebugHUDEnabled(bool enabled);
    bool isDebugHUDEnabled() const;

    float getZoomScale() const;
    float getMinimumZoomScale() const;
    float getMaximumZoomScale() const;
    emscripten::val getContentOffset() const;
    emscripten::val getVisibleOriginalRect() const;
    float getFPS() const;

    void setSeatRenderZoomThreshold(float threshold);
    float getSeatRenderZoomThreshold() const;

    void zoomToRect(float left, float top, float right, float bottom, bool animated, float padding, double durationMs);

    void registerPricecodes(const emscripten::val &pricecodes);
    void setSeatData(const std::string &zoneId, const emscripten::val &seats);
    void clearSeatData();
    void updateSeatStatuses(const emscripten::val &updates);
    void updateSeatStatusesForZone(const std::string &zoneId, const emscripten::val &statuses);
    void setSelectedSeatIds(const emscripten::val &seatIds);
    void updateSelectedSeatIds(const emscripten::val &added, const emscripten::val &removed);
    void applySeatStyleJSONConfig(const std::string &jsonString);

    void updateSeatZoneAlternateColors(const emscripten::val &colors);
    void updateMiniMapZoneAlternateColors(const emscripten::val &colors);

    bool updateSize();
    void invalidateContent();

    std::shared_ptr<WebSeatCanvasCoreRendererDelegate> getDelegate() {
        return _delegate;
    }

    kk::renderer::SeatCanvasCoreRenderer *internalRenderer() {
        return _renderer.get();
    }

  private:
    static void syncSystemProperties();

    std::unique_ptr<kk::renderer::SeatCanvasCoreRenderer> _renderer = {nullptr};
    std::shared_ptr<WebSeatCanvasCoreRendererDelegate> _delegate = {nullptr};
};
};  // namespace kk::web

#endif /* WebRendererCore_hpp */
