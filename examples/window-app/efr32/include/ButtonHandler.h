/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include <AppTimer.h>
#include <gpiointerrupt.h>
#include <hal-config-board.h>
#include <stdint.h>

class ButtonHandler
{
public:
    typedef struct ButtonConfig
    {
        GPIO_Port_TypeDef port;
        unsigned int pin;
    } ButtonConfig_t;

    typedef enum ButtonId
    {
        kButton_Up   = 0,
        kButton_Down = 1,
    } ButtonId_t;

    typedef enum ButtonEvent
    {
        kButton_Pressed  = 0,
        kButton_Released = 1,
        kButton_Timeout  = 2,
    } ButtonEvent_t;

    struct Button;

    struct Button
    {
        ButtonId_t mId;
        const char * mName;
        GPIO_Port_TypeDef mPort;
        unsigned int mPin;
        AppTimer mTimer;
        bool mIsPressed;

        Button(ButtonId_t mId, const char * tag);
        void Init(const ButtonConfig_t & config);
    };

    static ButtonHandler & Instance();

    void Init();
    bool IsPressed(ButtonId_t button);

    Button mButtonUp;
    Button mButtonDown;

private:
    ButtonHandler();

    static ButtonHandler sInstance;
    static void ButtonISR(uint8_t pin);
    static void ButtonTimerCallback(AppTimer & timer, void * context);
};
