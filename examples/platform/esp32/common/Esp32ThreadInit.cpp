/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "Esp32ThreadInit.h"
#include <platform/ConnectivityManager.h>
#if CONFIG_OPENTHREAD_ENABLED
#include <platform/ESP32/OpenthreadLauncher.h>
#include <platform/ThreadStackManager.h>
#endif // CONFIG_OPENTHREAD_ENABLED
#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
#include <esp_spiffs.h>
#ifdef CONFIG_AUTO_UPDATE_RCP
#include <esp_ot_rcp_update.h>
#include <esp_rcp_update.h>
#endif
#endif

#include <esp_log.h>
#if CONFIG_PM_ENABLE
#include "esp_pm.h"
#endif

using namespace ::chip::DeviceLayer;

static constexpr char TAG[] = "Esp32ThreadInit";

void ESPOpenThreadInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config  = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
#if defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && defined(CONFIG_AUTO_UPDATE_RCP)
    esp_vfs_spiffs_conf_t rcp_fw_conf = {
        .base_path = "/rcp_fw", .partition_label = "rcp_fw", .max_files = 10, .format_if_mount_failed = false
    };
    if (ESP_OK != esp_vfs_spiffs_register(&rcp_fw_conf))
    {
        ESP_LOGE(TAG, "Failed to mount rcp firmware storage");
        return;
    }
    esp_rcp_update_config_t rcp_update_config = ESP_OPENTHREAD_RCP_UPDATE_CONFIG();
    esp_rcp_update_init(&rcp_update_config);
    esp_ot_register_rcp_handler();
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER && CONFIG_AUTO_UPDATE_RCP
    set_openthread_platform_config(&config);

    if (ThreadStackMgr().InitThreadStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize Thread stack");
        return;
    }
#if defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && defined(CONFIG_AUTO_UPDATE_RCP)
    // TODO: This is a workaround to update the RCP if the Thread is enabled.
    bool thread_was_enabled = ThreadStackMgr().IsThreadEnabled();
    if (thread_was_enabled)
    {
        if (ThreadStackMgr().SetThreadEnabled(false) != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Failed to disable Thread before updating RCP");
            return;
        }
    }

    esp_ot_update_rcp_if_different();

    if (thread_was_enabled)
    {
        if (ThreadStackMgr().SetThreadEnabled(true) != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Failed to enable Thread after updating RCP");
            return;
        }
    }
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER && CONFIG_AUTO_UPDATE_RCP
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    if (ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router) != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to set the Thread device type");
        return;
    }
#elif CHIP_CONFIG_ENABLE_ICD_SERVER
#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = { .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
                                  .min_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
                                  .light_sleep_enable = true
#endif // CONFIG_FREERTOS_USE_TICKLESS_IDLE
    };
    esp_pm_configure(&pm_config);
#endif // CONFIG_PM_ENABLE
    if (ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice) != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to set the Thread device type");
        return;
    }
#else
    if (ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice) != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to set the Thread device type");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD

    if (ThreadStackMgr().StartThreadTask() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to launch Thread task");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
}
