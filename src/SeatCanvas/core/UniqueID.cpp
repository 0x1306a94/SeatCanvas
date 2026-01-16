//
//  UniqueID.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "UniqueID.h"

#include <atomic>
namespace kk {
static constexpr uint32_t InvalidUniqueID = 0;

uint32_t UniqueID::Next() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == InvalidUniqueID);
    return id;
}
};  // namespace kk