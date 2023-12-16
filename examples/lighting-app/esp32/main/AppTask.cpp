/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "DeviceWithDisplay.h"
#include <app/server/Server.h>

#include <app-common/zap-generated/attributes/Accessors.h>

#define APP_TASK_NAME "APP"
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (3072)

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static const char * TAG = "app-task";

namespace {
    constexpr EndpointId kEndpointId = 1;
    QueueHandle_t sAppEventQueue;
    TaskHandle_t sAppTaskHandle;
} // namespace

#define BUTTON_1_GPIO_NUM     ((gpio_num_t) 6)
// Button gButtons[BUTTON_NUMBER] = { Button(BUTTON_1_GPIO_NUM) };

AppTask AppTask::sAppTask;
ButtonTask AppTask::sButtonTask;

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
    CHIP_ERROR err = CHIP_NO_ERROR;

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

    button_event_t btn_event;
    QueueHandle_t button_events = sButtonTask.button_init(PIN_BIT(BUTTON_1_GPIO_NUM));
    ESP_LOGI(TAG, "Buttons initialized");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0); // return immediately if the queue is empty
        }

        if (xQueueReceive(button_events, &btn_event, pdMS_TO_TICKS(10))) {
            if (btn_event.pin == BUTTON_1_GPIO_NUM) {
                switch(btn_event.event) {
                    case BUTTON_DOWN:
                        ESP_LOGI(TAG, "BUTTON DOWN EVENT SEEN");
                        break;
                    case BUTTON_UP:
                        ESP_LOGI(TAG, "BUTTON UP EVENT SEEN");
                        break;
                    case BUTTON_HELD:
                        ESP_LOGI(TAG, "BUTTON HELD EVENT SEEN");
                        chip::Server::GetInstance().ScheduleFactoryReset();
                        break;
                    default:
                        break;
                }
            }
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

void AppTask::PersonDetectedEventHandler(AppEvent * aEvent)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    sAppTask.personDetected = aEvent->PersonDetectedEvent.PersonDetected;
    sAppTask.UpdateClusterState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void AppTask::EnableStatusUpdates()
{
    sAppTask.wifiConnected = true;
}

void AppTask::DisableStatusUpdates()
{
    sAppTask.wifiConnected = false;
}

void AppTask::UpdateOccupancySensorConfiguration()
{
    if (!sAppTask.wifiConnected) {
        return;
    }

    ESP_LOGI(TAG, "Writing to Occupancy Sensing Cluster Configuration");
    EmberAfStatus status = Clusters::OccupancySensing::Attributes::OccupancySensorType::Set(kEndpointId, Clusters::OccupancySensing::OccupancySensorTypeEnum::kUltrasonic);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG, "Updating occupancy sensor cluster configuration failed: %x", status);
    }
}

void AppTask::UpdateClusterState()
{
    if (!sAppTask.wifiConnected) {
        return;
    }

    ESP_LOGI(TAG, "Writing to Occupancy Sensing Cluster");
    EmberAfStatus status = Clusters::OccupancySensing::Attributes::Occupancy::Set(kEndpointId, sAppTask.personDetected);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG, "Updating occupancy sensor cluster failed: %x", status);
    }
}
