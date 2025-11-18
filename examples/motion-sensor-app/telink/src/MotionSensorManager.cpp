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
#include "AppTask.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

MotionSensorManager MotionSensorManager::sMotionSensor;

int MotionSensorManager::Init()
{
    mState    = State::kMotionUndetected;
    mCallback = nullptr;
    return 0;
}

bool MotionSensorManager::IsMotionDetected() const
{
    return mState == State::kMotionDetected;
}

void MotionSensorManager::SetCallback(CallbackStateChanged cb)
{
    mCallback = cb;
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

    State newState = sMotionSensor.mState;

    if (action == Action::kSetDetected)
        newState = State::kMotionDetected;
    else if (action == Action::kSetUndetected)
        newState = State::kMotionUndetected;

    if (newState != sMotionSensor.mState)
        sMotionSensor.mState = newState;

    if (sMotionSensor.mCallback)
        sMotionSensor.mCallback(sMotionSensor.mState);
    else
        LOG_WRN("MotionSensor callback not set");
}
