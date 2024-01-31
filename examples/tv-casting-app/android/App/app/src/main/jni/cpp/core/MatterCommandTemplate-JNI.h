/*
 *
 *    Copyright (c) 2020-24 Project CHIP Authors
 *    All rights reserved.
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

#include "../support/Converters-JNI.h"

#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <functional>

namespace matter {
namespace casting {
namespace core {

template <typename Type>
class MatterCommandTemplateJNI
{
public:
    virtual ~MatterCommandTemplateJNI() {}

    MatterCommandTemplateJNI(jobject jCommand)
    {
        ChipLogProgress(AppServer, "<MatterCommandTemplateJNI>() called");
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

        mCompletableFutureClass       = env->FindClass("java/util/concurrent/CompletableFuture");
        mCompletableFutureConstructor = env->GetMethodID(mCompletableFutureClass, "<init>", "()V");

        mCompleteMethod = env->GetMethodID(mCompletableFutureClass, "complete", "(Ljava/lang/Object;)Z");
        VerifyOrReturn(mCompleteMethod != nullptr,
                       ChipLogError(AppServer, "<MatterCommandTemplateJNI()> Could not get completeMethod"));

        mCompleteExceptionallyMethod =
            env->GetMethodID(mCompletableFutureClass, "completeExceptionally", "(Ljava/lang/Throwable;)Z");
        VerifyOrReturn(mCompleteExceptionallyMethod != nullptr,
                       ChipLogError(AppServer, "<MatterCommandTemplateJNI()> Could not get completeExceptionallyMethod"));

        mThrowableClass = env->FindClass("java/lang/RuntimeException");
        VerifyOrReturn(mThrowableClass != nullptr,
                       ChipLogError(AppServer, "<MatterCommandTemplateJNI()> Could not find throwableClass"));
        mThrowableConstructor = env->GetMethodID(mThrowableClass, "<init>", "(Ljava/lang/String;)V");
        VerifyOrReturn(mThrowableConstructor != nullptr,
                       ChipLogError(AppServer, "<MatterCommandTemplateJNI()> Could not get throwableConstructor"));

        mCppCommand = support::convertCommandFromJavaToCpp(jCommand);
        VerifyOrReturn(mCppCommand != nullptr,
                       ChipLogError(AppServer, "<MatterCommandTemplateJNI>() Could not get mCppCommand from jCommand"));
    }

    virtual CHIP_ERROR GetCppRequestFromJava(jobject inRequest, Type & outRequest) = 0;

    virtual jobject GetJResponseFromCpp(const typename Type::ResponseType * response) = 0;

    jobject Invoke(jobject jRequest, jobject jTimedInvokeTimeoutMs)
    {
        VerifyOrReturnValue(mCppCommand != nullptr, nullptr,
                            ChipLogError(AppServer, "<MatterCommandTemplateJNI::invoke()> mCppCommand not set"));
        ChipLogProgress(AppServer, "<MatterCommandTemplateJNI::invoke()> called");

        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

        // Setup completableFuture
        jobject completableFuture = env->NewGlobalRef(env->NewObject(mCompletableFutureClass, mCompletableFutureConstructor));
        VerifyOrReturnValue(
            completableFuture != nullptr, nullptr,
            ChipLogError(AppServer, "<MatterCommandTemplateJNI::invoke()> Could not instantiate completableFuture"));

        matter::casting::core::Command<Type> * cppCommand = static_cast<matter::casting::core::Command<Type> *>(mCppCommand);

        // create the LaunchURL request
        Type cppRequest;
        VerifyOrReturnValue(GetCppRequestFromJava(jRequest, cppRequest) == CHIP_NO_ERROR, nullptr);

        // call Invoke on launchURLCommand while passing in success/failure callbacks
        cppCommand->Invoke(
            cppRequest, nullptr,
            [completableFuture, this](void * context, const typename Type::ResponseType & response) {
                ChipLogProgress(AppServer, "<MatterCommandTemplateJNI::invoke()> converting 'response' from Cpp to Java");
                jobject jResponse = GetJResponseFromCpp(&response);

                JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
                chip::DeviceLayer::StackUnlock unlock;
                env->CallBooleanMethod(completableFuture, mCompleteMethod, jResponse);
            },
            [completableFuture, this](void * context, CHIP_ERROR error) {
                JNIEnv * env         = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
                jstring errorMessage = env->NewStringUTF(error.Format());
                VerifyOrReturn(errorMessage != nullptr,
                               ChipLogError(AppServer, "<MatterCommandTemplateJNI::invoke()> Could not create errorMessage"));
                jobject throwableObject = env->NewObject(mThrowableClass, mThrowableConstructor, errorMessage);
                VerifyOrReturn(throwableObject != nullptr,
                               ChipLogError(AppServer, "<MatterCommandTemplateJNI::invoke()> Could not create throwableObject"));

                chip::DeviceLayer::StackUnlock unlock;
                env->CallBooleanMethod(completableFuture, mCompleteExceptionallyMethod, throwableObject);
            },
            chip::MakeOptional(static_cast<unsigned short>(5000)) // time out after kTimedInvokeCommandTimeoutMs
        );

        return completableFuture;
    }

private:
    void * mCppCommand = nullptr;

    jclass mCompletableFutureClass, mThrowableClass;
    jmethodID mCompletableFutureConstructor, mCompleteMethod, mCompleteExceptionallyMethod, mThrowableConstructor;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
