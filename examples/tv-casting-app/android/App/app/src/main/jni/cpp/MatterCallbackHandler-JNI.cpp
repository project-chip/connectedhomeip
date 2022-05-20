/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MatterCallbackHandler-JNI.h"

#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

CHIP_ERROR MatterCallbackHandlerJNI::SetUp(JNIEnv * env, jobject inHandler)
{
    ChipLogProgress(AppServer, "MatterCallbackHandlerJNI::SetUp called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    mObject = env->NewGlobalRef(inHandler);
    VerifyOrExit(mObject != nullptr, ChipLogError(AppServer, "Failed to NewGlobalRef for handler object"));

    mClazz = env->GetObjectClass(mObject);
    VerifyOrExit(mClazz != nullptr, ChipLogError(AppServer, "Failed to get handler Java class"));

    mMethod = env->GetMethodID(mClazz, "handle", "(ILjava/lang/String;)V");
    if (mMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'handle' method");
        env->ExceptionClear();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp error: %s", err.AsString());
        return err;
    }

    return err;
}

void MatterCallbackHandlerJNI::Handle(CHIP_ERROR callbackErr)
{
    ChipLogProgress(AppServer, "MatterCallbackHandlerJNI::Handle called");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniCallbackErrString(env, callbackErr.AsString());

    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    env->CallVoidMethod(mObject, mMethod, static_cast<jint>(callbackErr.AsInteger()), jniCallbackErrString.jniValue());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "MatterCallbackHandlerJNI::Handle status error: %s", err.AsString());
    }
}
