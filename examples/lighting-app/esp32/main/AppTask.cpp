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

#include <app-common/zap-generated/attributes/Accessors.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include "Button.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/general-commissioning-server/general-commissioning-server.h>

#define APP_TASK_NAME "APP"
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (3072)
#define BUTTON_PRESSED 1
#define APP_LIGHT_SWITCH 1

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static const char TAG[] = "app-task";

LEDWidget AppLED;

namespace {
constexpr EndpointId kLightEndpointId = 1;
QueueHandle_t sAppEventQueue;
TaskHandle_t sAppTaskHandle;
} // namespace

AppTask AppTask::sAppTask;

#if CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED
/* add the timer for checking WiFi state*/
static esp_timer_handle_t g_checking_state_polling_timer_handle = NULL;
static void checking_state_polling_cb(void *args)
{
    static uint8_t timerCount = 0;
    static bool lastWiFiStationConnected = true;
    chip::app::Clusters::GeneralCommissioning::Attributes::NetworkRecoveryReason::TypeInfo::Type reason;
    if (chip::DeviceLayer::ConnectivityMgr().IsWiFiStationConnected()){
        timerCount = 0;
        if (false == lastWiFiStationConnected){
            int WiFiDisconnectFlagValue = 0;
            //chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get("WiFiDisconnectFlag", &WiFiDisconnectFlagValue);
            if (WiFiDisconnectFlagValue != 0x11){
                lastWiFiStationConnected = true;
                ESP_LOGI(TAG, "WiFi Connected");
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                if (ConnectivityMgr().IsWiFiStationProvisioned()){
                    //chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::NetworkRecoveryReasonEnum> reason;
                    chip::app::Clusters::GeneralCommissioning::GetNetworkRecoveryReasonValue(reason);
                    if (!reason.IsNull()){
                        //chip::app::DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::NetworkRecoveryReasonEnum> reason;
                        reason.SetNull();
                        //chip::app::Clusters::GeneralCommissioning::Attributes::NetworkRecoveryReason::Set(kRootEndpointId, reason);
                        chip::app::Clusters::GeneralCommissioning::SetNetworkRecoveryReasonValue(reason);
                        if (ConnectivityMgr().IsBLEAdvertising()){
                            ESP_LOGI(TAG, "Stop WiFi Network Recovery BLE Advertisement");
                            ConnectivityMgr().SetBLEAdvertisingEnabled(false);
                        }
                    }
                }
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
        } else {
            //Do nothing
        }
    } else {
        if (lastWiFiStationConnected){
            //wifi disconnected
            if (timerCount < 10){
                timerCount++; //checking ten seconds
                ESP_LOGI(TAG, "WiFi Disonnected Count: %d",timerCount);
            } else {
                lastWiFiStationConnected = false;
                using chip::DeviceLayer::ConnectivityMgr;
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                if (ConnectivityMgr().IsWiFiStationProvisioned()){
                    ESP_LOGI(TAG, "Start WiFi Network Recovery BLE Advertisement");
                    chip::app::Clusters::GeneralCommissioning::GetNetworkRecoveryReasonValue(reason);
                    if (reason.IsNull()){
                        reason = chip::app::Clusters::GeneralCommissioning::NetworkRecoveryReasonEnum::kVisibility;
                        chip::app::Clusters::GeneralCommissioning::SetNetworkRecoveryReasonValue(reason);
                    }
                    ConnectivityMgr().SetBLEAdvertisingEnabled(true);
                }
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
        } else {
            timerCount = 0;
        }
    }
}
//end checking WiFi state
#endif //CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED

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

void AppTask::ButtonEventHandler(const uint8_t buttonHandle, uint8_t btnAction)
{
    if (btnAction != APP_BUTTON_PRESSED)
    {
        return;
    }

    AppEvent button_event = {};
    button_event.Type     = AppEvent::kEventType_Button;

#if CONFIG_HAVE_DISPLAY
    button_event.ButtonEvent.PinNo  = buttonHandle;
    button_event.ButtonEvent.Action = btnAction;
    button_event.mHandler           = ButtonPressedAction;
#else
    button_event.mHandler = AppTask::LightingActionEventHandler;
#endif

    sAppTask.PostEvent(&button_event);
}

#if CONFIG_DEVICE_TYPE_M5STACK
void AppTask::ButtonPressedAction(AppEvent * aEvent)
{
    uint32_t io_num = aEvent->ButtonEvent.PinNo;
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
}
#endif

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AppLED.Init();
#if CONFIG_HAVE_DISPLAY
    InitDeviceDisplay();

    AppLED.SetVLED(ScreenManager::AddVLED(TFT_YELLOW));
#endif

#if CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED
/* add the timer for checking WiFi state*/
    esp_timer_create_args_t checking_state_polling_timer;
    checking_state_polling_timer.arg = NULL;
    checking_state_polling_timer.callback = checking_state_polling_cb;
    checking_state_polling_timer.dispatch_method = ESP_TIMER_TASK;
    checking_state_polling_timer.name = "checking_state_polling_timer";
    esp_timer_create(&checking_state_polling_timer, &g_checking_state_polling_timer_handle);
    esp_timer_start_periodic(g_checking_state_polling_timer_handle, 1000000U);
//end
#endif //CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED
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

void AppTask::UpdateClusterState()
{
    ESP_LOGI(TAG, "Writing to OnOff cluster");
    // write the new on/off value
    Protocols::InteractionModel::Status status = Clusters::OnOff::Attributes::OnOff::Set(kLightEndpointId, AppLED.IsTurnedOn());

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ESP_LOGE(TAG, "Updating on/off cluster failed: %x", to_underlying(status));
    }

    ESP_LOGI(TAG, "Writing to Current Level cluster");
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, AppLED.GetLevel());

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ESP_LOGE(TAG, "Updating level cluster failed: %x", to_underlying(status));
    }
}
