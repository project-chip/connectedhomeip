/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2022 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
