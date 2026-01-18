//
//  CPPObject.hpp
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#ifndef CPPObject_hpp
#define CPPObject_hpp

namespace kk::bridge {

enum class ObjectTag {
    CoreRenderer,
    ZoneData,
    ZoneDataBuilder,
    SeatData,
};

using CPPObjectDeleterFunc = void (*_Nullable)(void *_Nonnull);

struct CPPObject {
    ObjectTag tag;
    void *_Nonnull realValue;
    CPPObjectDeleterFunc deleter;
};

template <typename T>
static void CPPObjectDeleter(void *_Nonnull obj) {
    T *typedObj = (T *)obj;
    if (typedObj != nullptr) {
        delete typedObj;
    }
}

CPPObject *_Nonnull AllocCPPObject(ObjectTag tag, void *_Nonnull realValue, CPPObjectDeleterFunc deleter);
void ReleaseCPPObject(CPPObject *_Nonnull *_Nonnull obj);

};  // namespace kk::bridge

#define __GetCPPObjectOrReturn(rawObj, objType, typedObjName, action) \
    if (rawObj == nullptr) {                                          \
        action;                                                       \
    }                                                                 \
    if (rawObj->realValue == nullptr) {                               \
        action;                                                       \
    }                                                                 \
    auto typedObjName = static_cast<objType>(rawObj->realValue);

/* clang-format off */

#define GetCPPObjectOrReturn(rawObj, objType, typedObjName) \
    __GetCPPObjectOrReturn(rawObj, objType, typedObjName, return)


#define GetCPPObjectOrReturnValue(rawObj, objType, typedObjName, value) \
    __GetCPPObjectOrReturn(rawObj, objType, typedObjName, return value)

/* clang-format on */

#endif /* CPPObject_hpp */
