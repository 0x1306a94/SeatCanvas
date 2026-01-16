//
//  BaseMapFormat.swift
//  SeatCanvas
//
//  Created by king on 2026/1/14.
//

import Foundation

internal import SeatCanvas_Private

@objc(KKBaseMapFormat)
public enum BaseMapFormat: Int {
    case svg
    case json

    package var cppFormat: kk.parser.BaseMapFormat {
        switch self {
        case .svg: .SVG
        case .json: .JSON
        }
    }
}
