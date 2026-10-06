/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#include "board/board.h"
#include "pwm/pwm_device.h"

#include <platform/CHIPDeviceLayer.h>

struct k_timer;
struct Identify;

enum class LightingActor : uint8_t
{
    Remote,
    Button
};

struct LightingEvent
{
    uint8_t Action;
    LightingActor Actor;
};

class AppTask
{
public:
    static AppTask & Instance()
    {
        static AppTask sAppTask;
        return sAppTask;
    };

    CHIP_ERROR StartApp();

    void UpdateClusterState();
    void InitPWMDDevice();
    Nrf::PWMDevice & GetPWMDevice() { return mPWMDevice; }

private:
    CHIP_ERROR Init();

    static void LightingActionEventHandler(const LightingEvent & event);
    static void ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged);

    static void ActionInitiated(Nrf::PWMDevice::Action_t action, int32_t actor);
    static void ActionCompleted(Nrf::PWMDevice::Action_t action, int32_t actor);

    Nrf::PWMDevice mPWMDevice;
};
