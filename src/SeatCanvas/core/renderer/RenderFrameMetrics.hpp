//
//  RenderFrameMetrics.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#ifndef RenderFrameMetrics_hpp
#define RenderFrameMetrics_hpp

#include <deque>

namespace kk::renderer {
class RenderFrameMetrics {
  public:
    RenderFrameMetrics() = default;

    /// 当前帧率
    float currentFPS() const;

    /// 最近一帧的绘制耗时（毫秒）
    int64_t lastDrawTime() const;

    /// 时间窗口内的平均绘制耗时（毫秒）
    int64_t averageDrawTime() const;

    /// 是否是第一帧（还没有有效数据）
    bool isFirstFrame() const;

    /// 记录一帧
    /// @param drawTime 本帧绘制耗时（毫秒）
    void recordFrame(int64_t drawTime);

    /// 重置统计数据
    void resetFrames();

  private:
    /// 清理超出时间窗口的旧数据
    void pruneOldFrames(int64_t currentTime);

  private:
    static constexpr int64_t kWindowMs = 1000;
    std::deque<int64_t> _frameTimeStamps = {};
    std::deque<int64_t> _drawTimes = {};
    float _cachedFPS = {0.0f};
};
};  // namespace kk::renderer

#endif /* RenderFrameMetrics_hpp */
