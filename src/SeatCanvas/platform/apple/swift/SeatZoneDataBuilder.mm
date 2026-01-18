//
//  SeatZoneDataBuilder.mm
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "SeatZoneDataBuilder.h"
#import "ColorCast.h"
#import "core/SeatZoneData.hpp"

#import <unordered_map>

namespace kk::bridge {
CPPObject *_Nonnull CreateSeatZoneDataBuilder() {
    auto map = new std::unordered_map<std::string, kk::SeatZoneData>();
    CPPObject *cppObj = AllocCPPObject(ObjectTag::ZoneDataBuilder, map, CPPObjectDeleter<std::unordered_map<std::string, kk::SeatZoneData>>);
    return cppObj;
}

void SeatZoneDataBuilderPut(CPPObject *_Nonnull cppObject, const std::string &zoneId, UIColor *_Nullable color, UIColor *_Nullable priceColor) {
    if (cppObject == nullptr || cppObject->realValue == nullptr || zoneId.empty()) {
        return;
    }
    auto map = static_cast<std::unordered_map<std::string, kk::SeatZoneData> *>(cppObject->realValue);
    map->insert_or_assign(zoneId, kk::SeatZoneData(zoneId, UIColorToTGFXOptional(color), UIColorToTGFXOptional(priceColor)));
}

void SeatZoneDataBuilderDelete(CPPObject *_Nonnull cppObject, const std::string &zoneId) {
    if (cppObject == nullptr || cppObject->realValue == nullptr || zoneId.empty()) {
        return;
    }
    auto map = static_cast<std::unordered_map<std::string, kk::SeatZoneData> *>(cppObject->realValue);
    auto iter = map->find(zoneId);
    if (iter != map->end()) {
        map->erase(iter);
    }
}
};  // namespace kk::bridge
