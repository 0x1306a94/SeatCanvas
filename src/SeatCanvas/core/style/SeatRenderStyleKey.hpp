//
//  SeatRenderStyleKey.hpp
//  SeatCanvas
//

#ifndef SeatRenderStyleKey_hpp
#define SeatRenderStyleKey_hpp

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>

namespace kk::renderer {

static constexpr uint16_t kNoPricecodeIndex = std::numeric_limits<uint16_t>::max();

struct SeatRenderStyleKey {
    uint16_t pricecodeIndex = kNoPricecodeIndex;
    uint32_t status = 0;
    bool selected = false;

    bool operator==(const SeatRenderStyleKey &other) const;
};

struct SeatRenderStyleKeyHash {
    size_t operator()(const SeatRenderStyleKey &key) const;
};

struct ParsedSeatStyleId {
    std::string pricecode = {};
    uint32_t status = 0;
    bool selected = false;
};

/// 组合座位样式 ID，格式：pricecode_{code}_status_{status}_selected_{0|1}
std::string composeSeatStyleId(const std::string &pricecode, uint32_t status, bool selected);

/// 解析座位样式 ID；格式不匹配时返回 nullopt
std::optional<ParsedSeatStyleId> parseSeatStyleId(const std::string &styleId);

/// 将价档字符串解析为 pricecodeIndex
uint16_t resolvePricecodeIndex(const std::unordered_map<std::string, uint16_t> &pricecodeToIndex, const std::string &pricecode);

}  // namespace kk::renderer

#endif /* SeatRenderStyleKey_hpp */
