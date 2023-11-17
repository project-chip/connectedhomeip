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
#include <platform/Ameba/DiagnosticDataProviderImpl.h>
#include <platform/Ameba/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

namespace chip {
namespace DeviceLayer {
namespace Internal {
CHIP_ERROR InitLwIPCoreLock(void);
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

extern "C" {
extern int rtw_get_random_bytes(void * dst, uint32_t size);
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

    err = System::Clock::InitClock_RealTime();

    SuccessOrExit(err);

exit:
    return err;
}

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
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

    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
