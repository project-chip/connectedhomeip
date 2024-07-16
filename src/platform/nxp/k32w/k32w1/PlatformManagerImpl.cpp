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

#include <crypto/CHIPCryptoPAL.h>
#include <openthread-system.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/nxp/k32w/k32w1/ConfigurationManagerImpl.h>
#include <platform/nxp/k32w/k32w1/DiagnosticDataProviderImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif

#include "fsl_component_mem_manager.h"
#include "fwk_platform.h"
#include <openthread/platform/entropy.h>

extern "C" void HAL_ResetMCU(void);

extern uint8_t __data_end__[], m_data0_end[];
memAreaCfg_t data0Heap = { .start_address = (void *) __data_end__, .end_address = (void *) m_data0_end };

#if defined(gAppHighSystemClockFrequency_d) && (gAppHighSystemClockFrequency_d > 0) && defined(USE_SMU2_AS_SYSTEM_MEMORY)
extern "C" void APP_SysInitHook(void)
{
    // NBU has to be initialized before calling this function
    PLATFORM_SetNbuConstraintFrequency(PLATFORM_NBU_MIN_FREQ_64MHZ);
    // Disable low-power on NBU
    PLATFORM_DisableControllerLowPower();
}
#endif

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::InitBoardFwk(void)
{
    mem_status_t memSt = kStatus_MemSuccess;

    SecLib_Init();

    memSt = MEM_RegisterExtendedArea(&data0Heap, NULL, 0U);
    VerifyOrReturnError(memSt == kStatus_MemSuccess, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}
void PlatformManagerImpl::CleanReset()
{
    StopEventLoopTask();
    Shutdown();
#if CHIP_PLAT_NVM_SUPPORT
    NvCompletePendingOperations();
#endif
    HAL_ResetMCU();
}

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the configuration system.
    err = Internal::K32WConfig::Init();
    SuccessOrExit(err);

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Initialize LwIP.
    tcpip_init(NULL, NULL);
#endif

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
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
