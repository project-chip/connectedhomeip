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

#include "LightingManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>

LightingManager LightingManager::sLightingManager;

#if CONFIG_ENABLE_FEEDBACK
UserInterfaceFeedback & FeedbackMgr()
{
    return LightingMgr();
}
#endif

void LightingManager::ApplyTurnOn()
{
    if (!lightLed.IsTurnedOff())
        return;

    ChipLogProgress(DeviceLayer, "Turn on action has been initiated");
    lightLed.Set(true);
    ChipLogProgress(DeviceLayer, "Turn on action has been completed")
}

void LightingManager::ApplyTurnOff()
{
    if (lightLed.IsTurnedOff())
        return;

    ChipLogProgress(DeviceLayer, "Turn off action has been initiated");
    lightLed.Set(false);
    ChipLogProgress(DeviceLayer, "Turn off action has been completed")
}

void LightingManager::Init()
{
    /* start with all LEDS turned off */
#if LIGHTING_MANAGER_ENABLE_STATUS_LED
    statusLed.Init(LIGHTING_MANAGER_STATUS_LED_INDEX, false);
#endif
    /* The parameters will not be used by the dimmer init. */
    lightLed.Init(LIGHTING_MANAGER_LIGHT_LED_INDEX, false);

    RestoreState();
}

void LightingManager::DisplayInLoop()
{
#if LIGHTING_MANAGER_ENABLE_STATUS_LED
    UpdateStatus();
    statusLed.Animate();
#endif
    lightLed.Animate();
}

void LightingManager::DisplayOnAction(Action action)
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

void LightingManager::RestoreState()
{
    /* restore initial state for the LED indicating Lighting state */
    lightLed.Set(false);

    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t arg) {
        bool val = false;
        LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE::Get(LIGHTING_MANAGER_APP_DEVICE_TYPE_ENDPOINT, &val);
        if (val)
            LightingMgr().ApplyTurnOn();
        else
            LightingMgr().ApplyTurnOff();
    });
}

void LightingManager::UpdateState()
{
    uint8_t newValue = static_cast<uint8_t>(lightLed.IsTurnedOff());

    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            // write the new on/off value
            auto status = LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE::Set(LIGHTING_MANAGER_APP_DEVICE_TYPE_ENDPOINT, (bool) arg);
            if (status != chip::Protocols::InteractionModel::Status::Success)
            {
                ChipLogProgress(DeviceLayer, "Error when updating cluster attribute");
            }
        },
        (intptr_t) newValue);
}

void LightingManager::AnimateOnAction(uint32_t onTimeMS, uint32_t offTimeMS)
{
#if LIGHTING_MANAGER_ENABLE_STATUS_LED
    statusLed.Set(false);
    statusLed.Animate(onTimeMS, offTimeMS);
#endif

    lightLed.Set(false);
    lightLed.Animate(onTimeMS, offTimeMS);
}

#if LIGHTING_MANAGER_ENABLE_STATUS_LED
void LightingManager::UpdateStatus()
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
