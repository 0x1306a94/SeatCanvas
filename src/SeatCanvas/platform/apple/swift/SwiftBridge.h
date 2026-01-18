//
//  SwiftBridge.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SwiftBridge_h
#define SwiftBridge_h

#import <CoreGraphics/CGGeometry.h>
#import <QuartzCore/CAEAGLLayer.h>
#import <UIKit/UIColor.h>
#import <UIKit/UIImage.h>
#import <string>

#import "CPPObject.hpp"
#import "FontManagerBridge.h"
#import "SeatCanvasCoreRendererBridge.h"

namespace kk::bridge {

struct HitTestSeatRegionResult {
    /// 区域ID
    std::string regionId{""};

    CGRect bounds{CGRectZero};

    HitTestSeatRegionResult() {
    }

    explicit HitTestSeatRegionResult(const std::string &regionId)
        : regionId(regionId) {
    }

    /// 是否有效
    bool valid() const {
        return !regionId.empty();
    }
};

/// 初始化系统属性
/// - Parameters:
///   - density: 屏幕像素密度
///   - fontScale: 字体缩放比例
void SeatCanvasInitSystemProperties(CGFloat density, CGFloat fontScale);

};  // namespace kk::bridge

#endif /* SwiftBridge_h */
