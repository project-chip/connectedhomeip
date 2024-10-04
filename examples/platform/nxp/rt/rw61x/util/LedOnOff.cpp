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

    if (strcmp(BOARD_NAME, "RD-RW61X-BGA") == 0)
    {
        // EVA board
        GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 1);
    }
    else if (strcmp(BOARD_NAME, "FRDM-RW612") == 0)
    {
        // FRDM board
        GPIO_PinWrite(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, 0);
    }
    else
    {
        ChipLogError(DeviceLayer, "Error: Unsupported Board Type");
    }
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
        if (strcmp(BOARD_NAME, "RD-RW61X-BGA") == 0)
            // eva-board
            GPIO_PortSet(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
        else if (strcmp(BOARD_NAME, "FRDM-RW612") == 0)
            // frdm-board
            GPIO_PortClear(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
    }
    else
    {
        // Turn off LED
        if (strcmp(BOARD_NAME, "RD-RW61X-BGA") == 0)
	        // eva-board
            GPIO_PortClear(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
        else if (strcmp(BOARD_NAME, "FRDM-RW612") == 0)
	        // frdm-board
            GPIO_PortSet(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
    }
}
} // namespace chip::NXP::App
