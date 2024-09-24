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

#include "ButtonWithTimer.h"

namespace chip::NXP::App {

/**
 * @brief This class implements a customized default button behavior.
 *
 * | Action       | Effect                                             |
 * | ------------ | -------------------------------------------------- |
 * | Short press  | If a factory reset is scheduled, cancel it.        |
 * |              | Else if the device is commissioned and a factory   |
 * |              | reset is not scheduled, switch to ICD active mode. |
 * |              | Otherwise, switch commissioning state.             |
 * | Long press   | Schedule a factory reset                           |
 * | Double click | Toggle SIT mode request through DSLS mechanism     |
 *
 */
class ButtonBle : public ButtonWithTimer
{
public:
    virtual CHIP_ERROR Init() override;
    virtual void HandleShortPress() override;
    virtual void HandleLongPress() override;
    virtual void HandleDoubleClick() override;
    virtual void HandleTimerExpire() override;
};

} // namespace chip::NXP::App
