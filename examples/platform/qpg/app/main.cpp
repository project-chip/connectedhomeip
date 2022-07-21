/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/** @file "main.cpp"
 *
 * Main application.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Qorvo CHIP library
#include "qvCHIP.h"

// CHIP includes
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif // PW_RPC_ENABLED

#if ENABLE_CHIP_SHELL
#include "shell_common/shell.h"
#endif // ENABLE_CHIP_SHELL

// Application level logic
#include "AppTask.h"
#include "ota.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace {
constexpr uint32_t kInitOTARequestorDelaySec = 3;
constexpr int extDiscTimeoutSecs             = 20;
} // namespace

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    External Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Application Function Definitions
 *****************************************************************************/
CHIP_ERROR CHIP_Init(void);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    InitializeOTARequestor();
}
#endif

void Application_Init(void)
{
    CHIP_ERROR error;

    /* Initialize CHIP stack */
    error = CHIP_Init();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "CHIP_Init failed");
        return;
    }

    /* Launch application task */
    ChipLogProgress(NotSpecified, "============================");
    ChipLogProgress(NotSpecified, "Qorvo " APP_NAME " Launching");
    ChipLogProgress(NotSpecified, "============================");

    CHIP_ERROR ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "GetAppTask().Init() failed");
        return;
    }
}

void ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t /* arg */)
{
    switch (aEvent->Type)
    {
    case DeviceEventType::kThreadConnectivityChange:
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        if (aEvent->ThreadConnectivityChange.Result == kConnectivity_Established)
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
        }
#endif
        break;
    default:
        break;
    }
}

CHIP_ERROR CHIP_Init(void)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

#if PW_RPC_ENABLED
    ret = (CHIP_ERROR) chip::rpc::Init();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "rpc::Init() failed");
        goto exit;
    }
#endif

    ret = chip::Platform::MemoryInit();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Platform::MemoryInit() failed");
        goto exit;
    }

#if ENABLE_CHIP_SHELL
    ret = (CHIP_ERROR) ShellTask::Start();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ShellTask::Start() failed");
        goto exit;
    }
#endif // ENABLE_CHIP_SHELL

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().InitChipStack() failed");
        goto exit;
    }

#if CHIP_ENABLE_OPENTHREAD
    ChipLogProgress(NotSpecified, "Initializing OpenThread stack");

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThreadStackMgr().InitThreadStack() failed");
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ConnectivityMgr().SetThreadDeviceType() failed");
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Starting OpenThread task");
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThreadStackMgr().StartThreadTask() failed");
        goto exit;
    }
#endif // CHIP_ENABLE_OPENTHREAD

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        goto exit;
    }

exit:
    return ret;
}

/*****************************************************************************
 * --- Main
 *****************************************************************************/

int main(void)
{
    int result;

    /* Initialize Qorvo stack */
    result = qvCHIP_init(Application_Init);
    if (result < 0)
    {
        return 0;
    }

    /* Start FreeRTOS */
    vTaskStartScheduler();
    return 0;
}
