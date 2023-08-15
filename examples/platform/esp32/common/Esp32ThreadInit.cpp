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

#include <esp_log.h>
#if CONFIG_PM_ENABLE
#include "esp_pm.h"
#endif

using namespace ::chip::DeviceLayer;

static constexpr char TAG[] = "Esp32ThreadInit";

void ESPOpenThreadInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
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
