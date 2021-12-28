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

#include "WakeOnLanManager.h"

#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app::Clusters::WakeOnLan;

WakeOnLanManager WakeOnLanManager::sInstance;

namespace {
static WakeOnLanManager wakeOnLanManager;
} // namespace

/** @brief Wake On LAN Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfWakeOnLanClusterInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: WakeOnLan::SetDefaultDelegate");
    chip::app::Clusters::WakeOnLan::SetDefaultDelegate(endpoint, &wakeOnLanManager);
}

chip::CharSpan WakeOnLanManager::HandleGetMacAddress()
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    chip::CharSpan macValue;

    ChipLogProgress(Zcl, "Received WakeOnLanManager::HandleGetMacAddress");
    VerifyOrExit(mWakeOnLanManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMacMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        env->ExceptionClear();
        jobject javaMac = env->CallObjectMethod(mWakeOnLanManagerObject, mGetMacMethod, static_cast<jint>(1));
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in WakeOnLanManager::getMac");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return macValue;
        }

        macValue = chip::JniUtfString(env, static_cast<jstring>(javaMac)).charSpan();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "WakeOnLanManager::HandleGetMacAddress status error: %s", err.AsString());
    }

    return macValue;
}

void WakeOnLanManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for WakeOnLanManager"));

    mWakeOnLanManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mWakeOnLanManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef WakeOnLanManager"));

    jclass WakeOnLanManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(WakeOnLanManagerClass != nullptr, ChipLogError(Zcl, "Failed to get WakeOnLanManager Java class"));

    mGetMacMethod = env->GetMethodID(WakeOnLanManagerClass, "getMac", "(I)Ljava/lang/String;");
    if (mGetMacMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access WakeOnLanManager 'getMac' method");
        env->ExceptionClear();
    }
}
