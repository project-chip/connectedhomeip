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

#include "ButtonBle.h"
#include "AppTaskBase.h"
#include "BLEApplicationManager.h"
#include "UserInterfaceFeedback.h"

extern "C" {
#include "app.h"
#include "board_comp.h"
}

#include <app/icd/server/ICDServerConfig.h>

/**
 * @brief Timeout (ms) for factory data reset action.
 *
 * During this timeout, the factory reset action can be cancelled by pressing a button.
 */
#ifndef BLE_BUTTON_FACTORY_RESET_TIMEOUT_MS
#define BLE_BUTTON_FACTORY_RESET_TIMEOUT_MS 6000
#endif

static void BleHandleLongPress(const AppEvent & event)
{
    auto & button = *static_cast<chip::NXP::App::ButtonBle *>(event.extra);

    if (xTimerIsTimerActive(button.timer))
    {
        button.CancelTimer();
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().RestoreState();
#endif
        ChipLogProgress(DeviceLayer, "Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = BLE_BUTTON_FACTORY_RESET_TIMEOUT_MS;
        ChipLogProgress(DeviceLayer, "Factory Reset Triggered. Push the RESET button within %lu ms to cancel!", resetTimeout);

#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().DisplayOnAction(UserInterfaceFeedback::Action::kFactoryReset);
#endif
        button.StartTimer(resetTimeout);
    }
}

static void BleHandleShortPress(const AppEvent & event)
{
    auto & button = *static_cast<chip::NXP::App::ButtonBle *>(event.extra);
    if (xTimerIsTimerActive(button.timer))
    {
        // If a factory reset is scheduled, pressing the BLE button will cancel it.
        BleHandleLongPress(event);
        return;
    }

#if CHIP_CONFIG_ENABLE_ICD_LIT
    if (chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned())
    {
        // If the device is commissioned and a factory reset is not scheduled, switch to active mode.
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifyNetworkActivityNotification(); }, 0);
        return;
    }
#endif

    chip::NXP::App::GetAppTask().SwitchCommissioningStateHandler();
    chip::NXP::App::BleAppMgr().EnableMultipleConnectionsHandler();
}

CHIP_ERROR chip::NXP::App::ButtonBle::Init()
{
    // Button is initialized in otSysInit, when APP_InitServices is called.
    // Overwrite the handle to reference the SDK handle.
    handle = &g_buttonHandle[0];

    return CHIP_NO_ERROR;
}

void chip::NXP::App::ButtonBle::HandleShortPress()
{
    AppEvent event;
    event.Handler = BleHandleShortPress;
    event.extra   = this;
    chip::NXP::App::GetAppTask().PostEvent(event);
}

void chip::NXP::App::ButtonBle::HandleLongPress()
{
    AppEvent event;
    event.Handler = BleHandleLongPress;
    event.extra   = this;
    chip::NXP::App::GetAppTask().PostEvent(event);
}

void chip::NXP::App::ButtonBle::HandleDoubleClick()
{
#if (CHIP_CONFIG_ENABLE_ICD_LIT && CHIP_CONFIG_ENABLE_ICD_DSLS)
    static bool sitModeRequested = false;

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
#endif
}

void chip::NXP::App::ButtonBle::HandleTimerExpire()
{
    ChipLogProgress(DeviceLayer, "Device will factory reset...");

    // Actually trigger Factory Reset
    chip::Server::GetInstance().ScheduleFactoryReset();
}
