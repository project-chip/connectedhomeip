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
 *          for scm1612s platform using the wise-sdk.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <platform/senscomm/scm1612s/DiagnosticDataProviderImpl.h>

#include "wise_wifi.h"
#include "wise_event_loop.h"
#include "scm_wifi.h"

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
    ChipLogError(DeviceLayer, "[%lu] %s", xTaskGetTickCount(), log);
}

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::SCM1612SConfig::Init();
    SuccessOrExit(err);
    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    (void)scm_wifi_register_event_callback(WiseEventHandler, NULL);

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

wise_err_t PlatformManagerImpl::WiseEventHandler(void * ctx, system_event_t * event)
{
    ChipDeviceEvent e = { 0 };

    ChipLogProgress(DeviceLayer, "%s %d", __func__, event->event_id);

    if (event->event_id < SYSTEM_EVENT_MAX)
    {
        e.Type = DeviceEventType::kSCMSystemEvent;
        memcpy(&e.Platform.SCMSystemEvent.event, event, sizeof(*event));
        ChipLogError(DeviceLayer, "event %d", event->event_id);
        (void) sInstance.PostEvent(&e);

        return WISE_OK;
    }
    else
    {
        ChipLogError(DeviceLayer, "Unhandled event %d", event->event_id);
        return WISE_FAIL;
    }
}

} // namespace DeviceLayer
} // namespace chip
