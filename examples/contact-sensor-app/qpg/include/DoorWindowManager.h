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

#include <functional>

#include "AppConfig.h"
#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/DecodableList.h>
#include <app/util/im-client-callbacks.h>

#define DOORWINDOW_ENDPOINT_ID 1

class DoorWindowManager
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
        kDoorWindowClosed = 0,
        kDoorWindowOpened,
        kInvalid
    };

    CHIP_ERROR Init(void);
    bool IsDoorWindowClosed(void);
    void InitiateAction(Action aAction);

    typedef void (*CallbackStateChanged)(State aState);
    void SetCallback(CallbackStateChanged aCallbackStateChanged);

    static void HandleAction(AppEvent * aEvent);

private:
    friend DoorWindowManager & DoorWindowMgr(void);
    State mState;
    CallbackStateChanged mCallbackStateChanged;
    static DoorWindowManager sDoorWindow;
};

inline DoorWindowManager & DoorWindowMgr(void)
{
    return DoorWindowManager::sDoorWindow;
}
