//
//  CPPObject.cpp
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#include "CPPObject.hpp"

#include <cstdlib>

namespace kk::bridge {
CPPObject *_Nonnull AllocCPPObject(ObjectTag tag, void *_Nonnull realValue, CPPObjectDeleterFunc deleter) {
    CPPObject *cppObj = new CPPObject();
    cppObj->tag = tag;
    cppObj->realValue = realValue;
    cppObj->deleter = deleter;
    return cppObj;
}

void ReleaseCPPObject(CPPObject *_Nonnull *_Nonnull obj) {
    if (obj == nullptr || *obj == nullptr) {
        return;
    }

    CPPObject *cppObj = *obj;
    if (cppObj->deleter != nullptr) {
        cppObj->deleter(cppObj->realValue);
    }

    delete cppObj;
    *obj = nullptr;
}
}  // namespace kk::bridge
