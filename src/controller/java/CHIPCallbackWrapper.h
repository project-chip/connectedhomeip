/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <controller/java/AndroidClusterExceptions.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/PlatformManager.h>
#include <zap-generated/CHIPClientCallbacks.h>

namespace chip {

typedef std::function<CHIP_ERROR(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure)> CHIPActionBlock;

template <class T>
class CHIPCallbackWrapper
{
public:
    CHIPCallbackWrapper(jobject javaCallback, chip::CHIPActionBlock action, T onSuccessFn, bool keepAlive) :
        mKeepAlive(keepAlive), mSuccess(onSuccessFn, this), mFailure(OnFailureFn, this)
    {
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        if (env == nullptr)
        {
            ChipLogError(Zcl, "Could not create global reference for Java callback");
            return;
        }

        mJavaCallbackRef = env->NewGlobalRef(javaCallback);
        if (mJavaCallbackRef == nullptr)
        {
            ChipLogError(Zcl, "Could not create global reference for Java callback");
        }

        CHIP_ERROR err = action(mSuccess.Cancel(), mFailure.Cancel());

        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(Zcl, "Failed to invoke action: %s", chip::ErrorStr(err));
            chip::AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, mJavaCallbackRef,
                                                                                      "Failed to invoke action: ", err);
        }
    };

    virtual ~CHIPCallbackWrapper()
    {
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        if (env == nullptr)
        {
            ChipLogError(Zcl, "Could not delete global reference for Java callback");
            return;
        }
        env->DeleteGlobalRef(mJavaCallbackRef);
    };

    jobject JavaCallback() { return mJavaCallbackRef; }

    bool KeepAlive() { return mKeepAlive; }

    static void OnFailureFn(void * context, uint8_t status)
    {
        chip::DeviceLayer::StackUnlock unlock;
        CHIP_ERROR err = CHIP_NO_ERROR;
        JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Missing JNIEnv"));

        auto cppCallback = std::unique_ptr<CHIPCallbackWrapper, void (*)(CHIPCallbackWrapper *)>(
            reinterpret_cast<CHIPCallbackWrapper *>(context), chip::Platform::Delete<CHIPCallbackWrapper>);
        VerifyOrReturn(cppCallback.get() != nullptr, ChipLogProgress(Zcl, "Unable to cast context to callback object"));

        // It's valid for javaCallbackRef to be nullptr if the Java code passed in a null callback.
        jobject javaCallbackRef = cppCallback->JavaCallback();
        VerifyOrReturn(javaCallbackRef != nullptr, ChipLogProgress(Zcl, "No Java callback assigned"));

        jmethodID javaMethod;
        err =
            chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onError", "(Ljava/lang/Exception;)V", &javaMethod);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Failed to find Java onError()"));

        jthrowable exception;
        err = chip::AndroidClusterExceptions::GetInstance().CreateChipClusterException(env, status, exception);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Failed to create ChipClusterException"));

        env->ExceptionClear();
        env->CallVoidMethod(javaCallbackRef, javaMethod, exception);
    }

private:
    jobject mJavaCallbackRef;
    bool mKeepAlive;

    chip::Callback::Callback<T> mSuccess;
    chip::Callback::Callback<DefaultFailureCallback> mFailure;
};

} // namespace chip
