/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
                                       APP_BUTTON_MIN_ASSERT_TIME_MS, // timer period
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
    result = cyhal_gpio_init(APP_LIGHT_BUTTON, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        printf(" cyhal_gpio_init failed for APP_LIGHT_BUTTON\r\n");
    }
    result = cyhal_gpio_init(APP_FUNCTION_BUTTON, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        printf(" cyhal_gpio_init failed for APP_FUNCTION_BUTTON\r\n");
    }
    /* Configure GPIO interrupt. */
    static cyhal_gpio_callback_data_t light_button_cbdata;
    static cyhal_gpio_callback_data_t func_button_cbdata;
    light_button_cbdata.callback     = light_button_callback;
    light_button_cbdata.callback_arg = NULL;
    cyhal_gpio_register_callback(APP_LIGHT_BUTTON, &light_button_cbdata);
    func_button_cbdata.callback     = func_button_callback;
    func_button_cbdata.callback_arg = NULL;
    cyhal_gpio_register_callback(APP_FUNCTION_BUTTON, &func_button_cbdata);
    cyhal_gpio_enable_event(APP_LIGHT_BUTTON, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
    cyhal_gpio_enable_event(APP_FUNCTION_BUTTON, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
}

void ButtonHandler::light_button_callback(void * handler_arg, cyhal_gpio_event_t event)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_LIGHT_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::func_button_callback(void * handler_arg, cyhal_gpio_event_t event)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_FUNCTION_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.
    uint32_t timerId;
    uint8_t buttonevent = 1;
    timerId             = (uint32_t) pvTimerGetTimerID(xTimer);

    switch (timerId)
    {
    case APP_LIGHT_BUTTON_IDX:
        buttonevent = cyhal_gpio_read(APP_LIGHT_BUTTON);
        break;
    case APP_FUNCTION_BUTTON_IDX:
        buttonevent = cyhal_gpio_read(APP_FUNCTION_BUTTON);
        break;
    default:
        P6_LOG("Unhandled TimerID: %d", timerId);
        break;
    }

    if (!buttonevent)
    {
        GetAppTask().ButtonEventHandler(timerId, APP_BUTTON_PRESSED);
    }
}
