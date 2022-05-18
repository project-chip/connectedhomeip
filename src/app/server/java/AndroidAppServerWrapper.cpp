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

#include "AndroidAppServerWrapper.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <iostream>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <thread>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

CHIP_ERROR ChipAndroidAppInit(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    err = chip::Server::GetInstance().Init(initParams);
    SuccessOrExit(err);

    if (!IsDeviceAttestationCredentialsProviderSet())
    {
        SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Failed to run ChipAndroidAppInit: %s ", ErrorStr(err));
        return err;
    }
    return err;
}

void ChipAndroidAppShutdown(void)
{
    chip::Server::GetInstance().Shutdown();
    chip::Platform::MemoryShutdown();
}

void GetFabricCount(uint8_t & count)
{
    count = chip::Server::GetInstance().GetFabricTable().FabricCount();
}

jobject ReadFabricList(JNIEnv * env, jobject self)
{
    jobject jFabricList;
    JniReferences::GetInstance().CreateArrayList(jFabricList);
    for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
    {
        if (!fabricInfo.IsInitialized())
            continue;
        jclass jFabricCls       = env->FindClass("chip/appserver/Fabric");
        jmethodID jFabricMethod = env->GetMethodID(jFabricCls, "<init>", "()V");
        jobject jFabric         = env->NewObject(jFabricCls, jFabricMethod);
        jfieldID jvendorId      = env->GetFieldID(jFabricCls, "vendorId", "I");
        jfieldID jfabricId      = env->GetFieldID(jFabricCls, "fabricId", "J");
        jfieldID jnodeId        = env->GetFieldID(jFabricCls, "nodeId", "J");
        jfieldID jfabricIndex   = env->GetFieldID(jFabricCls, "fabricIndex", "S");
        jfieldID jlabel         = env->GetFieldID(jFabricCls, "label", "Ljava/lang/String;");
        env->SetIntField(jFabric, jvendorId, fabricInfo.GetVendorId());
        env->SetLongField(jFabric, jfabricId, fabricInfo.GetFabricId());
        env->SetLongField(jFabric, jnodeId, fabricInfo.GetNodeId());
        env->SetShortField(jFabric, jfabricIndex, fabricInfo.GetFabricIndex());
        UtfString jLabelStr(env, fabricInfo.GetFabricLabel());
        env->SetObjectField(jFabric, jlabel, jLabelStr.jniValue());

        JniReferences::GetInstance().AddToList(jFabricList, jFabric);
    }
    return jFabricList;
}
