//
//  UniqueID.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef UniqueID_h
#define UniqueID_h

#include <cstdint>

namespace kk {
class UniqueID {
  public:
    static uint32_t Next();
};
};  // namespace kk

#endif  // UniqueID_h
