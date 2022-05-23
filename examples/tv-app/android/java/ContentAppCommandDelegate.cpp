/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @brief Contains Implementation of the ContentAppCommandDelegate
 */

#include "ContentAppCommandDelegate.h"

#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {
namespace AppPlatform {

const char * ContentAppCommandDelegate::sendCommand(chip::EndpointId epID, std::string commandPayload)
{
    // to support the hardcoded sample apps.
    if (mSendCommandMethod == nullptr)
    {
        return "Failed";
    }

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jCommandPayload(env, commandPayload.c_str());
    ChipLogProgress(Zcl, "ContentAppCommandDelegate::sendCommand with payload %s", commandPayload.c_str());
    jstring resp = (jstring) env->CallObjectMethod(mContentAppEndpointManager, mSendCommandMethod, static_cast<jint>(epID),
                                                   jCommandPayload.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Java exception in ContentAppCommandDelegate::sendCommand");
        env->ExceptionDescribe();
        env->ExceptionClear();
        // TODO : Need to have proper errors passed back.
        return "Failed";
    }
    const char * ret = env->GetStringUTFChars(resp, 0);
    return ret;
}

} // namespace AppPlatform
} // namespace chip
