/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"

#include "mbedtls/platform.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/mbed/Logging.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

int main(int argc, char * argv[])
{
    int ret        = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mbed_logging_init();

    ChipLogProgress(NotSpecified, "Mbed all-clusters-minimal-app example application start");

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
