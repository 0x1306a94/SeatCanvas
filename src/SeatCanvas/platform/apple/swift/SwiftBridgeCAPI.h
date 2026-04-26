//
//  SwiftBridgeCAPI.h
//  SeatCanvas
//

#ifndef SwiftBridgeCAPI_h
#define SwiftBridgeCAPI_h

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void switf_bridge_didTapZone(uint32_t coreID, const char *zoneId);
bool switf_bridge_shouldSelectSeat(uint32_t coreID, const char *zoneId, const char *seatId);
void switf_bridge_didSelectSeat(uint32_t coreID, const char *zoneId, const char *seatId);
void switf_bridge_didDeselectSeat(uint32_t coreID, const char *zoneId, const char *seatId);

#ifdef __cplusplus
}
#endif

#endif /* SwiftBridgeCAPI_h */
