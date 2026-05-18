//
//  SwiftBridgeCAPI.h
//  SeatCanvas
//

#ifndef SwiftBridgeCAPI_h
#define SwiftBridgeCAPI_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void switf_bridge_didTapZone(uint32_t coreID, const char *zoneId);
bool switf_bridge_styleIdForSeat(uint32_t coreID, const char *zoneId, const char *seatId, char *outStyleId, size_t outStyleIdLen);
bool switf_bridge_didTapSeat(uint32_t coreID, const char *zoneId, const char *seatId);
void switf_bridge_viewportWillBeginDragging(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                            float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportDidScroll(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                    float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportDidEndDragging(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                         float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight,
                                         bool willDecelerate);
void switf_bridge_viewportDidEndDecelerating(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                             float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportWillBeginZooming(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                           float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportDidZoom(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                  float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportDidEndZooming(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                        float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);
void switf_bridge_viewportDidEndScrollingAnimation(uint32_t coreID, float zoomScale, float contentOffsetX, float contentOffsetY,
                                                   float visibleOriginalRectX, float visibleOriginalRectY, float visibleOriginalRectWidth, float visibleOriginalRectHeight);

#ifdef __cplusplus
}
#endif

#endif /* SwiftBridgeCAPI_h */
