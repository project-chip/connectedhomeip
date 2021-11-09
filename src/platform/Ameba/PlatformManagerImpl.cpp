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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for the Ameba platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.cpp>

namespace chip {
namespace DeviceLayer {
namespace Internal {
CHIP_ERROR InitLwIPCoreLock(void);
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

extern "C" {
extern int rtw_get_random_bytes(void * dst, size_t size);
}
static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    *olen = 0;

    if (len == 0)
        return (0);

    rtw_get_random_bytes(output, len);
    *olen = len;

    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{

    CHIP_ERROR err;

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());

    // Make sure the LwIP core lock has been initialized
    err = Internal::InitLwIPCoreLock();

    SuccessOrExit(err);

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

    // TODO Wi-Fi Initialzation currently done through the example app needs to be moved into here.
    // for now we will let this happen that way and assume all is OK

    chip::Crypto::add_entropy_source(app_entropy_source, NULL, 1);

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

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
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

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    currentHeapFree = xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    currentHeapUsed = xPortGetTotalHeapSize() - xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    currentHeapHighWatermark = xPortGetTotalHeapSize() - xPortGetMinimumEverFreeHeapSize();
    return CHIP_NO_ERROR;
}
CHIP_ERROR PlatformManagerImpl::_GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR PlatformManagerImpl::_GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

    if (currentTime >= mStartTime)
    {
        upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - mStartTime).count();
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
        if (ConfigurationMgr().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR PlatformManagerImpl::_GetBootReasons(uint8_t & bootReasons)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReasons(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReasons = static_cast<uint8_t>(reason);
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
