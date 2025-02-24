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

#include "DoorWindowManager.h"
#include "AppTask.h"
#include "qvIO.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

DoorWindowManager DoorWindowManager::sDoorWindow;
using namespace ::chip;
using namespace chip::DeviceLayer;

CHIP_ERROR DoorWindowManager::Init(void)
{
    mState                = State::kDoorWindowOpened;
    mCallbackStateChanged = nullptr;

    return CHIP_NO_ERROR;
}

void DoorWindowManager::SetCallback(CallbackStateChanged aCallbackStateChanged)
{
    mCallbackStateChanged = aCallbackStateChanged;
}

bool DoorWindowManager::IsDoorWindowClosed(void)
{
    return mState == State::kDoorWindowClosed;
}

void DoorWindowManager::InitiateAction(Action aAction)
{
    AppEvent event;
    event.Type                   = AppEvent::kEventType_DoorWindow;
    event.DoorWindowEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler                = HandleAction;
    GetAppTask().PostEvent(&event);
}

void DoorWindowManager::HandleAction(AppEvent * aEvent)
{
    Action action = static_cast<Action>(aEvent->DoorWindowEvent.Action);
    // Change current state based on action:
    // - if state is closed and action is signal lost, change state to opened
    // - if state is opened and action is signal detected, change state to closed
    // - else, the state/action combination does not change the state.
    if (sDoorWindow.mState == State::kDoorWindowClosed && action == Action::kSignalLost)
    {
        sDoorWindow.mState = State::kDoorWindowOpened;
    }
    else if (sDoorWindow.mState == State::kDoorWindowOpened && action == Action::kSignalDetected)
    {
        sDoorWindow.mState = State::kDoorWindowClosed;
    }

    if (sDoorWindow.mCallbackStateChanged != nullptr)
    {
        sDoorWindow.mCallbackStateChanged(sDoorWindow.mState);
    }
    else
    {
        ChipLogError(NotSpecified, "Callback for state change was not set. Please set an appropriate callback.");
    }
}
