/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ButtonManager.h"
#include "AppConfig.h"
#include "AppMatterButton.h"
#include "AppTask.h"
#include "UserInterfaceFeedback.h"

#include <app-common/zap-generated/attributes/Accessors.h>

extern "C" {
#include "app.h"
#include "board_comp.h"
}

CHIP_ERROR chip::NXP::App::AppMatterButton_registerButtons(void)
{
    /* Board buttons are initialized in otSysInit, when APP_InitServices is called. */
    button_status_t bStatus;

    bStatus = BUTTON_InstallCallback((button_handle_t) g_buttonHandle[0], ButtonManager::BleCallback, NULL);
    VerifyOrReturnError(bStatus == kStatus_BUTTON_Success, CHIP_ERROR_UNEXPECTED_EVENT,
                        ChipLogError(DeviceLayer, "button init error"));

    bStatus = BUTTON_InstallCallback((button_handle_t) g_buttonHandle[1], ButtonManager::AppActionCallback, NULL);
    VerifyOrReturnError(bStatus == kStatus_BUTTON_Success, CHIP_ERROR_UNEXPECTED_EVENT,
                        ChipLogError(DeviceLayer, "button init error"));

    ReturnErrorOnFailure(ButtonMgr().Init());

    return CHIP_NO_ERROR;
}

ButtonManager ButtonManager::sInstance;

TimerHandle_t resetTimer;

#if (CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS)
static bool sitModeRequested;
#endif // CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS

CHIP_ERROR ButtonManager::Init()
{
    resetTimer = xTimerCreate("FnTmr", 1, false, (void *) this, [](TimerHandle_t xTimer) {
        AppEvent event;
        event.Handler = FunctionTimerEventHandler;
        chip::NXP::App::GetAppTask().PostEvent(event);
    });
    VerifyOrReturnError(resetTimer != NULL, APP_ERROR_CREATE_TIMER_FAILED);

#if (CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS)
    static bool sitModeRequested;
#endif // CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS

    return CHIP_NO_ERROR;
}

button_status_t ButtonManager::BleCallback(void * handle, button_callback_message_t * message, void * param)
{
    AppEvent event;

    switch (message->event)
    {
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        event.Handler = ButtonManager::BleHandler;
        break;
    case kBUTTON_EventLongPress:
        event.Handler = ButtonManager::ResetActionEventHandler;
        break;

#if (CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS)
    case kBUTTON_EventDoubleClick:
        event.Handler = ButtonManager::DSLSActionEventHandler;
        break;
#endif

    default:
        /* No action required */
        break;
    }

    chip::NXP::App::GetAppTask().PostEvent(event);

    return kStatus_BUTTON_Success;
}

button_status_t ButtonManager::AppActionCallback(void * handle, button_callback_message_t * message, void * param)
{
    AppEvent event;

    switch (message->event)
    {
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        event.Handler = ButtonManager::AppActionEventHandler;
        break;
    case kBUTTON_EventLongPress:
        // Soft reset ensures that platform manager shutdown procedure is called.
        event.Handler = ButtonManager::SoftResetHandler;
        break;
    default:
        /* No action required */
        break;
    }

    chip::NXP::App::GetAppTask().PostEvent(event);

    return kStatus_BUTTON_Success;
}

void ButtonManager::FunctionTimerEventHandler(const AppEvent & event)
{
    ChipLogProgress(DeviceLayer, "Device will factory reset...");

    // Actually trigger Factory Reset
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void ButtonManager::ResetActionEventHandler(const AppEvent & event)
{
    if (xTimerIsTimerActive(resetTimer))
    {
        ButtonMgr().CancelTimer();
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().RestoreState();
#endif
        ChipLogProgress(DeviceLayer, "Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = BUTTON_MANAGER_FACTORY_RESET_TIMEOUT_MS;
        ChipLogProgress(DeviceLayer, "Factory Reset Triggered. Push the RESET button within %lu ms to cancel!", resetTimeout);

#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().DisplayOnAction(UserInterfaceFeedback::Action::kFactoryReset);
#endif
        ButtonMgr().StartTimer(resetTimeout);
    }
}

void ButtonManager::AppActionEventHandler(const AppEvent & event)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            auto status = chip::NXP::App::GetAppTask().ProcessSetStateClusterHandler();
            if (status != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "Error when updating cluster attribute");
            }
        },
        (intptr_t) nullptr);
}

void ButtonManager::SoftResetHandler(const AppEvent & event)
{
    chip::DeviceLayer::PlatformMgrImpl().CleanReset();
}

#if CHIP_CONFIG_ENABLE_ICD_LIT
static void UserActiveModeHandler(const AppEvent & event)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifyNetworkActivityNotification(); }, 0);
}
#endif

void ButtonManager::BleHandler(const AppEvent & event)
{
    if (xTimerIsTimerActive(resetTimer))
    {
        // If a factory reset is scheduled, pressing the BLE button will cancel it.
        ResetActionEventHandler(event);
        return;
    }

#if CHIP_CONFIG_ENABLE_ICD_LIT
    if (chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned())
    {
        // If the device is commissioned and a factory reset is not scheduled, switch to active mode.
        UserActiveModeHandler(event);
        return;
    }
#endif

    chip::NXP::App::GetAppTask().SwitchCommissioningStateHandler();
}

#if (CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS)
void ButtonManager::DSLSActionEventHandler(const AppEvent & event)
{
    if (chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned())
    {
        if (!sitModeRequested)
        {
            chip::DeviceLayer::PlatformMgr().ScheduleWork(
                [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestNotification(); }, 0);
            sitModeRequested = true;
        }
        else
        {
            chip::DeviceLayer::PlatformMgr().ScheduleWork(
                [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestWithdrawal(); }, 0);
            sitModeRequested = false;
        }
    }
}
#endif // CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS

void ButtonManager::CancelTimer()
{
    if (xTimerStop(resetTimer, 0) == pdFAIL)
    {
        ChipLogProgress(DeviceLayer, "app timer stop() failed");
    }
}

void ButtonManager::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(resetTimer))
    {
        ChipLogProgress(DeviceLayer, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(resetTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ChipLogProgress(DeviceLayer, "app timer start() failed");
    }
}
