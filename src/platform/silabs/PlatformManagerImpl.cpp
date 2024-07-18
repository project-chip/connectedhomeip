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
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/silabs/DiagnosticDataProviderImpl.h>

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

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Initialize LwIP.
    tcpip_init(NULL, NULL);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

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
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
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
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
void PlatformManagerImpl::HandleWFXSystemEvent(wfx_event_base_t eventBase, sl_wfx_generic_message_t * eventData)
{
    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type                              = DeviceEventType::kWFXSystemEvent;
    event.Platform.WFXSystemEvent.eventBase = eventBase;

    if (eventBase == WIFI_EVENT)
    {
        switch (eventData->header.id)
        {
        case SL_WFX_STARTUP_IND_ID:
            memcpy(&event.Platform.WFXSystemEvent.data.startupEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.startupEvent));
            break;
        case SL_WFX_CONNECT_IND_ID:
            memcpy(&event.Platform.WFXSystemEvent.data.connectEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.connectEvent));
            break;
        case SL_WFX_DISCONNECT_IND_ID:
            memcpy(&event.Platform.WFXSystemEvent.data.disconnectEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.disconnectEvent));
            break;
        // case SL_WFX_RECEIVED_IND_ID:
        //     memcpy(&event.Platform.WFXSystemEvent.data.receivedEvent, eventData,
        //            sizeof(event.Platform.WFXSystemEvent.data.receivedEvent));
        //     break;
        // case SL_WFX_GENERIC_IND_ID:
        //     memcpy(&event.Platform.WFXSystemEvent.data.genericEvent, eventData,
        //            sizeof(event.Platform.WFXSystemEvent.data.genericEvent));
        //     break;
        // case SL_WFX_EXCEPTION_IND_ID:
        //     memcpy(&event.Platform.WFXSystemEvent.data.exceptionEvent, eventData,
        //            sizeof(event.Platform.WFXSystemEvent.data.exceptionEvent));
        //     break;
        // case SL_WFX_ERROR_IND_ID:
        //     memcpy(&event.Platform.WFXSystemEvent.data.errorEvent, eventData,
        //            sizeof(event.Platform.WFXSystemEvent.data.errorEvent));
        //     break;
        default:
            break;
        }
    }
    else if (eventBase == IP_EVENT)
    {
        switch (eventData->header.id)
        {
        case IP_EVENT_STA_GOT_IP:
            memcpy(&event.Platform.WFXSystemEvent.data.genericMsgEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.genericMsgEvent));
            break;
        case IP_EVENT_GOT_IP6:
            memcpy(&event.Platform.WFXSystemEvent.data.genericMsgEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.genericMsgEvent));
            break;
        case IP_EVENT_STA_LOST_IP:
            memcpy(&event.Platform.WFXSystemEvent.data.genericMsgEvent, eventData,
                   sizeof(event.Platform.WFXSystemEvent.data.genericMsgEvent));
            break;
        default:
            break;
        }
    }

    (void) sInstance.PostEvent(&event);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

} // namespace DeviceLayer
} // namespace chip
