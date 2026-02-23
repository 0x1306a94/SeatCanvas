//
//  SeatDataBuilder.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "CPPObject.hpp"

#import <CoreGraphics/CGGeometry.h>
#import <string>
namespace kk::bridge {
CPPObject *_Nonnull CreateSeatDataBuilder();
void SeatDataBuilderPut(CPPObject *_Nonnull cppObject, const std::string &seatId, uint32_t status, bool selected, CGPoint position, float rotationDegrees);
};  // namespace kk::bridge
