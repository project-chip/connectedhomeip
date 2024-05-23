/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ChipAppServerDelegate.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>

using namespace chip;

void ChipAppServerDelegate::OnCommissioningSessionEstablishmentStarted()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningSessionEstablishmentStartedMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningSessionEstablishmentStartedMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningSessionEstablishmentStartedMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningSessionEstablishmentStartedMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ChipAppServerDelegate::OnCommissioningSessionStarted()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningSessionStartedMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningSessionStartedMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningSessionStartedMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningSessionStartedMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ChipAppServerDelegate::OnCommissioningSessionEstablishmentError(CHIP_ERROR err)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningSessionEstablishmentErrorMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningSessionEstablishmentErrorMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningSessionEstablishmentErrorMethod,
                        static_cast<jint>(err.AsInteger()));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningSessionEstablishmentErrorMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ChipAppServerDelegate::OnCommissioningSessionStopped()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningSessionStoppedMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningSessionStoppedMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningSessionStoppedMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningSessionStoppedMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ChipAppServerDelegate::OnCommissioningWindowOpened()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningWindowOpenedMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningWindowOpenedMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningWindowOpenedMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningWindowOpenedMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ChipAppServerDelegate::OnCommissioningWindowClosed()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "JNIEnv is nullptr"));
    VerifyOrReturn(mOnCommissioningWindowClosedMethod != nullptr,
                   ChipLogError(AppServer, "mOnCommissioningWindowClosedMethod is nullptr"));

    env->ExceptionClear();
    VerifyOrReturn(mChipAppServerDelegateObject.HasValidObjectRef(),
                   ChipLogError(AppServer, "mChipAppServerDelegateObject is not valid"));
    env->CallVoidMethod(mChipAppServerDelegateObject.ObjectRef(), mOnCommissioningWindowClosedMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnCommissioningWindowClosedMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

CHIP_ERROR ChipAppServerDelegate::InitializeWithObjects(jobject appDelegateObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);
    ReturnLogErrorOnFailure(mChipAppServerDelegateObject.Init(appDelegateObject));

    jclass chipAppServerDelegateClass = env->GetObjectClass(mChipAppServerDelegateObject.ObjectRef());
    VerifyOrReturnLogError(chipAppServerDelegateClass != nullptr, CHIP_JNI_ERROR_JAVA_ERROR);

    mOnCommissioningSessionEstablishmentStartedMethod =
        env->GetMethodID(chipAppServerDelegateClass, "onCommissioningSessionEstablishmentStarted", "()V");
    VerifyOrReturnLogError(mOnCommissioningSessionEstablishmentStartedMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    mOnCommissioningSessionStartedMethod = env->GetMethodID(chipAppServerDelegateClass, "onCommissioningSessionStarted", "()V");
    VerifyOrReturnLogError(mOnCommissioningSessionStartedMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    mOnCommissioningSessionEstablishmentErrorMethod =
        env->GetMethodID(chipAppServerDelegateClass, "onCommissioningSessionEstablishmentError", "(I)V");
    VerifyOrReturnLogError(mOnCommissioningSessionEstablishmentErrorMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    mOnCommissioningSessionStoppedMethod = env->GetMethodID(chipAppServerDelegateClass, "onCommissioningSessionStopped", "()V");
    VerifyOrReturnLogError(mOnCommissioningSessionStoppedMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    mOnCommissioningWindowOpenedMethod = env->GetMethodID(chipAppServerDelegateClass, "onCommissioningWindowOpened", "()V");
    VerifyOrReturnLogError(mOnCommissioningWindowOpenedMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    mOnCommissioningWindowClosedMethod = env->GetMethodID(chipAppServerDelegateClass, "onCommissioningWindowClosed", "()V");
    VerifyOrReturnLogError(mOnCommissioningWindowClosedMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    return CHIP_NO_ERROR;
}
