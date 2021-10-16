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

#include "CHIPDefaultCallbacks.h"
#include "AndroidClusterExceptions.h"
#include "CHIPCallbackWrapper.h"

#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

void chip::CHIPDefaultSuccessCallback::CallbackFn(void * context)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Missing JNIEnv"));

    std::unique_ptr<chip::CHIPDefaultSuccessCallback, void (*)(chip::CHIPDefaultSuccessCallback *)> cppCallback(
        reinterpret_cast<chip::CHIPDefaultSuccessCallback *>(context), chip::Platform::Delete<chip::CHIPDefaultSuccessCallback>);
    VerifyOrReturn(cppCallback.get() != nullptr, ChipLogProgress(Zcl, "Unable to cast context to callback object"));

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a null callback.
    jobject javaCallbackRef = cppCallback->JavaCallback();
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogProgress(Zcl, "No Java callback assigned"));

    jmethodID javaMethod;
    err = JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "()V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Failed to find Java onError()"));

    env->ExceptionClear();
    env->CallVoidMethod(javaCallbackRef, javaMethod);
}
