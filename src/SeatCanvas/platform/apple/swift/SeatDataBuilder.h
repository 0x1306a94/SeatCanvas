//
//  SeatDataBuilder.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "CPPObject.hpp"

#import <CoreGraphics/CGGeometry.h>
#import <Foundation/NSString.h>
namespace kk::bridge {
CPPObject *_Nonnull CreateSeatDataBuilder();
void SeatDataBuilderPut(CPPObject *_Nonnull cppObject, NSString *_Nonnull seatId, CGPoint position, float rotationDegrees,
                        uint16_t pricecodeIndex);
};  // namespace kk::bridge
