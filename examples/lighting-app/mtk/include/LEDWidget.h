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

class LEDWidget
{
public:
    // bind this LEDWidget with the specified LED
    void Init(enum led_id led);
    // retrieve the name of this LED
    const char * Name(void);
    // set to ON or OFF, no blink
    void Set(bool state);
    // change the color
    void Color(enum led_color color);
    // specify the ON, OFF duration
    void Blink(int on, int off);
    // specify evenly ON and OFF both to 'duration'
    void Blink(int duration);

private:
    enum led_id mLed;
    enum led_color mColor;
    int mOn;
    int mOff;
    bool mState;
    void Toggle(void);
    void DoSet(bool state);

    TimerHandle_t mTimer;
    static void TimerHandler(TimerHandle_t xTimer);
    void DoBlink(void);
    void StartTimer(uint32_t aTimeoutInMs);
    void CancelTimer(void);
};
