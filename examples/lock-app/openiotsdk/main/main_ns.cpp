/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <stdio.h>
#include <stdlib.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include "openiotsdk_platform.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer;

static void app_thread(void * argument)
{
    CHIP_ERROR error;

    if (openiotsdk_network_init(true))
    {
        ChipLogError(NotSpecified, "Network initialization failed");
        goto exit;
    }

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    error = Server::GetInstance().Init(initParams);
    SuccessOrExit(error);

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork));

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    ChipLogProgress(NotSpecified, "Open IoT SDK lock-app example application run");

    while (true)
    {
        // Add forever delay to ensure proper workload for this thread
        osDelay(osWaitForever);
    }

    Server::GetInstance().Shutdown();

exit:
    osThreadTerminate(osThreadGetId());
}

int main()
{
    ChipLogProgress(NotSpecified, "Open IoT SDK lock-app example application start");

    if (openiotsdk_platform_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK CHIP stack initialization failed");
        return EXIT_FAILURE;
    }

    static const osThreadAttr_t thread_attr = {
        .stack_size = 16 * 1024 // Allocate enough stack for app thread
    };

    osThreadId_t appThread = osThreadNew(app_thread, NULL, &thread_attr);
    if (appThread == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create app thread");
        return EXIT_FAILURE;
    }

    if (openiotsdk_platform_run())
    {
        ChipLogError(NotSpecified, "Open IoT SDK platform run failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
