//
//  SeatStatus.h
//  SeatCanvasKit
//
//  Created by king on 2025/12/8.
//

#ifndef SeatStatus_h
#define SeatStatus_h

namespace kk {

/// 座位状态
enum class SeatStatus {
    Available,  // 可用
    Sold,       // 已售
    Locked,     // 锁定
    Disabled,   // 禁用
};

};  // namespace kk

#endif /* SeatStatus_h */
