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
#include <platform/nxp/k32w/k32w0/DiagnosticDataProviderImpl.h>
#include <platform/nxp/k32w/k32w0/KeyValueStoreManagerImpl.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif

#if defined(MBEDTLS_USE_TINYCRYPT)
#include "ecc.h"
#endif

#include <openthread/platform/entropy.h>

#include "MemManager.h"
#include "PDM.h"
#include "RNG_Interface.h"
#include "SecLib.h"
#include "TimersManager.h"
#include "board.h"
#include "fsl_sha.h"
#include "k32w0-chip-mbedtls-config.h"

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
extern "C" void InitLowPower();
#endif

#if defined(MBEDTLS_USE_TINYCRYPT)
osaMutexId_t PlatformManagerImpl::rngMutexHandle = NULL;
#endif

CHIP_ERROR PlatformManagerImpl::InitBoardFwk(void)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    char initString[] = "app";
    char * argv[1]    = { 0 };
    argv[0]           = &initString[0];

    SHA_ClkInit(SHA_INSTANCE);

    if (MEM_Init() != MEM_SUCCESS_c)
    {
        err = CHIP_ERROR_NO_MEMORY;
        goto exit;
    }

    if (RNG_Init() != gRngSuccess_d)
    {
        err = CHIP_ERROR_RANDOM_DATA_UNAVAILABLE;
        goto exit;
    }
    RNG_SetPseudoRandomNoSeed(NULL);

    SecLib_Init();

    TMR_Init();

    /* Used for OT initializations */
    otSysInit(1, argv);

#if gAdcUsed_d
    BOARD_ADCWakeupInit();
#endif

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    /* Low Power Init */
    InitLowPower();
#endif

exit:
    return err;
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

#if defined(MBEDTLS_USE_TINYCRYPT)
int PlatformManagerImpl::uECC_RNG_Function(uint8_t * dest, unsigned int size)
{
    int res;
    OSA_MutexLock(rngMutexHandle, osaWaitForever_c);
    res = (chip::Crypto::DRBG_get_bytes(dest, size) == CHIP_NO_ERROR) ? size : 0;
    OSA_MutexUnlock(rngMutexHandle);

    return res;
}
#endif

#if CHIP_DEVICE_LAYER_ENABLE_PDM_LOGS
static void PDM_SystemCallback(uint32_t number, PDM_eSystemEventCode code)
{
    uint8_t capacity  = PDM_u8GetSegmentCapacity();
    uint8_t occupancy = PDM_u8GetSegmentOccupancy();
    ChipLogProgress(DeviceLayer, "[PDM]Event (number, code): (%lu, %d)", number, code);
    ChipLogProgress(DeviceLayer, "[PDM]Capacity: %hhu", capacity);
    ChipLogProgress(DeviceLayer, "[PDM]Occupancy: %hhu", occupancy);
}
#endif

static CHIP_ERROR MapPdmInitStatusToChipError(int status)
{
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPlatform, status);
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    uint32_t chipType;
    CHIP_ERROR err = CHIP_NO_ERROR;
    int status;

    /* Initialise the Persistent Data Manager */
    status = PDM_Init();
    SuccessOrExit(err = MapPdmInitStatusToChipError(status));
#if CHIP_DEVICE_LAYER_ENABLE_PDM_LOGS
    PDM_vRegisterSystemCallback(PDM_SystemCallback);
#endif

    // Initialize the configuration system.
    err = Internal::K32WConfig::Init();
    SuccessOrExit(err);

    err = PersistedStorage::KeyValueStoreManagerImpl::Init();
    SuccessOrExit(err);

    chipType = Chip_GetType();
    if ((chipType != CHIP_K32W061) && (chipType != CHIP_K32W041) && (chipType != CHIP_K32W041A) && (chipType != CHIP_K32W041AM))
    {
        err = CHIP_ERROR_INTERNAL;
        ChipLogError(DeviceLayer, "Invalid chip type, expected K32W061");

        goto exit;
    }

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Initialize LwIP.
    tcpip_init(NULL, NULL);
#endif

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

#if defined(MBEDTLS_USE_TINYCRYPT)
    /* Set RNG function for tinycrypt operations. */
    rngMutexHandle = OSA_MutexCreate();
    VerifyOrExit((NULL != rngMutexHandle), err = CHIP_ERROR_NO_MEMORY);
    uECC_set_rng(PlatformManagerImpl::uECC_RNG_Function);
#endif

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
