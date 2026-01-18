//
//  SeatStyleKey.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation

struct SeatStyleKey: Encodable, Hashable {
    package let status: UInt32
    package let selected: Bool

    package init(status: UInt32, selected: Bool) {
        self.status = status
        self.selected = selected
    }
}

// MARK: - Hashable

extension SeatStyleKey {
    func hash(into hasher: inout Hasher) {
        hasher.combine(status)
        hasher.combine(selected)
    }
}
