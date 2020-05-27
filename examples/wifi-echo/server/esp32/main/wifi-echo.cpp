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

#include "LEDWidget.h"
#include "esp_event_loop.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>

#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

extern void startServer(UDPEndPoint * endpoint);
extern void startClient(void);

#if CONFIG_DEVICE_TYPE_M5STACK

#define ATTENTION_BUTTON_GPIO_NUM GPIO_NUM_37        // Use the right button (button "C") as the attention button on M5Stack
#define STATUS_LED_GPIO_NUM GPIO_NUM_MAX             // No status LED on M5Stack
#define LIGHT_SWITCH_ON_BUTTON_GPIO_NUM GPIO_NUM_39  // Use the left button (button "A") as the light switch ON button on M5Stack
#define LIGHT_SWITCH_OFF_BUTTON_GPIO_NUM GPIO_NUM_38 // Use the middle button (button "B") as the light switch OFF button on M5Stack
#define LIGHT_CONTROLLER_OUTPUT_GPIO_NUM GPIO_NUM_2  // Use GPIO2 as the light controller output on M5Stack

#elif CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#define ATTENTION_BUTTON_GPIO_NUM GPIO_NUM_0         // Use the IO0 button as the attention button on ESP32-DevKitC and compatibles
#define STATUS_LED_GPIO_NUM GPIO_NUM_2               // Use LED1 (blue LED) as status LED on DevKitC
#define LIGHT_SWITCH_ON_BUTTON_GPIO_NUM GPIO_NUM_34  // Use GPIO34 as the light switch ON button input on DevKitC
#define LIGHT_SWITCH_OFF_BUTTON_GPIO_NUM GPIO_NUM_35 // Use GPIO35 as the light switch OFF button input on DevKitC
#define LIGHT_CONTROLLER_OUTPUT_GPIO_NUM GPIO_NUM_33 // Use GPIO33 as the light controller output on DevKitC

#else // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#error "Unsupported device type selected"

#endif // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

static LEDWidget statusLED;

const char * TAG = "wifi-echo-demo";

static void DeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg);

extern "C" void app_main()
{
    ESP_LOGI(TAG, "WiFi Echo Demo!");

    /* Print chip information */
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
        return;
    }

    // Initialize the LwIP core lock.  This must be done before the ESP
    // tcpip_adapter layer is initialized.
    err = PlatformMgrImpl().InitLwIPCoreLock();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().InitLocks() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the ESP tcpip adapter.
    tcpip_adapter_init();

    // Arrange for the ESP event loop to deliver events into the CHIP Device layer.
    err = esp_event_loop_init(PlatformManagerImpl::HandleESPSystemEvent, NULL);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_event_loop_init() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the ESP WiFi layer.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err                    = esp_wifi_init(&cfg);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "esp_event_loop_init() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the CHIP stack.
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().InitChipStack() failed: %s", ErrorStr(err));
        return;
    }

    // Configure the CHIP Connectivity Manager to always enable the AP. The Station interface
    // will be enabled automatically if the required configuration is set.
    ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(DeviceEventHandler, 0);

    // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().StartEventLoopTask() failed: %s", ErrorStr(err));
        return;
    }

    statusLED.Init(STATUS_LED_GPIO_NUM);

    // Start the Echo Server
    UDPEndPoint * sEndpoint = NULL;
    startServer(sEndpoint);
#if CONFIG_USE_ECHO_CLIENT
    startClient();
#endif

    // Run the UI Loop
    while (true)
    {
        statusLED.Blink(500);
        statusLED.Animate();

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/* Handle events from the CHIP Device layer.
 *
 * NOTE: This function runs on the CHIP event loop task.
 */
void DeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceEventType::kSessionEstablished && event->SessionEstablished.IsCommissioner)
    {
        ESP_LOGI(TAG, "Commissioner detected!");
    }
}
