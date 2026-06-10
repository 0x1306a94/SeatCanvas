//
//  JSeatStatusUpdate.cpp
//  SeatCanvas
//

#include "JSeatStatusUpdate.h"

#include "JNIHelper.hpp"
#include "JStringUtil.hpp"

#include "core/Log.hpp"

namespace kk::jni {

static kk::jni::Global<jclass> SeatStatusUpdateClass;
static jfieldID SeatStatusUpdate_seatId = nullptr;
static jfieldID SeatStatusUpdate_status = nullptr;

void JSeatStatusUpdate::InitJNI(JNIEnv *env) {
    SeatStatusUpdateClass = env->FindClass("com/libseatcanvas/SeatStatusUpdate");
    if (SeatStatusUpdateClass.get() == nullptr) {
        SC_LOG_ERROR("Could not run JSeatStatusUpdate::InitJNI, class is not found!");
        return;
    }
    SeatStatusUpdate_seatId = env->GetFieldID(SeatStatusUpdateClass.get(), "seatId", "Ljava/lang/String;");
    SeatStatusUpdate_status = env->GetFieldID(SeatStatusUpdateClass.get(), "status", "I");
}

std::vector<kk::SeatStatusUpdate> JSeatStatusUpdate::FromJavaArray(JNIEnv *env, jobjectArray array) {
    std::vector<kk::SeatStatusUpdate> updates = {};
    if (env == nullptr || array == nullptr || SeatStatusUpdateClass.get() == nullptr) {
        return updates;
    }

    jsize length = env->GetArrayLength(array);
    updates.reserve(static_cast<size_t>(length));
    for (jsize index = 0; index < length; ++index) {
        jobject element = env->GetObjectArrayElement(array, index);
        if (element == nullptr) {
            continue;
        }
        auto jseatId = static_cast<jstring>(env->GetObjectField(element, SeatStatusUpdate_seatId));
        kk::SeatStatusUpdate update = {};
        update.seatId = SafeConvertToStdString(env, jseatId);
        update.status = static_cast<uint32_t>(env->GetIntField(element, SeatStatusUpdate_status));
        env->DeleteLocalRef(element);
        if (!update.seatId.empty()) {
            updates.push_back(std::move(update));
        }
    }
    return updates;
}

}  // namespace kk::jni
