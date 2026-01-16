//
//  SeatStyleKey.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#ifndef SeatStyleKey_hpp
#define SeatStyleKey_hpp

#include <cstddef>
#include <cstdint>
#include <functional>

namespace kk {
/**
 * 座位样式键
 * 由业务层定义的状态值 + 是否选中组成，用于在渲染器中查找对应的样式
 */
class SeatStyleKey {
  public:
    /**
     * 构造函数
     * @param status 座位状态（由业务层定义，可以是任意 uint32_t 值）
     * @param selected 是否选中
     */
    explicit SeatStyleKey(uint32_t status = 0, bool selected = false);

    //    SeatStyleKey(const SeatStyleKey &other) = default;
    //    SeatStyleKey &operator=(const SeatStyleKey &other) = default;

    // 支持 std::map (需要 operator<)
    bool operator<(const SeatStyleKey &other) const;

    // 支持 std::unordered_map (需要 operator==)
    bool operator==(const SeatStyleKey &other) const;

    /**
     * 获取座位状态（由业务层定义）
     */
    uint32_t getStatus() const {
        return status;
    }

    /**
     * 是否选中
     */
    bool isSelected() const {
        return selected;
    }

    std::size_t hash() const;

  private:
    uint32_t status;  // 座位状态，由业务层定义
    bool selected;    // 是否选中
};
};  // namespace kk

namespace std {
template <>
struct hash<kk::SeatStyleKey> {
    size_t operator()(const kk::SeatStyleKey &key) const {
        return key.hash();
    }
};
}  // namespace std

#endif /* SeatStyleKey_hpp */
