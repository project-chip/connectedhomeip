/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          for Silabs platforms using the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_CMSISOS.ipp>
#include <platform/silabs/DiagnosticDataProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include <platform/silabs/wifi/WifiInterface.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
#include "tinycrypt/ecc.h"
#endif // SL_MBEDTLS_USE_TINYCRYPT

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
osMutexId_t PlatformManagerImpl::rngMutexHandle = nullptr;

int PlatformManagerImpl::uECC_RNG_Function(uint8_t * dest, unsigned int size)
{
    osMutexAcquire(rngMutexHandle, osWaitForever);
    int res = (chip::Crypto::DRBG_get_bytes(dest, size) == CHIP_NO_ERROR) ? size : 0;
    osMutexRelease(rngMutexHandle);

    return res;
}

#if !(SLI_SI91X_MCU_INTERFACE)
static void app_get_random(uint8_t * aOutput, size_t aLen)
{
    VerifyOrReturn(aOutput != nullptr);
    for (size_t i = 0; i < aLen; i++)
    {
        aOutput[i] = rand();
    }
}

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    app_get_random(reinterpret_cast<uint8_t *>(output), static_cast<uint16_t>(len));
    *olen = len;

    return 0;
}
#endif // !SLI_SI91X_MCU_INTERFACE
#endif // SL_MBEDTLS_USE_TINYCRYPT
CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;
    // Initialize the configuration system.
    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    SuccessOrExit(err);

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !defined(SLI_SI91X_MCU_INTERFACE) && !defined(EXP_BOARD)
    // Initialize LwIP.
    tcpip_init(NULL, NULL);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && !defined(SLI_SI91X_MCU_INTERFACE) && !defined(EXP_BOARD)

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
#if !(SLI_SI91X_MCU_INTERFACE)
    // 16 : Threshold value
    ReturnErrorOnFailure(chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16));
#endif // !SLI_SI91X_MCU_INTERFACE
    /* Set RNG function for tinycrypt operations. */
    rngMutexHandle = osMutexNew(nullptr);
    VerifyOrExit((&rngMutexHandle != nullptr), err = CHIP_ERROR_NO_MEMORY);
    uECC_set_rng(PlatformManagerImpl::uECC_RNG_Function);
#endif // SL_MBEDTLS_USE_TINYCRYPT

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_CMSISOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    // Start timer to increment TotalOperationalHours every hour
    SystemLayer().StartTimer(System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);

exit:
    return err;
}

void PlatformManagerImpl::UpdateOperationalHours(System::Layer * systemLayer, void * appState)
{
    uint32_t totalOperationalHours = 0;

    if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + 1);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    SystemLayer().StartTimer(System::Clock::Seconds32(kSecondsPerHour), UpdateOperationalHours, NULL);
}

void PlatformManagerImpl::_Shutdown()
{
    Internal::GenericPlatformManagerImpl_CMSISOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
// This function needs to be global so it can be used from the platform implementation without depending on the platfrom itself.
// This is a workaround to avoid a circular dependency.
void HandleWFXSystemEvent(sl_wfx_generic_message_t * eventData)
{
    using namespace chip;
    using namespace chip::DeviceLayer;
    using namespace chip::DeviceLayer::Silabs;

    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type = DeviceEventType::kWFXSystemEvent;

    switch (eventData->header.id)
    {
// TODO: Work around until we unify the data structures behind a Matter level common structure
#if WF200_WIFI
    case SL_WFX_STARTUP_IND_ID:
#endif
    case to_underlying(WifiInterface::WifiEvent::kStartUp):
        memcpy(&event.Platform.WFXSystemEvent.data.startupEvent, eventData,
               sizeof(event.Platform.WFXSystemEvent.data.startupEvent));
        // TODO: This is a workaround until we unify the Matter Data structures
        event.Platform.WFXSystemEvent.data.startupEvent.header.id = to_underlying(WifiInterface::WifiEvent::kStartUp);
        break;

    case to_underlying(WifiInterface::WifiEvent::kConnect):
        memcpy(&event.Platform.WFXSystemEvent.data.connectEvent, eventData,
               sizeof(event.Platform.WFXSystemEvent.data.connectEvent));
        break;

    case to_underlying(WifiInterface::WifiEvent::kDisconnect):
        memcpy(&event.Platform.WFXSystemEvent.data.disconnectEvent, eventData,
               sizeof(event.Platform.WFXSystemEvent.data.disconnectEvent));
        break;

    case to_underlying(WifiInterface::WifiEvent::kGotIPv4):
    case to_underlying(WifiInterface::WifiEvent::kLostIP):
    case to_underlying(WifiInterface::WifiEvent::kGotIPv6):
        memcpy(&event.Platform.WFXSystemEvent.data.genericMsgEvent, eventData,
               sizeof(event.Platform.WFXSystemEvent.data.genericMsgEvent));
        break;

    default:
        break;
    }

    // TODO: We should add error processing here
    (void) PlatformMgr().PostEvent(&event);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
