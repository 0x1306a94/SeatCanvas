//
//  DisplayLink.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#ifndef DisplayLink_hpp
#define DisplayLink_hpp

namespace kk {
class DisplayLink {
  public:
    virtual ~DisplayLink() = default;

    virtual void start() = 0;

    virtual void stop() = 0;
};
};  // namespace kk

#endif /* DisplayLink_hpp */
