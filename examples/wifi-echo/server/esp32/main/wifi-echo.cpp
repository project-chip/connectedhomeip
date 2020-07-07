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

#include "Button.h"
#include "CHIPDeviceManager.h"
#include "DataModelHandler.h"
#include "Display.h"
#include "EchoDeviceCallbacks.h"
#include "LEDWidget.h"
#include "QRCodeWidget.h"
#include "esp_event_loop.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "tcpip_adapter.h"
#include <stdio.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>
#include <support/ErrorStr.h>
#include <transport/SecureSessionMgr.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

extern void startServer(SecureSessionMgr * transportIPv4, SecureSessionMgr * transportIPv6);
extern void startBle();
#if CONFIG_USE_ECHO_CLIENT
extern void startClient(void);
#endif // CONFIG_USE_ECHO_CLIENT

#if CONFIG_DEVICE_TYPE_M5STACK

#define ATTENTION_BUTTON_GPIO_NUM GPIO_NUM_37 // Use the right button (button "C") as the attention button on M5Stack
#define STATUS_LED_GPIO_NUM GPIO_NUM_MAX      // No status LED on M5Stack

#elif CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#define ATTENTION_BUTTON_GPIO_NUM GPIO_NUM_0 // Use the IO0 button as the attention button on ESP32-DevKitC and compatibles
#define STATUS_LED_GPIO_NUM GPIO_NUM_2       // Use LED1 (blue LED) as status LED on DevKitC

#else // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#error "Unsupported device type selected"

#endif // !CONFIG_DEVICE_TYPE_ESP32_DEVKITC

#if CONFIG_HAVE_DISPLAY

static QRCodeWidget sQRCodeWidget;

// Where to draw the connection status message
#define CONNECTION_MESSAGE 75
// Where to draw the IPv6 information
#define IPV6_INFO 85
#endif // CONFIG_HAVE_DISPLAY

LEDWidget statusLED;
static Button attentionButton;

const char * TAG = "wifi-echo-demo";

static EchoDeviceCallbacks EchoCallbacks;

namespace {

// Globals as these are large and will not fit onto the stack
SecureSessionMgr sTransportIPv4;
SecureSessionMgr sTransportIPv6;

} // namespace

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

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(err));
        return;
    }

    statusLED.Init(STATUS_LED_GPIO_NUM);

    // Start the Echo Server
    InitDataModelHandler();
    startServer(&sTransportIPv4, &sTransportIPv6);

    if (static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE) == RendezvousInformationFlags::kBLE)
    {
        startBle();
    }

#if CONFIG_USE_ECHO_CLIENT
    startClient();
#endif

#if CONFIG_HAVE_DISPLAY

    // Only set up the button for the M5Stack since it's only being used to wake the display right now
    err = attentionButton.Init(ATTENTION_BUTTON_GPIO_NUM, 50);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Button.Init() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the display device.
    err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", ErrorStr(err));
        return;
    }

    // Display the UI widgets.
    ClearDisplay();
    sQRCodeWidget.Display();
    statusLED.Display();

#endif // CONFIG_HAVE_DISPLAY

    // Run the UI Loop
    while (true)
    {
#if CONFIG_HAVE_DISPLAY

        // TODO consider refactoring this example to use FreeRTOS tasks
        // Poll the attention button.  Whenever we detect a *release* of the button
        // reset the display timer
        if (attentionButton.Poll() && !attentionButton.IsPressed())
        {
            WakeDisplay();
        }

#endif // CONFIG_HAVE_DISPLAY

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
