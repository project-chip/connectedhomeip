/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppTask.h"

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#endif

#ifdef CHIP_PW_RPC
#include "Rpc.h"
#endif

#include "mbedtls/platform.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/mbed/Logging.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

int main()
{
    int ret        = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mbed_logging_init();

#ifdef CAPSENSE_ENABLED
    Capsense::getInstance().init();
#endif

#if CHIP_PW_RPC
    auto rpcThread = chip::rpc::Init();
    if (rpcThread == NULL)
    {
        ChipLogError(NotSpecified, "RPC service initialization and run failed");
        ret = EXIT_FAILURE;
        goto exit;
    }
#endif

    ChipLogProgress(NotSpecified, "Mbed lock-app example application start");

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(NotSpecified, "Mbed TLS platform initialization failed [%d]", ret);
        goto exit;
    }

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Memory initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

#ifdef MBED_CONF_APP_BLE_DEVICE_NAME
    err = ConnectivityMgr().SetBLEDeviceName(MBED_CONF_APP_BLE_DEVICE_NAME);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Set BLE device name failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }
#endif

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack start failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    ret = GetAppTask().StartApp();

exit:
    ChipLogProgress(NotSpecified, "Exited with code %d", ret);
    return ret;
}
