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

#include "MotionSensorManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <lib/support/CodeUtils.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

MotionSensorManager MotionSensorManager::sMotionSensor;

int MotionSensorManager::Init()
{
    int err = 0;

    mState                = State::kMotionOpened;
    mCallbackStateChanged = nullptr;

    return err;
}

void MotionSensorManager::SetCallback(CallbackStateChanged aCallbackStateChanged)
{
    mCallbackStateChanged = aCallbackStateChanged;
}

bool MotionSensorManager::IsMotionClosed()
{
    return mState == State::kMotionClosed;
}

void MotionSensorManager::InitiateAction(Action aAction)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_DeviceAction;
    event.DeviceEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler            = HandleAction;
    GetAppTask().PostEvent(&event);
}

void MotionSensorManager::HandleAction(AppEvent * aEvent)
{
    Action action = static_cast<Action>(aEvent->DeviceEvent.Action);
    // Change current state based on action:
    // - if state is closed and action is signal lost, change state to opened
    // - if state is opened and action is signal detected, change state to closed
    // - else, the state/action combination does not change the state.
    if (sMotionSensor.mState == State::kMotionClosed && action == Action::kSignalLost)
    {
        sMotionSensor.mState = State::kMotionOpened;
    }
    else if (sMotionSensor.mState == State::kMotionOpened && action == Action::kSignalDetected)
    {
        sMotionSensor.mState = State::kMotionClosed;
    }

    if (sMotionSensor.mCallbackStateChanged != nullptr)
    {
        sMotionSensor.mCallbackStateChanged(sMotionSensor.mState);
    }
    else
    {
        LOG_ERR("Callback for state change was not set. Please set an appropriate callback.");
    }
}
