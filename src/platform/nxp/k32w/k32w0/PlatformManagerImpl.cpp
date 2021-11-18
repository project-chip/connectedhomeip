/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for K32W platforms using the NXP K32W SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.cpp>

#include <lwip/tcpip.h>

#include <openthread/platform/entropy.h>

#include "fsl_power.h"

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    otError otErr = otPlatEntropyGet(output, (uint16_t) len);

    if (otErr != OT_ERROR_NONE)
    {
        return -1;
    }

    *olen = len;
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    mStartTimeMilliseconds = System::SystemClock().GetMonotonicMilliseconds();

    // Initialize the configuration system.
    err = Internal::K32WConfig::Init();
    SuccessOrExit(err);

    // Initialize LwIP.
    tcpip_init(NULL, NULL);

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (_GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgrImpl().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgrImpl().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    return Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

CHIP_ERROR PlatformManagerImpl::_GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgrImpl().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR PlatformManagerImpl::_GetUpTime(uint64_t & upTime)
{
    uint64_t currentTimeMilliseconds = System::SystemClock().GetMonotonicMilliseconds();

    if (currentTimeMilliseconds >= mStartTimeMilliseconds)
    {
        upTime = (currentTimeMilliseconds - mStartTimeMilliseconds) / 1000;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR PlatformManagerImpl::_GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (_GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgrImpl().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR PlatformManagerImpl::_GetBootReasons(uint8_t & bootReason)
{
    bootReason = EMBER_ZCL_BOOT_REASON_TYPE_UNSPECIFIED;
    uint8_t reason;
    reason = POWER_GetResetCause();

    if (reason == RESET_UNDEFINED)
    {
        bootReason = EMBER_ZCL_BOOT_REASON_TYPE_UNSPECIFIED;
    }
    else if ((reason == RESET_POR) || (reason == RESET_EXT_PIN))
    {
        bootReason = EMBER_ZCL_BOOT_REASON_TYPE_POWER_ON_REBOOT;
    }
    else if (reason == RESET_BOR)
    {
        bootReason = EMBER_ZCL_BOOT_REASON_TYPE_BROWN_OUT_RESET;
    }
    else if (reason == RESET_SW_REQ)
    {
        bootReason = EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_RESET;
    }
    else if (reason == RESET_WDT)
    {
        bootReason = EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_WATCHDOG_RESET;
        /* Reboot can be due to hardware or software watchdog*/
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize;

    freeHeapSize    = xPortGetFreeHeapSize();
    currentHeapFree = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    size_t freeHeapSize;
    size_t usedHeapSize;

    freeHeapSize = xPortGetFreeHeapSize();
    usedHeapSize = HEAP_SIZE - freeHeapSize;

    currentHeapUsed = static_cast<uint64_t>(usedHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    size_t highWatermarkHeapSize;

    highWatermarkHeapSize    = HEAP_SIZE - xPortGetMinimumEverFreeHeapSize();
    currentHeapHighWatermark = static_cast<uint64_t>(highWatermarkHeapSize);
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
