//
//  SeatStyleConfigBuilder.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation
import UIKit

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

    @objc @discardableResult
    public func addCircleStyle(
        styleId: String,
        fill: UIColor,
        overlay: UIColor,
        checkmark: UIColor
    ) -> SeatStyleConfigBuilder {
        guard !styleId.isEmpty else {
            return self
        }
        let config = CircleSeatStyleConfig(fill: fill, overlay: overlay, checkmark: checkmark)
        configs[styleId] = config
        return self
    }

    @discardableResult
    public func addCircleStyle(
        styleId: String,
        fill: UIColor,
        overlay: UIColor? = nil,
        checkmark: UIColor? = nil
    ) -> SeatStyleConfigBuilder {
        guard !styleId.isEmpty else {
            return self
        }
        let config = CircleSeatStyleConfig(fill: fill, overlay: overlay, checkmark: checkmark)
        configs[styleId] = config
        return self
    }

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

    @objc
    public func toJSONString() -> String? {
        guard let data = toJSONData() else {
            return nil
        }
        return String(data: data, encoding: .utf8)
    }

    @objc public func clear() {
        configs.removeAll()
    }
}
