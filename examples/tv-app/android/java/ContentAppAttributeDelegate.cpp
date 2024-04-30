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
 * @brief Contains Implementation of the ContentAppAttributeDelegate
 */

#include "ContentAppAttributeDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/PlatformManager.h>
#include <zap-generated/endpoint_config.h>

#include <string>

namespace chip {
namespace AppPlatform {

using LaunchResponseType = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;

std::string ContentAppAttributeDelegate::Read(const chip::app::ConcreteReadAttributePath & aPath)
{
    if (aPath.mEndpointId < FIXED_ENDPOINT_COUNT)
    {
        // returning blank string causes the caller to default to output required by the tests
        return "";
    }

    DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ChipLogProgress(Zcl, "ContentAppAttributeDelegate::Read being called for endpoint %d cluster %d attribute %d",
                    aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    jstring resp = static_cast<jstring>(
        env->CallObjectMethod(mContentAppEndpointManager.ObjectRef(), mReadAttributeMethod, static_cast<jint>(aPath.mEndpointId),
                              static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId)));
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Java exception in ContentAppAttributeDelegate::Read");
        env->ExceptionDescribe();
        env->ExceptionClear();
        // returning blank string causes the caller to default to output required by the tests
        return "";
    }
    const char * respStr = env->GetStringUTFChars(resp, 0);
    std::string retStr(respStr);
    env->ReleaseStringUTFChars(resp, respStr);
    env->DeleteLocalRef(resp);
    ChipLogProgress(Zcl, "ContentAppAttributeDelegate::Read got response %s", respStr);
    return retStr;
}

} // namespace AppPlatform
} // namespace chip
