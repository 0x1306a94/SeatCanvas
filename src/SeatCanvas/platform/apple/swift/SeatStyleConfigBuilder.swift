//
//  SeatStyleConfigBuilder.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation

#if os(macOS)
    import AppKit
#elseif os(iOS)
    import UIKit
#endif

struct StyleConfigEntry: Encodable {
    let key: String
    let config: SeatStyleConfig

    enum CodingKeys: String, CodingKey {
        case key
        case config
    }

    func encode(to encoder: any Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(key, forKey: .key)
        try container.encode(config, forKey: .config)
    }
}

@objc(KKSeatStyleConfigBuilder)
public final class SeatStyleConfigBuilder: NSObject {
    private var configs: [String: SeatStyleConfig] = [:]

    @objc override public init() {
        super.init()
    }

    /// 添加圆形样式
    /// - Parameters:
    ///   - styleId: 样式ID（使用[SeatRenderStyleId.compose] 创建）
    ///   - fill: 填充颜色
    ///   - overlay: 选中时的覆盖颜色
    ///   - checkmark: 选中时的勾选颜色
    /// - Returns: SeatStyleConfigBuilder
    @objc @discardableResult
    public func addCircleStyle(
        styleId: String,
        fill: PlatformColor,
        overlay: PlatformColor,
        checkmark: PlatformColor
    ) -> SeatStyleConfigBuilder {
        guard !styleId.isEmpty else {
            return self
        }
        let config = CircleSeatStyleConfig(fill: fill, overlay: overlay, checkmark: checkmark)
        configs[styleId] = config
        return self
    }

    /// 添加圆形样式
    /// - Parameters:
    ///   - styleId: 样式ID（使用[SeatRenderStyleId.compose] 创建）
    ///   - fill: 填充颜色
    ///   - overlay: 选中时的覆盖颜色
    ///   - checkmark: 选中时的勾选颜色
    /// - Returns: SeatStyleConfigBuilder
    @discardableResult
    public func addCircleStyle(
        styleId: String,
        fill: PlatformColor,
        overlay: PlatformColor? = nil,
        checkmark: PlatformColor? = nil
    ) -> SeatStyleConfigBuilder {
        guard !styleId.isEmpty else {
            return self
        }
        let config = CircleSeatStyleConfig(fill: fill, overlay: overlay, checkmark: checkmark)
        configs[styleId] = config
        return self
    }

    /// 添加SVG样式
    /// - Parameters:
    ///   - styleId: 样式ID（使用[SeatRenderStyleId.compose] 创建）
    ///   - content: svg内容
    /// - Returns: SeatStyleConfigBuilder
    @objc @discardableResult
    public func addSVGStyle(
        styleId: String,
        content: String
    ) -> SeatStyleConfigBuilder {
        guard !styleId.isEmpty else {
            return self
        }
        let config = SVGSeatStyleConfig(content: content)
        configs[styleId] = config
        return self
    }

    /// 序列化为json数据
    /// - Returns: json data
    @objc
    public func toJSONData() -> Data? {
        let entries = configs.compactMap { key, config -> StyleConfigEntry? in
            return StyleConfigEntry(key: key, config: config)
        }

        do {
            let encoder = JSONEncoder()
            encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
            return try encoder.encode(entries)
        } catch {
            return nil
        }
    }

    /// 序列化为json字符串
    /// - Returns: json 字符串
    @objc
    public func toJSONString() -> String? {
        guard let data = toJSONData() else {
            return nil
        }
        return String(data: data, encoding: .utf8)
    }

    /// 清除
    @objc public func clear() {
        configs.removeAll()
    }
}
