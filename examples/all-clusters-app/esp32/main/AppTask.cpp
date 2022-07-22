/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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
#include "Button.h"
#include "DeviceWithDisplay.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "ScreenManager.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include <app/server/OnboardingCodesUtil.h>

#define APP_TASK_NAME "APP"
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (3072)

static const char * TAG = "app-task";

namespace {

QueueHandle_t sAppEventQueue;
TaskHandle_t sAppTaskHandle;

} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate app event queue");
        return APP_ERROR_EVENT_QUEUE_FAILED;
    }

    // Start App task.
    BaseType_t xReturned;
    xReturned = xTaskCreate(AppTaskMain, APP_TASK_NAME, APP_TASK_STACK_SIZE, NULL, 1, &sAppTaskHandle);
    return (xReturned == pdPASS) ? CHIP_NO_ERROR : APP_ERROR_CREATE_TASK_FAILED;
}

CHIP_ERROR AppTask::Init()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);
    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    CHIP_ERROR err = CHIP_NO_ERROR;
    statusLED1.Init(STATUS_LED_GPIO_NUM);
    // Our second LED doesn't map to any physical LEDs so far, just to virtual
    // "LED"s on devices with screens.
    statusLED2.Init(GPIO_NUM_MAX);
    bluetoothLED.Init();
    wifiLED.Init();
    pairingWindowLED.Init();

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

#if CONFIG_HAVE_DISPLAY
    InitDeviceDisplay();
#endif
    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "AppTask.Init() failed due to %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    ESP_LOGI(TAG, "App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0); // return immediately if the queue is empty
        }
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        BaseType_t status;
        if (xPortInIsrContext())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);
        }
        else
        {
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }
        if (!status)
            ESP_LOGE(TAG, "Failed to post event to app task event queue");
    }
    else
    {
        ESP_LOGE(TAG, "Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->mHandler)
    {
        aEvent->mHandler(aEvent);
    }
    else
    {
        ESP_LOGI(TAG, "Event received with no handler. Dropping event.");
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    AppEvent button_event             = {};
    button_event.mType                = AppEvent::kEventType_Button;
    button_event.mButtonEvent.mPinNo  = btnIdx;
    button_event.mButtonEvent.mAction = btnAction;

    if (btnAction == APP_BUTTON_PRESSED)
    {
        button_event.mHandler = ButtonPressedAction;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::ButtonPressedAction(AppEvent * aEvent)
{
#if CONFIG_DEVICE_TYPE_M5STACK
    uint32_t io_num = aEvent->mButtonEvent.mPinNo;
    int level       = gpio_get_level((gpio_num_t) io_num);
    if (level == 0)
    {
        bool woken = WakeDisplay();
        if (woken)
        {
            return;
        }
        // Button 1 is connected to the pin 39
        // Button 2 is connected to the pin 38
        // Button 3 is connected to the pin 37
        // So we use 40 - io_num to map the pin number to button number
        ScreenManager::ButtonPressed(40 - io_num);
    }
#endif
}
