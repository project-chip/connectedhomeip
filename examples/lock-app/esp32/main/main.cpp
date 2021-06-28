/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "shell_extension/launch.h"
#include <app/server/Server.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <support/ErrorStr.h>

#if CONFIG_ENABLE_PW_RPC
#include "PigweedLogger.h"
#include "Rpc.h"
#endif

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

static const char * TAG = "lock-app";

static DeviceCallbacks EchoCallbacks;

extern "C" void app_main()
{
    int err = 0;
    // Initialize the ESP NVS layer.
    err = nvs_flash_init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", ErrorStr(err));
        return;
    }

#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "chip-esp32-lock-example starting");
    ESP_LOGI(TAG, "==================================================");

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(err));
        return;
    }

    InitServer();

    ESP_LOGI(TAG, "------------------------Starting App Task---------------------------");
    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetAppTask().Init() failed");
    }
}
