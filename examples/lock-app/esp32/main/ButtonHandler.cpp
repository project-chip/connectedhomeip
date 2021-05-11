/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ButtonHandler.h"
#include "AppTask.h"

#include "AppConfig.h"

TimerHandle_t buttonTimer;

void ButtonHandler::Init(void)
{
    buttonTimer = xTimerCreate("BtnTmr",                       // Just a text name, not used by the RTOS kernel
                               APP_BUTTON_DEBOUNCE_PERIOD_MS,  // timer period
                               false,                          // no timer reload (==one-shot)
                               (void *) (int) APP_LOCK_BUTTON, // init timer id = button index
                               TimerCallback                   // timer callback handler (all buttons use
                                                               // the same timer cn function)
    );
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.
    uint32_t timerId;

    timerId         = (uint32_t) pvTimerGetTimerID(xTimer);
    uint32_t btnIdx = timerId;
    gpio_num_t button;
    if (btnIdx == APP_LOCK_BUTTON)
    {
        button = APP_LOCK_BUTTON;
    }
    if (btnIdx == APP_FUNCTION_BUTTON)
    {
        button = APP_FUNCTION_BUTTON;
    }
    bool gpioPinPressed = gpio_get_level(button);
    GetAppTask().ButtonEventHandler(btnIdx, (gpioPinPressed) ? APP_BUTTON_PRESSED : APP_BUTTON_RELEASED);
}
