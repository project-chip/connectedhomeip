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

#include "esp_event.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include <stdio.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

const char * TAG = "CHIP-tests";

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    esp_fill_random(output, len);
    *olen = len;
    return 0;
}

static void tester_task(void * pvParameters)
{
    ESP_LOGI(TAG, "Starting CHIP tests!");
    int status = RunRegisteredUnitTests();
    ESP_LOGI(TAG, "CHIP test status: %d", status);
    exit(status);
}

extern "C" void app_main()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    CHIP_ERROR err; // A quick note about errors: CHIP adopts the error type and numbering
                    // convention of the environment into which it is ported.  Thus esp_err_t
                    // and CHIP_ERROR are in fact the same type, and both ESP-IDF errors
                    // and CHIO-specific errors can be stored in the same value without
                    // ambiguity.  For convenience, ESP_OK and CHIP_NO_ERROR are mapped
                    // to the same value.

    // Initialize the ESP NVS layer.
    err = nvs_flash_init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", ErrorStr(err));
        exit(err);
    }

    // Initialize the LwIP core lock.  This must be done before the ESP
    // tcpip_adapter layer is initialized.
    err = PlatformMgrImpl().InitLwIPCoreLock();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().InitLocks() failed: %s", ErrorStr(err));
        exit(err);
    }

    err = esp_netif_init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_netif_init() failed: %s", ErrorStr(err));
        exit(err);
    }

    // Arrange for the ESP event loop to deliver events into the CHIP Device layer.
    err = esp_event_loop_create_default();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default() failed: %s", ErrorStr(err));
        exit(err);
    }
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, NULL);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_event_handler_register() failed for WIFI_EVENT: %s", ErrorStr(err));
        exit(err);
    }
    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, NULL);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_event_handler_register() failed for IP_EVENT: %s", ErrorStr(err));
        exit(err);
    }

    err = Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "add_entropy_source() failed: %s", ErrorStr(err));
        exit(err);
    }

    xTaskCreate(tester_task, "tester", 8192, (void *) NULL, tskIDLE_PRIORITY + 10, NULL);

    while (1)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
