/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <FreeRTOS.h>
#include <timers.h>

#include "DimmableLEDIf.h"
#include "hal_gpio.h"

class DimmableLEDWidget : public DimmableLEDIf
{
public:
    void Init(hal_gpio_pin_t gpio);
    /* set to ON or OFF */
    virtual void Set(bool state);
    /* Get On/Off state */
    virtual bool Get(void);
    /* change light level */
    virtual void SetLevel(uint8_t level);
    /* Get current level*/
    virtual uint8_t GetLevel(void);
    virtual uint8_t GetMaxLevel(void);
    virtual uint8_t GetMinLevel(void);

private:
    bool mState;    /* On or Off status */
    uint8_t mLevel; /* Current level */
    hal_gpio_pin_t mGpioPin;
};
