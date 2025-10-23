/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023 NXP
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
 *          for MCXW72 platform using the NXP MCXW72 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <openthread-system.h>
#include <openthread/platform/entropy.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <stdlib.h>

#include "fsl_os_abstraction.h"

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OtaSupport.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "fwk_platform_ot.h"
#endif

extern "C" status_t CRYPTO_InitHardware(void);
extern "C" void BOARD_InitAppConsole();
extern "C" int mbedtls_platform_set_calloc_free(void * (*calloc_func)(size_t, size_t), void (*free_func)(void *));

extern "C" void vApplicationMallocFailedHook(void)
{
    ChipLogError(DeviceLayer, "Malloc Failure");
}

extern "C" void vApplicationIdleHook(void)
{
    chip::DeviceLayer::PlatformManagerImpl::IdleHook();
}

extern "C" void __wrap_exit(int __status)
{
    ChipLogError(DeviceLayer, "======> error exit function !!!");
    assert(0);
}

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

static int plat_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    otError otErr = otPlatEntropyGet(output, (uint16_t) len);
    VerifyOrReturnValue(otErr == OT_ERROR_NONE, -1);

    *olen = len;
    return 0;
}

void PlatformManagerImpl::HardwareInit(void)
{
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    /* Used for HW initializations */
    otSysInit(0, NULL);

    CRYPTO_InitHardware();

    BOARD_InitAppConsole();
}

CHIP_ERROR PlatformManagerImpl::ServiceInit(void)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    SecLib_Init();

    err = chip::Crypto::add_entropy_source(plat_entropy_source, NULL, 16);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    // Initialize the configuration system.
    err = Internal::NXPConfig::Init();
    SuccessOrExit(err);

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Initialize platform services */
    err = ServiceInit();
    SuccessOrExit(err);
#endif

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    err = System::Clock::InitClock_RealTime();
    SuccessOrExit(err);

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

exit:
    return err;
}

void PlatformManagerImpl::SaveSettings(void) {}

void PlatformManagerImpl::IdleHook(void)
{
    bool isResetScheduled = PlatformMgrImpl().GetResetInIdleValue();
    if (isResetScheduled)
    {
        /*
         * In case a reset in IDLE has been scheduled
         * Disable IRQs so that the idle task won't be preempted until the reset
         */
        OSA_InterruptDisable();
    }

    chip::DeviceLayer::Internal::NXPConfig::RunSystemIdleTask();

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* Resume OTA Transactions in Idle task */
    OTA_TransactionResume();
#endif

    /*
     * In case a reset in idle operation has been posted,
     * reset the device after having run the idle function
     */
    if (isResetScheduled)
    {
        PlatformMgrImpl().Reset();
    }
}

void PlatformManagerImpl::Reset(void)
{
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

void PlatformManagerImpl::StopBLEConnectivity(void) {}

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
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node's last reboot");
    }

    /* Handle the server shutting down & emit the ShutDown event*/
    PlatformMgr().HandleServerShuttingDown();
    /* Shutdown all layers */
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
