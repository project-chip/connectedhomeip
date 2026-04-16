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

#if CHIP_CRYPTO_PSA
#include <crypto/PSAKeyAllocator.h>
#include "crypto/S200/S200KeyAllocator.h"
#include "psa/crypto.h"
static_assert(CHIP_CONFIG_SHA256_CONTEXT_SIZE == sizeof(psa_hash_operation_t),
              "CHIP_CONFIG_SHA256_CONTEXT_SIZE is too small for psa_hash_operation_t");
#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
#include "mbedtls/threading.h"
#include "threading_alt.h"
#endif
#endif

extern "C" status_t CRYPTO_InitHardware(void);
extern "C" void BOARD_InitAppConsole();

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

    /* Used for HW initializations */
    otSysInit(0, NULL);

status_t crypto_init_status = kStatus_Success;
#if CHIP_CRYPTO_PSA
#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
    config_mbedtls_threading_alt();
#endif /* (MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT) */
    crypto_init_status = psa_crypto_init();
#else
    crypto_init_status = CRYPTO_InitHardware();
#endif /* CHIP_CRYPTO_PSA */

    VerifyOrDieWithMsg(crypto_init_status == kStatus_Success, DeviceLayer,
        "A2 Board Detected: secure subsystem is not supported for A2 boards. Aborting...");
    BOARD_InitAppConsole();
}

CHIP_ERROR PlatformManagerImpl::ServiceInit(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SecLib_Init();

#if !CHIP_CRYPTO_PSA
    err = chip::Crypto::add_entropy_source(plat_entropy_source, NULL, 16);
    SuccessOrExit(err);
#endif

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

#if CHIP_CRYPTO_PSA
    static chip::DeviceLayer::S200KeyAllocator s200KeyAllocator;
#endif

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

#if CHIP_CRYPTO_PSA
    chip::Crypto::SetPSAKeyAllocator(&s200KeyAllocator);
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
            TEMPORARY_RETURN_IGNORED ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                                                   static_cast<uint32_t>(upTime / 3600));
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
