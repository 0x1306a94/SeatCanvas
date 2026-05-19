//
//  SwiftBridge.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SwiftBridge_h
#define SwiftBridge_h

#import <CoreGraphics/CGGeometry.h>
#import <UIKit/UIColor.h>
#import <UIKit/UIImage.h>
#import <string>

#import "CPPObject.hpp"
#import "FontManagerBridge.h"
#import "SeatCanvasCoreRendererBridge.h"

namespace kk::bridge {

/// 初始化系统属性
/// - Parameters:
///   - density: 屏幕像素密度
///   - fontScale: 字体缩放比例
void SeatCanvasInitSystemProperties(CGFloat density, CGFloat fontScale);

};  // namespace kk::bridge

#endif /* SwiftBridge_h */
