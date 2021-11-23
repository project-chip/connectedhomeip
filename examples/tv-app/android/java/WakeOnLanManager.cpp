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

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>
#include <lib/support/ZclString.h>

#include <inipp/inipp.h>

#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app::Clusters;

WakeOnLanManager WakeOnLanManager::sInstance;

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
    WakeOnLanMgr().InitWakeOnLanCluster(endpoint);
}

void WakeOnLanManager::InitWakeOnLanCluster(chip::EndpointId endpoint)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received WakeOnLanManager::InitWakeOnLanCluster %d", endpoint);
    VerifyOrReturn(mWakeOnLanManagerObject != nullptr, ChipLogError(Zcl, "mWakeOnLanManagerObject null"));
    VerifyOrReturn(mGetMacMethod != nullptr, ChipLogError(Zcl, "mGetMacMethod null"));
    VerifyOrReturn(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();
    jobject javaMac = env->CallObjectMethod(mWakeOnLanManagerObject, mGetMacMethod, static_cast<jint>(endpoint));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in WakeOnLanManager::getMac");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    chip::JniUtfString macValue(env, static_cast<jstring>(javaMac));

    uint8_t bufferMemory[32];
    MutableByteSpan zclString(bufferMemory);
    MakeZclCharString(zclString, macValue.c_str());

    EmberAfStatus macAddressStatus = emberAfWriteServerAttribute(
        endpoint, WakeOnLan::Id, WakeOnLan::Attributes::WakeOnLanMacAddress::Id, zclString.data(), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (macAddressStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store mac address attribute.");
    }
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
