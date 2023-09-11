/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Esp32ThreadInit.h"
#include <platform/ConnectivityManager.h>
#if CONFIG_OPENTHREAD_ENABLED
#include <platform/ESP32/OpenthreadLauncher.h>
#include <platform/ThreadStackManager.h>
#endif // CONFIG_OPENTHREAD_ENABLED

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
    set_openthread_platform_config(&config);

    if (ThreadStackMgr().InitThreadStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize Thread stack");
        return;
    }
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    if (ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router) != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to set the Thread device type");
        return;
    }
#elif CHIP_CONFIG_ENABLE_ICD_SERVER
#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = {
        .max_freq_mhz       = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
        .min_freq_mhz       = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
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
