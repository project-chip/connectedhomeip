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

#include "MatterCallbackHandler.h"

#include <lib/support/JniReferences.h>

using namespace chip;

CHIP_ERROR MatterCallbackHandler::SetUp(JNIEnv * env, jobject inHandler)
{
    ChipLogProgress(AppServer, "MatterCallbackHandler::SetUp called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    mObject = env->NewGlobalRef(inHandler);
    VerifyOrExit(mObject != nullptr, ChipLogError(AppServer, "Failed to NewGlobalRef for handler object"));

    mClazz = env->GetObjectClass(mObject);
    VerifyOrExit(mClazz != nullptr, ChipLogError(AppServer, "Failed to get handler Java class"));

    mMethod = env->GetMethodID(mClazz, "handle", "(Z)Z");
    if (mMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'handle' method");
        env->ExceptionClear();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "MatterCallbackHandler::SetUp error: %s", err.AsString());
        return err;
    }

    return err;
}

CHIP_ERROR MatterCallbackHandler::Handle(bool success)
{
    ChipLogProgress(AppServer, "MatterCallbackHandler::Handle called");

    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env->CallBooleanMethod(mObject, mMethod, static_cast<jboolean>(success)) != JNI_FALSE,
                 err = CHIP_ERROR_INCORRECT_STATE);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "MatterCallbackHandler::Handle status error: %s", err.AsString());
    }

    return err;
}
