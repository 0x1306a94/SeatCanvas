//
//  SeatCanvasCoreRendererBridge.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "CPPObject.hpp"
#import "SeatDataBuilder.h"
#import "SeatZoneDataBuilder.h"
#import "core/gesture/GestureState.hpp"
#import "core/parser/BaseMapFormat.hpp"

#import <CoreGraphics/CGGeometry.h>
#import <QuartzCore/CAEAGLLayer.h>
#import <UIKit/UIColor.h>
#import <UIKit/UIImage.h>
#import <string>
#import <vector>

namespace kk::bridge {
struct ZoomLevel {
    CGFloat zoomScale9 = {1.0f};
    CGFloat zoomScale18 = {1.0f};
    CGFloat zoomScale30 = {1.0f};
    CGFloat zoomScale50 = {1.0f};
};

/// 创建C++渲染器
/// - Parameter eagLayer: 负责渲染输出的 OpenGL ES 目标
/// - Returns: C++渲染器实例
CPPObject *_Nonnull CreateSeatCanvasCoreRenderer(CAEAGLLayer *_Nullable eaglLayer);
/// 获取C++渲染器ID
/// - Parameter cppObject: C++渲染器实例
/// - Returns: 渲染器ID
uint32_t SeatCanvasCoreRendererGetCoreID(CPPObject *_Nonnull cppObject);

/// 替换C++渲染器平台视图
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - eagLayer: 负责渲染输出的 OpenGL ES 目标
bool SeatCanvasCoreRendererReplacePlatformView(CPPObject *_Nonnull cppObject, CAEAGLLayer *_Nullable eagLayer);

/// 解析底图（统一接口，支持多种格式）
/// - Parameters:
///   - bytes: 底图二进制数据
///   - len: 数据长度
///   - format: 格式
///   - miniMapImage: 生成的minimap 图片（可选）
/// - Returns: 解析成功则返回不透明数据指针，失败时返回 nullptr
void *_Nullable SeatCanvasCoreRendererParseBaseMap(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, kk::parser::BaseMapFormat format, UIImage *_Nullable *_Nullable miniMapImage);

/// 解析 SVG 底图（向后兼容接口）
/// - Parameters:
///   - bytes: svg 二进制数据
///   - len: 数据长度
///   - miniMapImage: 生成的minimap 图片
/// - Returns: 解析成功则返回不透明数据指针，失败时返回 nullptr
void *_Nullable SeatCanvasCoreRendererParseBaseMapFromSVG(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, UIImage *_Nullable *_Nullable miniMapImage);

/// 渲染器加载底图
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - loadResult: 由 SeatCanvasLoadBaseMapFromSVG 返回的指针，执行完此函数后，loadResult 将不在可用，上层请勿继续保留使用
/// - Returns: 是否成功
bool SeatCanvasCoreRendererLoadBaseMap(CPPObject *_Nonnull cppObject, void **_Nullable loadResult);

/// 设置渲染器座位区域数据
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - zoneBuilder: 区域信息
void SeatCanvasCoreRendererSetSeatZoneDatas(CPPObject *_Nonnull cppObject, CPPObject *_Nullable zoneBuilder);

/// 设置渲染器座位数据
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - zoneId: 区域ID
///   - seatBuilder: 座位信息
void SeatCanvasCoreRendererSetSeatDatas(CPPObject *_Nonnull cppObject, const std::string &zoneId, CPPObject *_Nullable seatBuilder);

/// 设置座位样式
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - bytes: json字节数据
///   - len: json字节大小
void SeatCanvasCoreRendererSetSeatStyleJSONConfig(CPPObject *_Nonnull cppObject, const void *_Nullable __sized_by_or_null(len) bytes, size_t len);

/// 让渲染内容失效
/// - Parameter cppObject: C++渲染器实例
void SeatCanvasCoreRendererInvalidateContent(CPPObject *_Nonnull cppObject);

/// 更新渲染器画布尺寸
/// - Parameter cppObject: C++渲染器实例
/// - Returns: 尺寸相对之前是否有变化
bool SeatCanvasCoreRendererUpdateSize(CPPObject *_Nonnull cppObject);

/// 更新渲染器缩放级别
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - zoomScale: 缩放级别
void SeatCanvasCoreRendererUpdateZoomScale(CPPObject *_Nonnull cppObject, CGFloat zoomScale);

/// 更新渲染器内容偏移量
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - x: 水平方向偏移
///   - y: 垂直方向偏移
void SeatCanvasCoreRendererUpdateContentOffset(CPPObject *_Nonnull cppObject, CGPoint contentOffset);

/// 渲染器缩放级别
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 缩放级别
CGFloat SeatCanvasCoreRendererZoomScale(CPPObject *_Nonnull cppObject);

/// 渲染器最小缩放级别
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 最小缩放级别
CGFloat SeatCanvasCoreRendererMinimumZoomScale(CPPObject *_Nonnull cppObject);

/// 渲染器最大缩放级别
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 最大缩放级别
CGFloat SeatCanvasCoreRendererMaximumZoomScale(CPPObject *_Nonnull cppObject);

/// 渲染器内容偏移量
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 内容偏移量
CGPoint SeatCanvasCoreRendereContentOffset(CPPObject *_Nonnull cppObject);

/// 渲染器底图缩放级别
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 缩放级别
CGFloat SeatCanvasCoreRendererBaseMapScale(CPPObject *_Nonnull cppObject);

/// 渲染器底图原始大小
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 底图大小
CGSize SeatCanvasCoreRendererBaseMapSize(CPPObject *_Nonnull cppObject);

/// 渲染器画布大小
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 画布大小
CGSize SeatCanvasCoreRendererBoundsSize(CPPObject *_Nonnull cppObject);

/// 渲染器内容大小
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 内容大小
CGSize SeatCanvasCoreRendererContentSize(CPPObject *_Nonnull cppObject);

/// 设置渲染画布背景色
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - color: 背景色
/// - Returns: 内容大小
void SeatCanvasCoreRendererSetBackgroundColor(CPPObject *_Nonnull cppObject, UIColor *_Nullable color);

/// 获取渲染画布背景色
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - color: 背景色
/// - Returns: 内容大小
UIColor *SeatCanvasCoreRendererGetBackgroundColor(CPPObject *_Nonnull cppObject);

/// 获取缩放级别
/// - Parameter cppObject: C++渲染器实例
/// - Returns: 缩放级别
ZoomLevel SeatCanvasCoreRendererGetZoomLevel(CPPObject *_Nonnull cppObject);

/// 处理点击手势
/// - Parameter cppObject: C++渲染器实例
/// - Parameter location: 当前手势位置, viewport 坐标系 (像素单位)
void SeatCanvasCoreRendererHandTap(CPPObject *_Nonnull cppObject, CGPoint location);

/// 处理滑动手势
/// - Parameter cppObject: C++渲染器实例
/// - Parameter state: 手势状态
/// - Parameter translation: 当前手势位置, viewport 坐标系 (像素单位)
/// - Parameter timestampMs: 手势时间戳（毫秒）
void SeatCanvasCoreRendererHandPan(CPPObject *_Nonnull cppObject, kk::gesture::GestureState state, CGPoint translation, double timestampMs);

/// 处理缩放手势
/// - Parameter cppObject: C++渲染器实例
/// - Parameter state: 手势状态
/// - Parameter scale: 缩放级别
/// - Parameter center: 缩放中心, viewport 坐标系  (像素单位)
void SeatCanvasCoreRendererHandPinch(CPPObject *_Nonnull cppObject, kk::gesture::GestureState state, CGFloat scale, CGPoint center);

/// 启动渲染器
/// - Parameter cppObject: C++渲染器实例
void SeatCanvasCoreRendererStart(CPPObject *_Nonnull cppObject);

/// 停止渲染器
/// - Parameter cppObject: C++渲染器实例
void SeatCanvasCoreRendererStop(CPPObject *_Nonnull cppObject);

///// 渲染器渲染当前帧
///// - Parameters:
/////   - cppObject: C++渲染器实例
/////   - force: true 强制渲染， false 内容有变化或者被 invalidateContent 后才会执行真实渲染
// void SeatCanvasCoreRendererDraw(CPPObject *_Nonnull cppObject, bool force);

/// 获取渲染器当前像素密度
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 像素密度
CGFloat SeatCanvasCoreRendererGetDensity(CPPObject *_Nonnull cppObject);

/// 获取渲染器当前显示区域
/// - Parameters:
///   - cppObject: C++渲染器实例
/// - Returns: 显示区域
CGRect SeatCanvasCoreRendererGetVisibleContentRect(CPPObject *_Nonnull cppObject);

/// 缩放到指定区域并居中显示
/// - Parameters:
///   - cppObject: C++渲染器实例
///   - rect: 目标区域（在内容坐标系中）
///   - animated: 是否使用动画
///   - padding: 区域周围的边距（在内容坐标系中），默认为 0
///   - durationMs: 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
void SeatCanvasCoreRendererZoomToRect(CPPObject *_Nonnull cppObject, CGRect rect, bool animated, CGFloat padding, double durationMs);
};  // namespace kk::bridge
