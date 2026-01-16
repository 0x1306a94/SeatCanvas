//
//  SystemProperties.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/6.
//

#ifndef SystemProperties_hpp
#define SystemProperties_hpp

namespace kk::utils {

struct SystemProperties {
    float density = 1.0f;
    float fontScale = 1.0f;

    static SystemProperties &Instance() {
        static SystemProperties properties{};
        return properties;
    }

    void updateDensity(float density) {
        this->density = density;
    }

    void updateFontScale(float fontScale) {
        this->fontScale = fontScale;
    }
};

}  // namespace kk::utils

#endif /* SystemProperties_hpp */
