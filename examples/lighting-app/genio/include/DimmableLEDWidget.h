/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
