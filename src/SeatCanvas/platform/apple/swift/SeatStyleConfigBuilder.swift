//
//  SeatStyleConfigBuilder.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation
import UIKit

struct StyleConfigEntry: Encodable {
    let key: SeatStyleKey
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
    private var configs: [SeatStyleKey: SeatStyleConfig] = [:]

    @objc override public init() {
        super.init()
    }

    /// 添加圆形样式配置
    /// - Parameters:
    ///   - status: 座位状态
    ///   - selected: 是否选中
    ///   - fill: 填充颜色
    ///   - overlay: 覆盖层颜色（选中时显示）
    ///   - checkmark: 勾选标记颜色
    /// - Returns: Builder 实例，支持链式调用
    @objc @discardableResult
    public func addCircleStyle(
        status: UInt32,
        selected: Bool,
        fill: UIColor,
        overlay: UIColor,
        checkmark: UIColor
    ) -> SeatStyleConfigBuilder {
        let key = SeatStyleKey(status: status, selected: selected)
        let config = CircleSeatStyleConfig(fill: fill, overlay: overlay, checkmark: checkmark)
        configs[key] = config
        return self
    }

    /// 添加 SVG 样式配置
    /// - Parameters:
    ///   - status: 座位状态
    ///   - selected: 是否选中
    ///   - content: svg 内容
    /// - Returns: Builder 实例，支持链式调用
    @objc @discardableResult
    public func addSVGStyle(
        status: UInt32,
        selected: Bool,
        content: String
    ) -> SeatStyleConfigBuilder {
        let key = SeatStyleKey(status: status, selected: selected)
        let config = SVGSeatStyleConfig(content: content)
        configs[key] = config
        return self
    }

    /// 序列化为 JSON 数据
    /// - Returns: JSON 数据，如果序列化失败则返回 nil
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

    /// 序列化为 JSON 字符串
    /// - Returns: JSON 字符串，如果序列化失败则返回 nil
    @objc
    public func toJSONString() -> String? {
        guard let data = toJSONData() else {
            return nil
        }
        return String(data: data, encoding: .utf8)
    }

    /// 清空所有配置
    @objc public func clear() {
        configs.removeAll()
    }
}
