/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "KeyValueStorageTest.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <lib/support/ErrorStr.h>
#include <platform/KeyValueStoreManager.h>

const char * TAG = "persistent-storage";

extern "C" void app_main()
{
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "chip-esp32-persitent-storage-example starting");
    ESP_LOGI(TAG, "=============================================");

    // Run tests
    while (true)
    {
        ESP_LOGI(TAG, "Running Tests:");

        // Partial and offset reads are not currently supported on the ESP32
        // platform, skip these tests,
        chip::RunKvsTest(chip::SKIP_MULTI_READ_TEST);
        vTaskDelay(60000); // Run every minute
    }
}
