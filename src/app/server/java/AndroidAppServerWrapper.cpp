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

#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <iostream>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <thread>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

CHIP_ERROR ChipAndroidAppInit(AppDelegate * appDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    if (appDelegate != nullptr)
    {
        initParams.appDelegate = appDelegate;
    }

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
        ChipLogProgress(NotSpecified, "Failed to run ChipAndroidAppInit: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return err;
}

void ChipAndroidAppShutdown(void)
{
    chip::Server::GetInstance().Shutdown();
    chip::Platform::MemoryShutdown();
}

void ChipAndroidAppReset(void)
{
    chip::Server::GetInstance().ScheduleFactoryReset();
}
