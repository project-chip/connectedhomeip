/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ButtonWithTimer.h"
#include "AppTaskBase.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

chip::NXP::App::ButtonWithTimer::ButtonWithTimer()
{
    // Pass the instance pointer as timer id pointer. The pointer can then
    // be retrieved inside the callback to delegate the action to the button
    // instance, through the HandleTimerExpire function.
    timer = xTimerCreate("FnTmr", 1, false, (void *) this, [](TimerHandle_t xTimer) {
        ButtonWithTimer * button = static_cast<ButtonWithTimer *>(pvTimerGetTimerID(xTimer));
        button->HandleTimerExpire();
    });
    VerifyOrDie(timer != nullptr);
}

CHIP_ERROR chip::NXP::App::ButtonWithTimer::Init()
{
    /* Currently empty */
    return CHIP_NO_ERROR;
}

void chip::NXP::App::ButtonWithTimer::HandleShortPress()
{
    /* Currently not mapped to any action */
}

void chip::NXP::App::ButtonWithTimer::HandleLongPress()
{
    /* Currently not mapped to any action */
}

void chip::NXP::App::ButtonWithTimer::HandleDoubleClick()
{
    /* Currently not mapped to any action */
}

void chip::NXP::App::ButtonWithTimer::HandleTimerExpire()
{
    /* Currently empty */
}

void chip::NXP::App::ButtonWithTimer::CancelTimer()
{
    if (xTimerStop(timer, 0) == pdFAIL)
    {
        ChipLogProgress(DeviceLayer, "app timer stop() failed");
    }
}

void chip::NXP::App::ButtonWithTimer::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(timer))
    {
        ChipLogProgress(DeviceLayer, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(timer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ChipLogProgress(DeviceLayer, "app timer start() failed");
    }
}
