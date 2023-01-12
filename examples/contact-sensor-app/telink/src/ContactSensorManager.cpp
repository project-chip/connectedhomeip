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

#include "ContactSensorManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <lib/support/CodeUtils.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_DECLARE(app);

ContactSensorManager ContactSensorManager::sContactSensor;

int ContactSensorManager::Init()
{
    int err = 0;

    mState                = State::kContactOpened;
    mCallbackStateChanged = nullptr;

    return err;
}

void ContactSensorManager::SetCallback(CallbackStateChanged aCallbackStateChanged)
{
    mCallbackStateChanged = aCallbackStateChanged;
}

bool ContactSensorManager::IsContactClosed()
{
    return mState == State::kContactClosed;
}

void ContactSensorManager::InitiateAction(Action aAction)
{
    AppEvent event;
    event.Type                = AppEvent::kEventType_Contact;
    event.ContactEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler             = HandleAction;
    GetAppTask().PostEvent(&event);
}

void ContactSensorManager::HandleAction(AppEvent * aEvent)
{
    Action action = static_cast<Action>(aEvent->ContactEvent.Action);
    // Change current state based on action:
    // - if state is closed and action is signal lost, change state to opened
    // - if state is opened and action is signal detected, change state to closed
    // - else, the state/action combination does not change the state.
    if (sContactSensor.mState == State::kContactClosed && action == Action::kSignalLost)
    {
        sContactSensor.mState = State::kContactOpened;
    }
    else if (sContactSensor.mState == State::kContactOpened && action == Action::kSignalDetected)
    {
        sContactSensor.mState = State::kContactClosed;
    }

    if (sContactSensor.mCallbackStateChanged != nullptr)
    {
        sContactSensor.mCallbackStateChanged(sContactSensor.mState);
    }
    else
    {
        LOG_ERR("Callback for state change was not set. Please set an appropriate callback.");
    }
}
