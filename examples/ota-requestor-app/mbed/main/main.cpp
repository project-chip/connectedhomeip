/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <mbedtls/platform.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/mbed/Logging.h>

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#endif

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::Logging::Platform;

int main()
{
    int ret        = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mbed_logging_init();

#ifdef CAPSENSE_ENABLED
    Capsense::getInstance().init();
#endif

    ChipLogProgress(SoftwareUpdate, "Mbed OTA Requestor example application start");

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(SoftwareUpdate, "Mbed TLS platform initialization failed [%d]", ret);
        goto exit;
    }

    err = MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Memory initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Chip stack initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

#ifdef MBED_CONF_APP_BLE_DEVICE_NAME
    err = ConnectivityMgr().SetBLEDeviceName(MBED_CONF_APP_BLE_DEVICE_NAME);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Set BLE device name failed: %s", err.AsString());
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
    ChipLogProgress(SoftwareUpdate, "Exited with code %d", ret);
    return ret;
}
