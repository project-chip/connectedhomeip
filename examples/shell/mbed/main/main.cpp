/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "mbedtls/platform.h"
#include <ChipShellCollection.h>
#include <DFUManager.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/mbed/Logging.h>

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

int main()
{
    int ret        = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mbed_logging_init();

    ChipLogProgress(NotSpecified, "Mbed shell example application start");

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(Shell, "Mbed TLS platform initialization failed [%d]", ret);
        goto exit;
    }

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Memory initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Chip stack initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

#ifdef MBED_CONF_APP_BLE_DEVICE_NAME
    err = ConnectivityMgr().SetBLEDeviceName(MBED_CONF_APP_BLE_DEVICE_NAME);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Set BLE device name failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }
#endif

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Chip stack start failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    err = GetDFUManager().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "DFU manager initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    // Initialize the default streamer that was linked.
    ret = Engine::Root().Init();
    if (ret)
    {
        ChipLogError(Shell, "Streamer initialization failed [%d]", ret);
        goto exit;
    }

    cmd_misc_init();
    cmd_otcli_init();
    cmd_app_server_init();

    ChipLogProgress(NotSpecified, "Mbed shell example application run");

    Engine::Root().RunMainLoop();

exit:
    ChipLogProgress(NotSpecified, "Exited with code %d", ret);
    return ret;
}
