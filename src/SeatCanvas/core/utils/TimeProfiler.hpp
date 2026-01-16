//
//  TimeProfiler.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/15.
//

#ifndef TimeProfiler_hpp
#define TimeProfiler_hpp

#if !defined(ENABLE_TIME_PROFILER) && defined(DEBUG)
#define ENABLE_TIME_PROFILER 1
#endif

#if ENABLE_TIME_PROFILER

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace kk::utils {

class TimeProfilerGroup;

/// 代码执行耗时统计工具
/// 支持手动控制或 RAII 模式（作为兜底）
class TimeProfiler {
  public:
    static void Point(const std::string &name);
    /// 构造函数
    /// @param tag 标签，用于标识不同的计时点
    /// @param autoStart 是否自动开始计时，默认为 true
    /// @param autoLog 是否在析构时自动输出日志（如果未手动结束），默认为 true
    /// @param group 所属的计时组，如果提供则会在组中记录该阶段的耗时
    explicit TimeProfiler(const std::string &tag, bool autoStart = true, bool autoLog = true, TimeProfilerGroup *group = nullptr);

    /// 析构函数，如果未手动结束且 autoLog 为 true，则自动结束并输出耗时
    ~TimeProfiler();

    /// 开始计时
    void start();

    /// 结束计时并返回耗时（毫秒）
    /// @return 耗时（毫秒），如果已经结束则返回 0
    double end();

    /// 结束计时并返回耗时（毫秒）（兼容旧接口）
    /// @return 耗时（毫秒）
    double stop() {
        return end();
    }

    /// 获取当前耗时（毫秒），不结束计时
    /// @return 当前耗时（毫秒）
    double elapsed() const;

    /// 重置计时器
    void reset();

    /// 是否已结束
    bool isStopped() const {
        return _stopped;
    }

    /// 是否正在计时
    bool isRunning() const {
        return !_stopped && _started;
    }

    /// 禁用拷贝和移动
    TimeProfiler(const TimeProfiler &) = delete;
    TimeProfiler &operator=(const TimeProfiler &) = delete;
    TimeProfiler(TimeProfiler &&) = delete;
    TimeProfiler &operator=(TimeProfiler &&) = delete;

  private:
    void recordToGroup(double elapsedMs);

    std::string _tag;
    bool _autoLog;
    bool _started;
    bool _stopped;
    TimeProfilerGroup *_group;
    std::chrono::high_resolution_clock::time_point _startTime;
};

/// 手动计时工具类
/// 需要手动调用 start() 和 stop()
class ManualTimeProfiler {
  public:
    ManualTimeProfiler() = default;

    /// 开始计时
    void start();

    /// 结束计时并返回耗时（毫秒）
    /// @return 耗时（毫秒）
    double stop();

    /// 获取当前耗时（毫秒），不结束计时
    /// @return 当前耗时（毫秒）
    double elapsed() const;

    /// 重置计时器
    void reset();

    /// 是否正在计时
    bool isRunning() const {
        return _running;
    }

  private:
    bool _running{false};
    std::chrono::high_resolution_clock::time_point _startTime;
};

/// 计时阶段信息
struct TimeProfilerStage {
    std::string name;
    double elapsedMs;
};

/// 计时组工具类
/// 用于组织多个相关的计时点，显示层级关系
/// 支持手动控制或 RAII 模式（作为兜底）
class TimeProfilerGroup {
  public:
    /// 构造函数
    /// @param groupName 组名称
    /// @param autoStart 是否自动开始计时，默认为 true
    /// @param autoLog 是否在析构时自动输出日志（如果未手动结束），默认为 true
    explicit TimeProfilerGroup(const std::string &groupName, bool autoStart = true, bool autoLog = true);

    /// 析构函数，如果未手动结束且 autoLog 为 true，则自动结束并输出层级化的耗时信息
    ~TimeProfilerGroup();

    /// 开始计时
    void start();

    /// 结束计时并输出结果
    /// @return 总耗时（毫秒）
    double end();

    /// 添加一个计时阶段（手动控制模式）
    /// @param stageName 阶段名称
    /// @return TimeProfiler 实例的引用，需要手动调用 start() 和 end()
    TimeProfiler &addStage(const std::string &stageName);

    /// 获取总耗时（毫秒）
    /// @return 总耗时（毫秒）
    double getTotalElapsed() const;

    /// 获取所有阶段的耗时信息
    /// @return 阶段信息列表
    const std::vector<TimeProfilerStage> &getStages() const {
        return _stages;
    }

    /// 是否已结束
    bool isStopped() const {
        return _stopped;
    }

    /// 是否正在计时
    bool isRunning() const {
        return !_stopped && _started;
    }

    /// 设置是否禁用自动输出日志
    void setDisableAutoLog(bool disable);

    /// 设置输出日志的耗时阈值（毫秒）
    /// 仅当总耗时 >= thresholdMs 时才输出，默认 0 表示始终输出
    void setLogThreshold(double thresholdMs);

    /// 禁用拷贝和移动
    TimeProfilerGroup(const TimeProfilerGroup &) = delete;
    TimeProfilerGroup &operator=(const TimeProfilerGroup &) = delete;
    TimeProfilerGroup(TimeProfilerGroup &&) = delete;
    TimeProfilerGroup &operator=(TimeProfilerGroup &&) = delete;

  private:
    void logResults() const;
    std::string getIndent(int level) const;

    std::string _groupName;
    bool _autoLog;
    bool _started;
    bool _stopped;
    std::chrono::high_resolution_clock::time_point _startTime;
    // 保存总耗时（在 end() 时计算）
    double _totalElapsedMs{0.0};
    std::vector<TimeProfilerStage> _stages;
    // 保存活跃的阶段计时器
    std::vector<std::unique_ptr<TimeProfiler>> _activeStages;
    double _logThresholdMs{0.0};
    // 允许 TimeProfiler 访问 _stages
    friend class TimeProfiler;
};

};  // namespace kk::utils

