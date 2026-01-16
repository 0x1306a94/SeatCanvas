//
//  SeatItemImageKey.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#ifndef SeatItemImageKey_hpp
#define SeatItemImageKey_hpp

#include "core/SeatStatus.h"

#include <cstddef>
#include <functional>

namespace kk {
class SeatItemImageKey {
  public:
    explicit SeatItemImageKey(kk::SeatStatus status, bool selected);

    //    SeatItemImageKey(const SeatItemImageKey &other) = default;
    //    SeatItemImageKey &operator=(const SeatItemImageKey &other) = default;

    // 支持 std::map (需要 operator<)
    bool operator<(const SeatItemImageKey &other) const;

    // 支持 std::unordered_map (需要 operator==)
    bool operator==(const SeatItemImageKey &other) const;

    kk::SeatStatus getStatus() const {
        return status;
    }

    bool isSelected() const {
        return selected;
    }

    std::size_t hash() const;

  private:
    kk::SeatStatus status;
    bool selected;
};
};  // namespace kk

namespace std {
template <>
struct hash<kk::SeatItemImageKey> {
    size_t operator()(const kk::SeatItemImageKey &key) const {
        return key.hash();
    }
};
}  // namespace std

#endif /* SeatItemImageKey_hpp */
