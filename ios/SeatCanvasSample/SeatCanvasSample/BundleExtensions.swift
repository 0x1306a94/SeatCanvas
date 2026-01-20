//
//  BundleExtensions.swift
//  SeatCanvasSample
//
//  Created by king on 2026/1/20.
//

import Foundation

extension Bundle {
    static var Sample: Bundle = {
        let path = Bundle.main.path(forResource: "SeatCanvasSample", ofType: "bundle")!
        let bundle = Bundle(path: path)!
        return bundle
    }()

    func defaultSeatStyleURL(name: String) -> URL {
        var url = bundleURL
        url = url.appendingPathComponent("\(ResourceScope.default.rawValue)/seatstyle")
        url = url.appendingPathComponent(name)
        return url
    }

    func defaultBaseMaps() -> [BaseMapFileInfo] {
        let rootPath = "\(bundlePath)/\(ResourceScope.default.rawValue)/basemap"
        return (try? FileManager.default.contentsOfDirectory(atPath: rootPath)
            .compactMap {
                let path = "\(rootPath)/\($0)"
                return BaseMapFileInfo(scope: ResourceScope.default, url: URL(fileURLWithPath: path))
            }) ?? []
    }

    func customizedBaseMaps() -> [BaseMapFileInfo] {
        let rootPath = "\(bundlePath)/\(ResourceScope.customized.rawValue)/basemap"
        return (try? FileManager.default.contentsOfDirectory(atPath: rootPath)
            .compactMap {
                let path = "\(rootPath)/\($0)"
                return BaseMapFileInfo(scope: ResourceScope.customized, url: URL(fileURLWithPath: path))
            }) ?? []
    }

    func zoneDataURL(scope: ResourceScope, name: String) -> URL {
        let path = "\(bundlePath)/\(scope.rawValue)/zonedata/\(name.replacingOccurrences(of: ".svg", with: ".json"))"
        return URL(fileURLWithPath: path)
    }

    func seatDataURL(scope: ResourceScope, name: String) -> URL {
        let path = "\(bundlePath)/\(scope.rawValue)/seatdata/\(name.replacingOccurrences(of: ".svg", with: ".json"))"
        return URL(fileURLWithPath: path)
    }
}
