/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <sl_si91x_button_pin_config.h>

#include <FreeRTOS.h>
#include <task.h>

#include <app/icd/server/ICDServerConfig.h>

#if SILABS_LOG_ENABLED
#include "silabs_utils.h"
#endif // SILABS_LOG_ENABLED

// TODO add includes ?
extern "C" {
#include "em_core.h"
#include "rsi_board.h"
#include "sl_event_handler.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_led.h"
#include "sl_si91x_led_config.h"

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 0
void soc_pll_config(void);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
}

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
#include "SEGGER_SYSVIEW.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace {
uint8_t sButtonStates[SL_SI91x_BUTTON_COUNT] = { 0 };
#if CHIP_CONFIG_ENABLE_ICD_SERVER
bool btn0_pressed = false;
#endif /* SL_ICD_ENABLED */
} // namespace

SilabsPlatform SilabsPlatform::sSilabsPlatformAbstractionManager;
SilabsPlatform::SilabsButtonCb SilabsPlatform::mButtonCallback = nullptr;

CHIP_ERROR SilabsPlatform::Init(void)
{
    mButtonCallback = nullptr;

    // TODO: Setting the highest priority for SVCall_IRQn to avoid the HardFault issue
    NVIC_SetPriority(SVCall_IRQn, CORE_INTERRUPT_HIGHEST_PRIORITY);

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 0
    // Configuration the clock rate
    soc_pll_config();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if SILABS_LOG_ENABLED
    silabsInitLog();
#endif

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
    SEGGER_SYSVIEW_Conf();
#endif

    return CHIP_NO_ERROR;
}

#ifdef ENABLE_WSTK_LEDS
void SilabsPlatform::InitLed(void)
{
    // TODO
    RSI_Board_Init();
    SilabsPlatformAbstractionBase::InitLed();
}

CHIP_ERROR SilabsPlatform::SetLed(bool state, uint8_t led)
{
    // TODO add range check
    (state) ? sl_si91x_led_set(led ? SL_LED_LED1_PIN : SL_LED_LED0_PIN)
            : sl_si91x_led_clear(led ? SL_LED_LED1_PIN : SL_LED_LED0_PIN);
    return CHIP_NO_ERROR;
}

bool SilabsPlatform::GetLedState(uint8_t led)
{
    // TODO
    return SilabsPlatformAbstractionBase::GetLedState(led);
}

CHIP_ERROR SilabsPlatform::ToggleLed(uint8_t led)
{
    // TODO add range check
    sl_si91x_led_toggle(led ? SL_LED_LED1_PIN : SL_LED_LED0_PIN);
    return CHIP_NO_ERROR;
}
#endif // ENABLE_WSTK_LEDS

void SilabsPlatform::StartScheduler()
{
    vTaskStartScheduler();
}

extern "C" {
void sl_button_on_change(uint8_t btn, uint8_t btnAction)
{
#if SL_ICD_ENABLED
    // This is to make sure we get a one-press and one-release event for the button
    // Hardware modification will be required for this to work permanently
    // Currently the btn0 is pull-up resistor due to which is sends a release event on every wakeup
    if (btn == SL_BUTTON_BTN0_NUMBER)
    {
        if (btnAction == BUTTON_PRESSED)
        {
            btn0_pressed = true;
        }
        else if ((btnAction == BUTTON_RELEASED) && (btn0_pressed == false))
        {
            // if the btn was not pressed and only a release event came, ignore it
            return;
        }
        else if ((btnAction == BUTTON_RELEASED) && (btn0_pressed == true))
        {
            btn0_pressed = false;
        }
    }
#endif /* SL_ICD_ENABLED */
    if (Silabs::GetPlatform().mButtonCallback == nullptr)
    {
        return;
    }

    if (btn < SL_SI91x_BUTTON_COUNT)
    {
        sButtonStates[btn] = btnAction;
    }
    Silabs::GetPlatform().mButtonCallback(btn, btnAction);
}
}

uint8_t SilabsPlatform::GetButtonState(uint8_t button)
{
    return (button < SL_SI91x_BUTTON_COUNT) ? sButtonStates[button] : 0;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
