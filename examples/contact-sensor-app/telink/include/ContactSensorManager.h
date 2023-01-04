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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

class ContactSensorManager
{
public:
    enum class Action : uint8_t
    {
        kSignalDetected = 0,
        kSignalLost,
        kInvalid
    };

    enum class State : uint8_t
    {
        kContactClosed = 0,
        kContactOpened,
        kInvalid
    };

    int Init();
    bool IsContactClosed();
    void InitiateAction(Action aAction);

    typedef void (*CallbackStateChanged)(State aState);
    void SetCallback(CallbackStateChanged aCallbackStateChanged);

    static void HandleAction(AppEvent * aEvent);

private:
    friend ContactSensorManager & ContactSensorMgr(void);
    State mState;
    CallbackStateChanged mCallbackStateChanged;
    static ContactSensorManager sContactSensor;
};

inline ContactSensorManager & ContactSensorMgr(void)
{
    return ContactSensorManager::sContactSensor;
}
