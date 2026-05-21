//
//  SeatDataBuilder.mm
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "SeatDataBuilder.h"

#import "core/SeatData.hpp"
#import "core/renderer/SeatCanvasCoreRenderer.hpp"

#import <string>
#import <vector>
namespace kk::bridge {
CPPObject *_Nonnull CreateSeatDataBuilder() {
    auto seats = new std::vector<kk::SeatData>();
    CPPObject *cppObj = AllocCPPObject(ObjectTag::ZoneDataBuilder, seats, CPPObjectDeleter<std::vector<kk::SeatData>>);
    return cppObj;
}

void SeatDataBuilderPut(CPPObject *_Nonnull cppObject, NSString *_Nonnull seatId, CGPoint position, float rotationDegrees,
                        uint16_t pricecodeIndex) {
    if (cppObject == nullptr || cppObject->realValue == nullptr || seatId.length == 0) {
        return;
    }
    auto seats = static_cast<std::vector<kk::SeatData> *>(cppObject->realValue);
    seats->push_back({std::string(seatId.UTF8String), static_cast<float>(position.x), static_cast<float>(position.y), rotationDegrees, pricecodeIndex});
}
};  // namespace kk::bridge
