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
 *          for MW320 platforms using the NXP MW320 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/TimeUtils.h>
#include <lwip/tcpip.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/nxp/mw320/DeviceInfoProviderImpl.h>
#include <platform/nxp/mw320/DiagnosticDataProviderImpl.h>
#if defined(MBEDTLS_USE_TINYCRYPT)
#include "ecc.h"
#endif

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;
#if defined(MBEDTLS_USE_TINYCRYPT)
sys_mutex_t PlatformManagerImpl::rngMutexHandle = NULL;
#endif

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    *olen = len;
    return 0;
}

CHIP_ERROR InitClock_RealTime()
{
    System::Clock::Microseconds64 curTime =
        System::Clock::Microseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * UINT64_C(1000000)));
    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will be always valid.
    return System::SystemClock().SetClock_RealTime(curTime);
}

#if defined(MBEDTLS_USE_TINYCRYPT)
int PlatformManagerImpl::uECC_RNG_Function(uint8_t * dest, unsigned int size)
{
    int res;

    sys_mutex_lock(&rngMutexHandle);
    res = (chip::Crypto::DRBG_get_bytes(dest, size) == CHIP_NO_ERROR) ? size : 0;
    sys_mutex_unlock(&rngMutexHandle);

    return res;
}
#endif

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::MW320Config::Init();
    SuccessOrExit(err);

    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());
    SetDeviceInfoProvider(&DeviceInfoProviderImpl::GetDefaultInstance());

    // Initialize LwIP.
    // tcpip_init(NULL, NULL);

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

#if defined(MBEDTLS_USE_TINYCRYPT)
    /* Set RNG function for tinycrypt operations. */
    err_t ret;
    ret = sys_mutex_new(&rngMutexHandle);
    VerifyOrExit((ERR_OK == ret), err = CHIP_ERROR_NO_MEMORY);
    uECC_set_rng(PlatformManagerImpl::uECC_RNG_Function);
#endif

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    // err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    // SuccessOrExit(err);
    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack());

    ReturnErrorOnFailure(InitClock_RealTime());

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
