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
 *          for MT793x platforms using the MT793x SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/mt793x/DiagnosticDataProviderImpl.h>

#include <lwip/tcpip.h>
#include <tcpip_wrapper.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "wifi_api_ex.h"
#endif

extern "C" void PlatformLogPrint(int module, int level, const char * msg, va_list args)
{
    char log[256];
    int i;

    i = vsnprintf(log, sizeof(log), msg, args);
    while ((--i) >= 0)
    {
        if (log[i] == '\n' || log[i] == '\r' || log[i] == ' ')
        {
            log[i] = '\0';
            continue;
        }
        break;
    }
    ChipLogError(DeviceLayer, "[%lu] FILOGIC %s", xTaskGetTickCount(), log);
}

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::MT793XConfig::Init();
    SuccessOrExit(err);
    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    mFilogicCtx = filogic_start_sync();
    assert(mFilogicCtx != NULL);

    assert(filogic_set_logv_callback_sync(mFilogicCtx, PlatformLogPrint));

    filogic_set_event_callback_sync(mFilogicCtx, FilogicEventHandler);

    // Initialize LwIP.
    mtk_tcpip_init(NULL, NULL);

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

void PlatformManagerImpl::FilogicEventHandler(void * c, filogic_async_event_id_t event, filogic_async_event_data * data)
{
    ChipDeviceEvent e = { 0 };

    ChipLogProgress(DeviceLayer, "%s %s", __func__, filogic_event_to_name(event));

    if (event < FILOGIC_EVENT_ID_MAX)
    {
        e.Type = DeviceEventType::kMtkWiFiEvent;
        memcpy(&e.Platform.MtkWiFiEvent.event_data, data, sizeof(*data));
        ChipLogError(DeviceLayer, "event %s", filogic_event_to_name(event));
        (void) sInstance.PostEvent(&e);
    }
    else
    {
        ChipLogError(DeviceLayer, "Unhandled event %d", event);
    }
}

} // namespace DeviceLayer
} // namespace chip
