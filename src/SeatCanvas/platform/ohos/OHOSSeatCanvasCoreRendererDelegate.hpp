//
//  OHOSSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#ifndef OHOSSeatCanvasCoreRendererDelegate_hpp
#define OHOSSeatCanvasCoreRendererDelegate_hpp

#include <napi/native_api.h>
#include <string>

#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::js {

class OHOSSeatCanvasCoreRendererDelegate : public kk::renderer::SeatCanvasCoreRendererDelegate {
  public:
    OHOSSeatCanvasCoreRendererDelegate();
    virtual ~OHOSSeatCanvasCoreRendererDelegate();
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId);
    virtual bool shouldSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
    virtual void didSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
    virtual void didDeselectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);

    void setDidTapZoneCallback(napi_env env, napi_value callback);
    void setShouldSelectSeatCallback(napi_env env, napi_value callback);
    void setDidSelectSeatCallback(napi_env env, napi_value callback);
    void setDidDeselectSeatCallback(napi_env env, napi_value callback);

  private:
    napi_ref _didTapZone = nullptr;
    napi_ref _shouldSelectSeat = nullptr;
    napi_ref _didSelectSeat = nullptr;
    napi_ref _didDeselectSeat = nullptr;
};

};  // namespace kk::js

#endif /* OHOSSeatCanvasCoreRendererDelegate_hpp */
