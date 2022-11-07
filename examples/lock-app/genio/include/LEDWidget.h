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

#include "ColorFormat.h"
#include "DimmableLEDIf.h"
#include <hal_gpio.h>

enum led_id
{
    LED_LIGHT,
    LED_STATUS
};

enum led_color
{
    LED_RED,
    LED_GREEN,
    LED_BLUE
};

class LEDWidget : public DimmableLEDIf
{
public:
    // bind this LEDWidget with the specified LED
    void Init(enum led_id led);
    // retrieve the name of this LED
    const char * Name(void);
    // change the color
    void Color(RgbColor_t color);
    // specify the ON, OFF duration
    void Blink(int on, int off);
    // specify evenly ON and OFF both to 'duration'
    void Blink(int duration);

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
    enum led_id mLed;
    int mOn;
    int mOff;
    uint8_t mLevel; /* Current level */
    bool mState;
    void Toggle(void);
    void DoSet(bool state);

    TimerHandle_t mTimer;
    static void TimerHandler(TimerHandle_t xTimer);
    void DoBlink(void);
    void StartTimer(uint32_t aTimeoutInMs);
    void CancelTimer(void);
};
