//
//  SeatZoneDataBuilder.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "CPPObject.hpp"

#import <UIKit/UIColor.h>
#import <string>

namespace kk::bridge {
CPPObject *_Nonnull CreateSeatZoneDataBuilder();
void SeatZoneDataBuilderPut(CPPObject *_Nonnull cppObject, const std::string &zoneId, UIColor *_Nullable color, UIColor *_Nullable priceColor);
void SeatZoneDataBuilderDelete(CPPObject *_Nonnull cppObject, const std::string &zoneId);
};  // namespace kk::bridge
