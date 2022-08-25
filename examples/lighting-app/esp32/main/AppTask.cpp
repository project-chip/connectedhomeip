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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>

#define APP_TASK_NAME "APP"
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (3072)
#define BUTTON_PRESSED 1
#define APP_LIGHT_SWITCH 1

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static const char * TAG = "app-task";

LEDWidget AppLED;
Button AppButton;

namespace {
constexpr EndpointId kLightEndpointId = 1;
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    AppLED.Init();
    AppButton.Init();

    AppButton.SetButtonPressCallback(ButtonPressCallback);

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

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    AppLED.Toggle();
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    sAppTask.UpdateClusterState();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void AppTask::ButtonPressCallback()
{
    AppEvent button_event;
    button_event.Type     = AppEvent::kEventType_Button;
    button_event.mHandler = AppTask::LightingActionEventHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::UpdateClusterState()
{
    ESP_LOGI(TAG, "Writing to OnOff cluster");
    // write the new on/off value
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, AppLED.IsTurnedOn());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG, "Updating on/off cluster failed: %x", status);
    }

    ESP_LOGI(TAG, "Writing to Current Level cluster");
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, AppLED.GetLevel());

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG, "Updating level cluster failed: %x", status);
    }
}
