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
#include "TvApp-JNI.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app::Clusters::WakeOnLan;

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
    ChipLogProgress(Zcl, "TV Android App: WakeOnLan::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::WakeOnLan::Id, endpoint);
}

void WakeOnLanManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: WakeOnLan::SetDefaultDelegate");
    WakeOnLanManager * mgr = new WakeOnLanManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::WakeOnLan::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

CHIP_ERROR WakeOnLanManager::HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder)
{
    jobject javaMac;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    chip::CharSpan macValue;

    ChipLogProgress(Zcl, "Received WakeOnLanManager::HandleGetMacAddress");
    VerifyOrExit(mWakeOnLanManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMacMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    javaMac = env->CallObjectMethod(mWakeOnLanManagerObject, mGetMacMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in WakeOnLanManager::getMac");
        env->ExceptionDescribe();
        env->ExceptionClear();
        goto exit;
    }

    macValue = chip::JniUtfString(env, static_cast<jstring>(javaMac)).charSpan();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "WakeOnLanManager::HandleGetMacAddress status error: %s", err.AsString());
    }

    return aEncoder.Encode(macValue);
}

void WakeOnLanManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for WakeOnLanManager"));

    mWakeOnLanManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mWakeOnLanManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef WakeOnLanManager"));

    jclass WakeOnLanManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(WakeOnLanManagerClass != nullptr, ChipLogError(Zcl, "Failed to get WakeOnLanManager Java class"));

    mGetMacMethod = env->GetMethodID(WakeOnLanManagerClass, "getMac", "()Ljava/lang/String;");
    if (mGetMacMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access WakeOnLanManager 'getMac' method");
        env->ExceptionClear();
    }
}
