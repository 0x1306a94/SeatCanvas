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

#ifdef __cplusplus
}
#endif

#endif /* SwiftBridgeCAPI_h */
