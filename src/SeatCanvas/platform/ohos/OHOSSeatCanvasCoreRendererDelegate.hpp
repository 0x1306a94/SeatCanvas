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
    void didTapZone(uint32_t coreID, const std::string &zoneId) override;
    bool styleIdForSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId,
                        std::string &outStyleId) override;
    bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) override;

    void setDidTapZoneCallback(napi_env env, napi_value callback);
    void setStyleIdForSeatCallback(napi_env env, napi_value callback);
    void setDidTapSeatCallback(napi_env env, napi_value callback);

  private:
    napi_ref _didTapZone = nullptr;
    napi_ref _styleIdForSeat = nullptr;
    napi_ref _didTapSeat = nullptr;
};

};  // namespace kk::js

#endif /* OHOSSeatCanvasCoreRendererDelegate_hpp */
