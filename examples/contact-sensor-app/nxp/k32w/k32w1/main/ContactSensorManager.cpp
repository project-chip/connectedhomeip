/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
 *    Copyright (c) 2022 Google LLC.
 *    Copyright (c) 2023 NXP
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

#include "AppTask.h"
#include "FreeRTOS.h"

#include "app_config.h"

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
    event.Type                = AppEvent::kContact;
    event.ContactEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler             = HandleAction;
    GetAppTask().PostEvent(&event);
}

void ContactSensorManager::HandleAction(void * aGenericEvent)
{
    AppEvent * event = static_cast<AppEvent *>(aGenericEvent);
    Action action    = static_cast<Action>(event->ContactEvent.Action);
    // Change current state based on action:
    // - if state is closed and action is signal lost, change state to opened
    // - if state is opened and action is signal detected, change state to closed
    // - else, the state/action combination does not change the state.
    if (State::kContactClosed == sContactSensor.mState && Action::kSignalLost == action)
    {
        sContactSensor.mState = State::kContactOpened;
    }
    else if (State::kContactOpened == sContactSensor.mState && Action::kSignalDetected == action)
    {
        sContactSensor.mState = State::kContactClosed;
    }

    if (sContactSensor.mCallbackStateChanged != nullptr)
    {
        sContactSensor.mCallbackStateChanged(sContactSensor.mState);
    }
    else
    {
        K32W_LOG("Callback for state change was not set. Please set an appropriate callback.");
    }
}
