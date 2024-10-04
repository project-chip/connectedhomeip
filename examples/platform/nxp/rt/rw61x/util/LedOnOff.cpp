/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "LedOnOff.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <system/SystemClock.h>
extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_io_mux.h"
#include "pin_mux.h"
}

namespace chip::NXP::App {

void LedOnOff::Init(uint8_t index, bool inverted)
{
    mLastChangeTimeMS = 0;
    mOnTimeMS         = 0;
    mOffTimeMS        = 0;
    mIndex            = index;
    mState            = false;
    mOnLogic          = !inverted;

    /* Initialize the GPIO pins */
    BOARD_InitPinLEDRGB();

    uint32_t port_state = 0;
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };

    GPIO_PortInit(GPIO, BOARD_LED_RED_GPIO_PORT);
    GPIO_PinInit(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, &led_config);

#if (BOARD_TYPE == 1)
    // EVA board
    GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 1);
#elif (BOARD_TYPE == 0)
    // FRDM board
    GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 0);
#else
    #pragma error "Unsupported Board Type, 0: Frdm_board, 1: eva_board"
#endif

    Set(false);
}

void LedOnOff::Set(uint8_t level)
{
    mLastChangeTimeMS = mOnTimeMS = mOffTimeMS = 0;

    DoSet(level != 0);
}

void LedOnOff::Animate(uint32_t onTimeMS, uint32_t offTimeMS)
{
    // TBD
}

void LedOnOff::DoSet(bool state)
{
    mState = state;

    if (state)
    {
        // Turn on LED
#if (BOARD_TYPE == 1)
        // eva-board
        GPIO_PortSet(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
#elif (BOARD_TYPE == 0)
        // frdm-boar
        GPIO_PortClear(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
#endif
    }
    else
    {
        // Turn off LED
#if (BOARD_TYPE == 1)
	// eva-board
        GPIO_PortClear(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
#elif (BOARD_TYPE == 0)
	// frdm-board
        GPIO_PortSet(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
#endif
    }
}
} // namespace chip::NXP::App
