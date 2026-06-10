//
//  PlatformTypes.swift
//  SeatCanvas
//
//  Created by king on 2026/6/10.
//

#if os(macOS)
    import AppKit

    public typealias PlatformColor = NSColor

#elseif os(iOS)

    import UIKit

    public typealias PlatformColor = UIColor

#endif
