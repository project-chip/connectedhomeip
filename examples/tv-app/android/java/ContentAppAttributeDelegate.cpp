/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Contains Implementation of the ContentAppAttributeDelegate
 */

#include "ContentAppAttributeDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <zap-generated/endpoint_config.h>

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

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ChipLogProgress(Zcl, "ContentAppAttributeDelegate::Read being called for endpoint %d cluster %d attribute %d",
                    aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    jstring resp =
        (jstring) env->CallObjectMethod(mContentAppEndpointManager, mReadAttributeMethod, static_cast<jint>(aPath.mEndpointId),
                                        static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId));
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
