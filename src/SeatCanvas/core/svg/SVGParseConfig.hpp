#ifndef SVGParseConfig_hpp
#define SVGParseConfig_hpp

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace tgfx {
class Data;
}

namespace kk::svg {

struct SVGParseConfig {
    std::vector<std::string> zoneIdAttributeNames = {"zoneId"};

    static SVGParseConfig FromJSON(const void *bytes, size_t len);
    static SVGParseConfig FromJSON(std::shared_ptr<tgfx::Data> data);
};

}  // namespace kk::svg

#endif /* SVGParseConfig_hpp */
