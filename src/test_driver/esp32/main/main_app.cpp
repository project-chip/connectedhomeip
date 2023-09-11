/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_event.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "spi_flash_mmap.h"

#include <stdio.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

const char * TAG = "CHIP-tests";

static void tester_task(void * pvParameters)
{
    ESP_LOGI(TAG, "Starting CHIP tests!");
    int status = RunRegisteredUnitTests();
    ESP_LOGI(TAG, "CHIP test status: %d", status);
    exit(status);
}

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        exit(err);
    }

    tester_task(nullptr);
}
