//
//  BaseMapFileInfo.swift
//  SeatCanvasSample
//
//  Created by king on 2026/1/20.
//

import Foundation

enum ResourceScope: String {
    case `default`
    case customized
}

class BaseMapFileInfo {
    let scope: ResourceScope
    let filename: String
    let url: URL
    init(scope: ResourceScope, url: URL) {
        self.scope = scope
        filename = url.lastPathComponent
        self.url = url
    }
}
