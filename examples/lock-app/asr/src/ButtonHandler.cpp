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

#include "ButtonHandler.h"
#include "AppConfig.h"
#include "AppTask.h"

namespace {
constexpr int kButtonCount = 2;

TimerHandle_t buttonTimers[kButtonCount]; // FreeRTOS timers used for debouncing
// buttons. Array to hold handles to
// the created timers.

} // namespace

void ButtonHandler::Init(void)
{
    GpioInit();
    // Create FreeRTOS sw timers for debouncing buttons.
    for (uint8_t i = 0; i < kButtonCount; i++)
    {
        buttonTimers[i] = xTimerCreate("BtnTmr",                      // Just a text name, not used by the RTOS kernel
                                       APP_BUTTON_DEBOUNCE_PERIOD_MS, // timer period
                                       false,                         // no timer reload (==one-shot)
                                       (void *) (int) i,              // init timer id = button index
                                       TimerCallback                  // timer callback handler (all buttons use
                                                                      // the same timer cn function)
        );
    }
}

// port pin
duet_gpio_dev_t lock_btn;
duet_gpio_dev_t func_btn;

void ButtonHandler::GpioInit(void)
{
    // lock button
    lock_btn.port   = APP_LOCK_BUTTON;
    lock_btn.config = DUET_INPUT_PULL_UP;
    lock_btn.priv   = NULL;
    duet_gpio_init(&lock_btn);
    duet_gpio_enable_irq(&lock_btn, DUET_IRQ_TRIGGER_FALLING_EDGE, lockbuttonIsr, NULL);

    // function button
    func_btn.port   = APP_FUNCTION_BUTTON;
    func_btn.config = DUET_INPUT_PULL_UP;
    func_btn.priv   = NULL;
    duet_gpio_init(&func_btn);
    duet_gpio_enable_irq(&func_btn, DUET_IRQ_TRIGGER_FALLING_EDGE, functionbuttonIsr, NULL);
}

void ButtonHandler::lockbuttonIsr(void * handler_arg)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_LOCK_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::functionbuttonIsr(void * handler_arg)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_FUNCTION_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.
    uint32_t timerId;
    uint8_t buttonevent = 0;
    uint32_t btnValue;

    timerId = (uint32_t) pvTimerGetTimerID(xTimer);
    if (timerId == APP_LOCK_BUTTON_IDX)
    {
        duet_gpio_input_get(&lock_btn, &btnValue);
        buttonevent = (uint8_t) btnValue;
    }
    else
    {
        duet_gpio_input_get(&func_btn, &btnValue);
        buttonevent = (uint8_t) btnValue;
    }
    GetAppTask().ButtonEventHandler(timerId, (buttonevent) ? APP_BUTTON_PRESSED : APP_BUTTON_RELEASED);
}
