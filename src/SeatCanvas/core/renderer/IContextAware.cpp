//
//  IContextAware.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#include "IContextAware.hpp"

#include <tgfx/gpu/Context.h>

namespace kk::renderer {

bool IContextAware::attachContext(tgfx::Context *context) {
    if (context == nullptr) {
        if (this->context != nullptr) {
            this->context = nullptr;
            this->contextID = 0;
            onResetGPUResources();
            return true;
        }
        return false;
    }

    uint32_t newContextID = context->uniqueID();
    bool contextChanged = (this->context == nullptr) || (this->contextID != newContextID);

    if (contextChanged) {
        this->context = context;
        this->contextID = newContextID;
        onResetGPUResources();
        return true;
    }

    return false;
}

};  // namespace kk::renderer
