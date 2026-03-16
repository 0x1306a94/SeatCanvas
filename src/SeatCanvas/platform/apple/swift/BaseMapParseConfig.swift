//
//  BaseMapParseConfig.swift
//  SeatCanvas
//
//  Created by KK on 2026/3/16.
//

import Foundation

@objc(KKBaseMapParseConfig)
public protocol BaseMapParseConfig {
    func serializeToData() -> Data?
}

@objc(KKSVGBaseMapParseConfig)
@objcMembers
public final class SVGBaseMapParseConfig: NSObject, BaseMapParseConfig, Encodable {
    public let zoneIdAttributeNames: [String]

    public init(zoneIdAttributeNames: [String] = ["zoneId"]) {
        self.zoneIdAttributeNames = zoneIdAttributeNames
    }

    public func serializeToData() -> Data? {
        return try? JSONEncoder().encode(self)
    }
}
