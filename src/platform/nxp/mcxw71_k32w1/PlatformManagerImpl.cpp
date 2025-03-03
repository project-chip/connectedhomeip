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
#include <lib/support/CHIPPlatformMemory.h>
#include <openthread-system.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#ifdef EXTERNAL_CONFIGURATIONMANAGERIMPL_HEADER
#include EXTERNAL_CONFIGURATIONMANAGERIMPL_HEADER
#else
#include <platform/nxp/mcxw71_k32w1/ConfigurationManagerImpl.h>
#endif
#include <platform/nxp/mcxw71_k32w1/DiagnosticDataProviderImpl.h>
#include <platform/nxp/mcxw71_k32w1/SMU2Manager.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif

#include "fsl_component_mem_manager.h"
#include "fwk_platform.h"
#include <openthread/platform/entropy.h>

extern "C" void freertos_mbedtls_mutex_init(void);
extern "C" int mbedtls_platform_set_calloc_free(void * (*calloc_func)(size_t, size_t), void (*free_func)(void *));

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

void PlatformManagerImpl::HardwareInit(void)
{
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    /* Used for HW initializations */
    otSysInit(0, NULL);

    /* Mbedtls Threading support is needed because both
     * Thread and Matter tasks are using it */
    freertos_mbedtls_mutex_init();
}

CHIP_ERROR PlatformManagerImpl::ServiceInit(void)
{
    mem_status_t memSt = kStatus_MemSuccess;

    SecLib_Init();

    memSt = MEM_RegisterExtendedArea(&data0Heap, NULL, 0U);
    VerifyOrReturnError(memSt == kStatus_MemSuccess, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::Reset()
{
#if (CHIP_PLAT_NVM_SUPPORT == 1)
    NvCompletePendingOperations();
#endif
    // Restart the system.
    NVIC_SystemReset();
    while (1)
    {
    }
}

void PlatformManagerImpl::ScheduleResetInIdle(void)
{
    resetInIdle = true;
}

bool PlatformManagerImpl::GetResetInIdleValue(void)
{
    return resetInIdle;
}

extern "C" void initiateResetInIdle(void)
{
    PlatformMgr().Shutdown();
    PlatformMgrImpl().ScheduleResetInIdle();
}

extern "C" void scheduleResetInIdle(void)
{
    PlatformMgrImpl().ScheduleResetInIdle();
}

extern "C" bool getResetInIdleValue(void)
{
    return PlatformMgrImpl().GetResetInIdleValue();
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
    err = Internal::NXPConfig::Init();
    SuccessOrExit(err);

    err = ServiceInit();
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

#if USE_SMU2_DYNAMIC
    ReturnErrorOnFailure(SMU2::Init());
#endif

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

    /* Handle the server shutting down & emit the ShutDown event */
    /* Make sure to call this function from Matter Task */
    PlatformMgr().HandleServerShuttingDown();
    /* Shutdown all layers */
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
