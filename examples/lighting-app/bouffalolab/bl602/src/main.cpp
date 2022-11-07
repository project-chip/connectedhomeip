/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include <DeviceInfoProviderImpl.h>
#include <FreeRTOS.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/bouffalolab/BL602/NetworkCommissioningDriver.h>
#include <task.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <lib/support/ErrorStr.h>

#if PW_RPC_ENABLED
#include "PigweedLogger.h"
#include "Rpc.h"
#endif

#include <InitPlatform.h>

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;

void appError(int err)
{
    log_error("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!\r\n", err);
    portDISABLE_INTERRUPTS();
    while (true)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

static const char * TAG = "light-app";

static DeviceCallbacks EchoCallbacks;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::BLWiFiDriver::GetInstance()));
} // namespace

extern "C" int main()
{
    InitPlatform();

#if PW_RPC_ENABLED
    PigweedLogger::init();
#endif

    log_info("==================================================\r\n");
    log_info("chip-bl602-lighting-example starting\r\n");
    log_info("==================================================\r\n");

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif

    log_info("------------------------Starting App Task---------------------------\r\n");

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    SetDeviceInfoProvider(&DeviceInfoProviderImpl::GetDefaultInstance());
    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        log_info("device init failed: %s", ErrorStr(error));
        return;
    }

    CHIP_ERROR err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        log_error("GetAppTask().Init() failed\r\n");
        return 1;
    }
}
