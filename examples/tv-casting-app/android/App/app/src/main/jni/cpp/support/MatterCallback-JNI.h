/*
 *
 *    Copyright (c) 2020-24 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include "Converters-JNI.h"

#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace matter {
namespace casting {
namespace support {

template <typename T>
class MatterCallbackJNI
{
public:
    MatterCallbackJNI(std::function<jobject(T)> conversionFn) { mConversionFn = conversionFn; }

    MatterCallbackJNI(const char * methodSignature, std::function<jobject(T)> conversionFn)
    {
        mMethodSignature = methodSignature;
        mConversionFn    = conversionFn;
    }

    CHIP_ERROR SetUp(JNIEnv * env, jobject inCallback)
    {
        ChipLogProgress(AppServer, "MatterCallbackJNI::SetUp called");
        VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NO_ENV, ChipLogError(AppServer, "JNIEnv was null!"));

        ReturnErrorOnFailure(mCallbackObject.Init(inCallback));

        jclass mClazz = env->GetObjectClass(mCallbackObject.ObjectRef());
        VerifyOrReturnError(mClazz != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND,
                            ChipLogError(AppServer, "Failed to get callback Java class"));

        jclass mSuperClazz = env->GetSuperclass(mClazz);
        VerifyOrReturnError(mSuperClazz != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND,
                            ChipLogError(AppServer, "Failed to get callback's parent's Java class"));

        mMethod = env->GetMethodID(mClazz, "handleInternal", mMethodSignature);
        VerifyOrReturnError(
            mMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND,
            ChipLogError(AppServer, "Failed to access 'handleInternal' method with signature %s", mMethodSignature));

        return CHIP_NO_ERROR;
    }

    void Handle(T responseData)
    {
        ChipLogProgress(AppServer, "MatterCallbackJNI::Handle called");

        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "Failed to get JNIEnv"));

        jobject jResponseData = mConversionFn(responseData);

        chip::DeviceLayer::StackUnlock unlock;
        VerifyOrReturn(mCallbackObject.HasValidObjectRef(),
                       ChipLogError(AppServer, "MatterCallbackJNI::Handle mCallbackObject has no valid ObjectRef"));
        VerifyOrReturn(mMethod != nullptr, ChipLogError(AppServer, "MatterCallbackJNI::Handle mMethod is nullptr"));
        env->CallVoidMethod(mCallbackObject.ObjectRef(), mMethod, jResponseData);
    }

protected:
    chip::JniGlobalReference mCallbackObject;
    jmethodID mMethod                       = nullptr;
    const char * mMethodSignature           = "(Ljava/lang/Object;)V";
    std::function<jobject(T)> mConversionFn = nullptr;
};

class MatterFailureCallbackJNI : public MatterCallbackJNI<CHIP_ERROR>
{
public:
    MatterFailureCallbackJNI() : MatterCallbackJNI(matter::casting::support::convertMatterErrorFromCppToJava) {}
};

}; // namespace support
}; // namespace casting
}; // namespace matter
