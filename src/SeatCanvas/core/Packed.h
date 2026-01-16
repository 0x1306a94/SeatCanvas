//
//  Packed.h
//  SeatCanvasKit
//
//  Created by king on 2025/12/20.
//

#ifndef Packed_h
#define Packed_h

#ifdef _MSC_VER
#define PACKED_STRUCT __pragma(pack(push, 1)) struct
#define PACKED_END __pragma(pack(pop))
#else
#define PACKED_STRUCT struct __attribute__((packed))
#define PACKED_END
#endif

#endif /* Packed_h */
