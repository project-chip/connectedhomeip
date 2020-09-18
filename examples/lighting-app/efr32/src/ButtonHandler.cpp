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
#include "AppTask.h"

#include "AppConfig.h"

#include "bsp.h"
#include "gpiointerrupt.h"
#include "hal-config-board.h"

typedef struct ButtonArray
{
    GPIO_Port_TypeDef port;
    unsigned int pin;
} ButtonArray_t;

static const ButtonArray_t sButtonArray[BSP_BUTTON_COUNT] = BSP_BUTTON_INIT; // GPIO info for the 2 WDTK buttons.
TimerHandle_t buttonTimers[BSP_BUTTON_COUNT];                                // FreeRTOS timers used for debouncing
// buttons. Array to hold handles to
// the created timers.

void ButtonHandler::Init(void)
{
    GpioInit();

    // Create FreeRTOS sw timers for debouncing buttons.
    for (uint8_t i = 0; i < BSP_BUTTON_COUNT; i++)
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
    // Set up button GPIOs to input with pullups.
    for (uint8_t i = 0; i < BSP_BUTTON_COUNT; i++)
    {
        GPIO_PinModeSet(sButtonArray[i].port, sButtonArray[i].pin, gpioModeInputPull, 1);
    }
    // Set up interrupt based callback function - trigger on both edges.
    GPIOINT_Init();
    GPIOINT_CallbackRegister(sButtonArray[0].pin, Button0Isr);
    GPIOINT_CallbackRegister(sButtonArray[1].pin, Button1Isr);
    GPIO_IntConfig(sButtonArray[0].port, sButtonArray[0].pin, true, true, true);
    GPIO_IntConfig(sButtonArray[1].port, sButtonArray[1].pin, true, true, true);

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);
}

void ButtonHandler::Button0Isr(uint8_t pin)
{
    // ISR for Button 0.
    uint8_t btnIdx = 0;

    if (pin == sButtonArray[btnIdx].pin)
    {
        EventHelper(btnIdx, true); // true== 'isr context'
    }
}

void ButtonHandler::Button1Isr(uint8_t pin)
{
    // ISR for Button 1.
    uint8_t btnIdx = 1;

    if (pin == sButtonArray[btnIdx].pin)
    {
        EventHelper(btnIdx, true); // true== 'isr context'
    }
}

void ButtonHandler::EventHelper(uint8_t btnIdx, bool isrContext)
{
    // May be called from Interrupt context so keep it short!

    if (btnIdx < BSP_BUTTON_COUNT)
    {
        // Get buton gpio pin state.
        bool gpioPinPressed = !GPIO_PinInGet(sButtonArray[btnIdx].port, sButtonArray[btnIdx].pin);

        if (isrContext)
        {
            portBASE_TYPE taskWoken = pdFALSE; // For FreeRTOS timer (below).

            // Start/restart the button debounce timer (Note ISR version of FreeRTOS
            // api call here).
            xTimerStartFromISR(buttonTimers[btnIdx], &taskWoken);

            if (taskWoken != pdFALSE)
            {
                taskYIELD();
            }
        }
        else
        {
            // Called by debounce timer expiry (this indicates that button gpio
            // is now stable).
            // Note- NOT in isr context at this point.

            // Notify AppTask of button state change.
            GetAppTask().ButtonEventHandler(btnIdx, (gpioPinPressed) ? APP_BUTTON_PRESSED : APP_BUTTON_RELEASED);
        }
    }
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.

    uint32_t timerId;

    timerId = (uint32_t) pvTimerGetTimerID(xTimer);
    if (timerId < BSP_BUTTON_COUNT)
    {
        uint8_t btnIdx = timerId;
        EventHelper(btnIdx, false); // false== 'not from isr context'
    }
}
