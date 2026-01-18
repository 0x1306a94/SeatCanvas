//
//  SeatStyleConfig.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation

protocol SeatStyleConfig: Encodable {
    var type: SeatStyleType { get }
}
