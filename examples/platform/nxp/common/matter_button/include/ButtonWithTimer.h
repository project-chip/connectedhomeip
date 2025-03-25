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

#pragma once

#include "Button.h"
#include "FreeRTOS.h"
#include "timers.h"

namespace chip::NXP::App {
/**
 * @brief This class implements a default button with timer behavior.
 *
 * This class provides an API for starting/cancelling the timer. It can
 * be used by derived classes to define their own timer handler behavior.
 */
class ButtonWithTimer : public Button
{
public:
    ButtonWithTimer();

    virtual CHIP_ERROR Init() override;
    virtual void HandleShortPress() override;
    virtual void HandleLongPress() override;
    virtual void HandleDoubleClick() override;

    virtual void HandleTimerExpire();

    void CancelTimer();
    void StartTimer(uint32_t aTimeoutInMs);

    TimerHandle_t timer;
};

} // namespace chip::NXP::App
