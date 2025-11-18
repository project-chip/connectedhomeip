/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "LEDManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>

// TODO: Ideally we should not depend on the codegen integration
// It would be best if we could use generic cluster API instead
#include <app/clusters/boolean-state-server/CodegenIntegration.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(MotionActionEventHandler);
    InitCommonParts();

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, MotionSensorMgr().IsMotionClosed());

    UpdateDeviceState();

    MotionSensorMgr().SetCallback(OnStateChanged);

    return CHIP_NO_ERROR;
}

void AppTask::OnStateChanged(MotionSensorManager::State aState)
{
    // If the Motion state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the Motion LED if Motion sensor is in closed state.
    //  - turn off the lock LED if Motion sensor is in opened state.
    if (MotionSensorManager::State::kMotionClosed == aState)
    {
        LOG_INF("Motion state changed to CLOSED");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
    }
    else if (MotionSensorManager::State::kMotionOpened == aState)
    {
        LOG_INF("Motion state changed to OPEN");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostMotionActionRequest(MotionSensorManager::Action aAction)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_DeviceAction;
    event.DeviceEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler            = MotionActionEventHandler;

    sAppTask.PostEvent(&event);
}

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = MotionSensorMgr().IsMotionClosed();

    ChipLogProgress(NotSpecified, "StateValue::Set : %d", newValue);

    auto booleanState = chip::app::Clusters::BooleanState::FindClusterOnEndpoint(1);
    VerifyOrReturn(booleanState != nullptr);
    booleanState->SetStateValue(newValue);
}

void AppTask::MotionActionEventHandler(AppEvent * aEvent)
{
    MotionSensorManager::Action action  = MotionSensorManager::Action::kInvalid;
    CHIP_ERROR err                      = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "MotionActionEventHandler");

    if (aEvent->Type == AppEvent::kEventType_DeviceAction)
    {
        action = static_cast<MotionSensorManager::Action>(aEvent->DeviceEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (MotionSensorMgr().IsMotionClosed())
        {
            action = MotionSensorManager::Action::kSignalLost;
        }
        else
        {
            action = MotionSensorManager::Action::kSignalDetected;
        }

        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = MotionSensorManager::Action::kInvalid;
    }

    if (err == CHIP_NO_ERROR)
    {
        MotionSensorMgr().InitiateAction(action);
    }
}

void AppTask::UpdateClusterState(void)
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    auto booleanState = chip::app::Clusters::BooleanState::FindClusterOnEndpoint(1);
    VerifyOrReturn(booleanState != nullptr);
    auto stateValueAttrValue = booleanState->GetStateValue();
    LedManager::getInstance().setLed(LedManager::EAppLed_App0, stateValueAttrValue);
}

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#else
    ledManager.linkLed(LedManager::EAppLed_App0, 0);
#endif // CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
}
