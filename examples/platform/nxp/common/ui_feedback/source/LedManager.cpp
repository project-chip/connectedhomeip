/*
 *
 *    Copyright (c) 2021, 2024 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "LedManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>

LedManager LedManager::sLedManager;

#if CONFIG_ENABLE_FEEDBACK
UserInterfaceFeedback & FeedbackMgr()
{
    return LightingMgr();
}
#endif

void LedManager::ApplyTurnOn()
{
    if (!lightLed.IsTurnedOff())
        return;

    lightLed.Set(true);
}

void LedManager::ApplyTurnOff()
{
    if (lightLed.IsTurnedOff())
        return;

    lightLed.Set(false);
}

void LedManager::Init()
{
    /* start with all LEDS turned off */
#if LED_MANAGER_ENABLE_STATUS_LED
    statusLed.Init(LED_MANAGER_STATUS_LED_INDEX, false);
#endif
    /* The parameters will not be used by the dimmer init. */
    lightLed.Init(LED_MANAGER_LIGHT_LED_INDEX, false);
}

void LedManager::DisplayInLoop()
{
#if LED_MANAGER_ENABLE_STATUS_LED
    UpdateStatus();
    statusLed.Animate();
#endif
    lightLed.Animate();
}

void LedManager::DisplayOnAction(Action action)
{
    switch (action)
    {
    case UserInterfaceFeedback::Action::kFactoryReset:
    case UserInterfaceFeedback::Action::kTriggerEffect:
        AnimateOnAction(500, 500);
        break;
    case UserInterfaceFeedback::Action::kIdentify:
        AnimateOnAction(250, 250);
        break;
    default:
        break;
    }
}

void LedManager::RestoreState()
{
    /* restore initial state for the LED indicating Lighting state */
    lightLed.Set(false);

    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t arg) {
        bool val = false;
        val      = chip::NXP::App::GetAppTask().CheckStateClusterHandler();
        if (val)
            LightingMgr().ApplyTurnOn();
        else
            LightingMgr().ApplyTurnOff();
    });
}

void LedManager::AnimateOnAction(uint32_t onTimeMS, uint32_t offTimeMS)
{
#if LED_MANAGER_ENABLE_STATUS_LED
    statusLed.Set(false);
    statusLed.Animate(onTimeMS, offTimeMS);
#endif

    lightLed.Set(false);
    lightLed.Animate(onTimeMS, offTimeMS);
}

#if LED_MANAGER_ENABLE_STATUS_LED
void LedManager::UpdateStatus()
{
    bool isThreadProvisioned = false;
    uint16_t bleConnections  = 0;

    // Collect connectivity and configuration state from the CHIP stack.  Because the
    // CHIP event loop is being run in a separate task, the stack must be locked
    // while these values are queried.  However we use a non-blocking lock request
    // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
    // task is busy (e.g. with a long crypto operation).
    if (chip::DeviceLayer::PlatformMgr().TryLockChipStack())
    {
        isThreadProvisioned = chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned();
        bleConnections      = chip::DeviceLayer::ConnectivityMgr().NumBLEConnections();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    if (isThreadProvisioned)
    {
        statusLed.Animate(950, 50);
    }
    else if (bleConnections != 0)
    {
        statusLed.Animate(100, 100);
    }
    else
    {
        statusLed.Animate(50, 950);
    }
}
#endif
