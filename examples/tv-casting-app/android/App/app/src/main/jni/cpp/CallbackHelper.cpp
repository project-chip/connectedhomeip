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

#include "CallbackHelper.h"

#include <lib/support/JniReferences.h>

using namespace chip;

struct MatterCallbackHandler gCommissioningCompleteHandler;

CHIP_ERROR SetUpMatterCallbackHandler(JNIEnv * env, jobject inHandler, MatterCallbackHandler & callback)
{
    ChipLogProgress(AppServer, "SetUpMatterCallbackHandler called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    callback.object = env->NewGlobalRef(inHandler);
    VerifyOrExit(callback.object != nullptr, ChipLogError(AppServer, "Failed to NewGlobalRef for handler object"));

    callback.clazz = env->GetObjectClass(callback.object);
    VerifyOrExit(callback.clazz != nullptr, ChipLogError(AppServer, "Failed to get handler Java class"));

    callback.method = env->GetMethodID(callback.clazz, "handle", "(Z)Z");
    if (callback.method == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'handle' method");
        env->ExceptionClear();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetUpMatterCallbackHandler error: %s", err.AsString());
        return err;
    }

    return err;
}

CHIP_ERROR CommissioningCompleteHandler()
{
    ChipLogProgress(AppServer, "CommissioningCompleteHandler called");

    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(gCommissioningCompleteHandler.object != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(gCommissioningCompleteHandler.method != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env->CallBooleanMethod(gCommissioningCompleteHandler.object, gCommissioningCompleteHandler.method,
                                        static_cast<jboolean>(true)) != JNI_FALSE,
                 err = CHIP_ERROR_INCORRECT_STATE);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CommissioningCompleteHandler status error: %s", err.AsString());
    }

    return err;
}
