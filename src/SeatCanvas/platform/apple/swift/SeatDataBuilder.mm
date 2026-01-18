//
//  SeatDataBuilder.mm
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "SeatDataBuilder.h"

#import "core/SeatData.hpp"
#import <vector>
namespace kk::bridge {
CPPObject *_Nonnull CreateSeatDataBuilder() {
    auto seats = new std::vector<kk::SeatData>();
    CPPObject *cppObj = AllocCPPObject(ObjectTag::ZoneDataBuilder, seats, CPPObjectDeleter<std::vector<kk::SeatData>>);
    return cppObj;
}

void SeatDataBuilderPut(CPPObject *_Nonnull cppObject, const std::string &seatId, uint32_t status, bool selected, CGPoint position) {
    if (cppObject == nullptr || cppObject->realValue == nullptr || seatId.empty()) {
        return;
    }
    auto seats = static_cast<std::vector<kk::SeatData> *>(cppObject->realValue);
    seats->push_back({seatId, status, selected, static_cast<float>(position.x), static_cast<float>(position.y)});
}
};  // namespace kk::bridge
