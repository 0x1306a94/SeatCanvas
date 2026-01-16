//
//  IContextAware.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#ifndef IContextAware_hpp
#define IContextAware_hpp

#include <cstdint>

namespace tgfx {
class Context;
};  // namespace tgfx

namespace kk::renderer {

/**
 * Context 感知接口
 * 所有需要管理 GPU 资源并响应 Context 变化的类都应该实现此接口
 *
 * 使用方式：
 * 1. 继承 IContextAware
 * 2. 实现 onResetGPUResources() 方法，在其中释放所有 GPU 资源
 * 3. 直接调用 attachContext(context) 来更新 Context（会自动检测变化并释放资源）
 *
 * 示例：
 * ```cpp
 * class MyClass : public IContextAware {
 *
 * protected:
 *     void onResetGPUResources() override {
 *         // 释放所有 GPU 资源
 *         myTexture.reset();
 *         myBuffer.reset();
 *     }
 * };
 * ```
 */
class IContextAware {
  public:
    virtual ~IContextAware() = default;

    bool attachContext(tgfx::Context *context);

    tgfx::Context *getContext() const {
        return context;
    }

    uint32_t getContextID() const {
        return contextID;
    }

  protected:
    /**
     * 重置资源（当 Context 变化时调用）
     * 子类应该在此方法中释放所有 GPU 资源（Pipeline、Buffer、Texture、Image 等）
     *
     * @note 此方法会在 attachContext() 检测到 Context 变化时自动调用
     * @note 子类必须实现此方法
     */
    virtual void onResetGPUResources() = 0;

  protected:
    tgfx::Context *context = {nullptr};
    uint32_t contextID = 0;  // Context 的唯一 ID，用于检测变化
};

};  // namespace kk::renderer

#endif /* IContextAware_hpp */