#endif

#if ENABLE_TIME_PROFILER

#define PROFILE_POINT(tag) kk::utils::TimeProfiler::Point(tag)
#define PROFILE_TIME(tag) kk::utils::TimeProfiler _profiler##__COUNTER__(tag);

#define PROFILE_TIME_SILENT(tag) kk::utils::TimeProfiler _profiler##__COUNTER__(tag, true, false);

#define PROFILE_GROUP(griupVar, groupName) kk::utils::TimeProfilerGroup griupVar(groupName);

#define PROFILE_STAGE(group, stageName) auto &_profiler##__COUNTER__ = group.addStage(stageName);

#define PROFILE_GROUP_START(varName, groupName)                          \
    kk::utils::TimeProfilerGroup timer##varName(groupName, false, true); \
    (timer##varName).start();

#define PROFILE_GROUP_END(varName) \
    (timer##varName).end();

#define PROFILE_GROUP_DISABLE_AUTO_LOG(varName) (timer##varName).setDisableAutoLog(true);
#define PROFILE_GROUP_ENABLE_AUTO_LOG(varName) (timer##varName).setDisableAutoLog(false);
#define PROFILE_GROUP_SET_THRESHOLD(varName, thresholdMs) (timer##varName).setLogThreshold(thresholdMs);

#define PROFILE_STAGE_START(groupVar, stageVar, stageName)                   \
    auto &timer##groupVar##stageVar = (timer##groupVar).addStage(stageName); \
    (timer##groupVar##stageVar).start();

#define PROFILE_STAGE_END(groupVar, stageVar) \
    timer##groupVar##stageVar.end();

#else

#define PROFILE_POINT(tag)
#define PROFILE_TIME(tag)
#define PROFILE_TIME_SILENT(tag)
#define PROFILE_GROUP(groupName)
#define PROFILE_STAGE(group, stageName)

#define PROFILE_GROUP_START(varName, groupName)
#define PROFILE_GROUP_END(varName)
#define PROFILE_GROUP_DISABLE_AUTO_LOG(varName)
#define PROFILE_GROUP_ENABLE_AUTO_LOG(varName)
#define PROFILE_GROUP_SET_THRESHOLD(varName, thresholdMs)
#define PROFILE_STAGE_START(groupVar, stageVar, stageName)
#define PROFILE_STAGE_END(groupVar, stageVar)

#endif  // ENABLE_TIME_PROFILER

#endif /* TimeProfiler_hpp */
