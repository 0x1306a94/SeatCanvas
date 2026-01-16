//
//  SpringBack.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef SpringBack_hpp
#define SpringBack_hpp

#include <optional>

namespace kk::gesture {
class SpringBack {
  public:
    SpringBack();

    void absorb(float velocity, float distance);

    void absorbWithResponse(float velocity, float distance, float response);

    std::optional<float> value(float time) const;

    void reset();

  private:
    float velocityAt(float time) const;

    float _lambda;
    float _c1;
    float _c2;
};

};  // namespace kk::gesture

#endif /* SpringBack_hpp */
