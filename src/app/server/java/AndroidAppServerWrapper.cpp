/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AndroidAppServerWrapper.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <iostream>
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

CHIP_ERROR ChipAndroidAppInit(AppDelegate * appDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
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
