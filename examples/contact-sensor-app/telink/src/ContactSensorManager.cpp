/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ContactSensorManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <lib/support/CodeUtils.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

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
