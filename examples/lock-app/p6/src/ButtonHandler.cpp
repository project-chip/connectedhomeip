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

void ButtonHandler::GpioInit(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    // Set up button GPIOs to input with pullups.
    result = cyhal_gpio_init(APP_LOCK_BUTTON, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        printf(" cyhal_gpio_init failed for APP_LOCK_BUTTON\r\n");
    }
    result = cyhal_gpio_init(APP_FUNCTION_BUTTON, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        printf(" cyhal_gpio_init failed for APP_FUNCTION_BUTTON\r\n");
    }
    /* Configure GPIO interrupt. */
    cyhal_gpio_register_callback(APP_LOCK_BUTTON, lockbuttonIsr, NULL);
    cyhal_gpio_register_callback(APP_FUNCTION_BUTTON, functionbuttonIsr, NULL);
    cyhal_gpio_enable_event(APP_LOCK_BUTTON, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
    cyhal_gpio_enable_event(APP_FUNCTION_BUTTON, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
}
void ButtonHandler::lockbuttonIsr(void * handler_arg, cyhal_gpio_event_t event)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_LOCK_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::functionbuttonIsr(void * handler_arg, cyhal_gpio_event_t event)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_FUNCTION_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.
    uint32_t timerId;
    uint8_t buttonevent = 0;
    timerId             = (uint32_t) pvTimerGetTimerID(xTimer);
    if (timerId)
    {
        buttonevent = cyhal_gpio_read(APP_FUNCTION_BUTTON);
    }
    else
    {
        buttonevent = cyhal_gpio_read(APP_LOCK_BUTTON);
    }
    GetAppTask().ButtonEventHandler(timerId, (buttonevent) ? APP_BUTTON_PRESSED : APP_BUTTON_RELEASED);
}
