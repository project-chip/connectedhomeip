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

#pragma once

#include "AppEventCommon.h"
#include <stdbool.h>
#include <stdint.h>

class MotionSensorManager
{
public:
    enum class State : uint8_t
    {
        kMotionUndetected = 0,
        kMotionDetected
    };

    enum class Action : uint8_t
    {
        kSetUndetected = 0,
        kSetDetected
    };

    int Init();
    bool IsMotionDetected() const;

    void InitiateAction(Action aAction);

    using CallbackStateChanged = void (*)(State aState);
    void SetCallback(CallbackStateChanged cb);

    static void HandleAction(AppEvent * aEvent);
    static MotionSensorManager sMotionSensor;

private:
    State mState                   = State::kMotionUndetected;
    CallbackStateChanged mCallback = nullptr;
};

inline MotionSensorManager & MotionSensorMgr()
{
    return MotionSensorManager::sMotionSensor;
}
