//
//  PlatformTypes.h
//  SeatCanvasKit
//
//  Created by king on 2026/6/10.
//

#ifndef PlatformTypes_h
#define PlatformTypes_h

#import <TargetConditionals.h>

#if TARGET_OS_IOS

#import <UIKit/UIColor.h>

#define PlatformColor UIColor

#elif TARGET_OS_MAC

#import <AppKit/NSColor.h>

#define PlatformColor NSColor

#endif

#endif /* PlatformTypes_h */
