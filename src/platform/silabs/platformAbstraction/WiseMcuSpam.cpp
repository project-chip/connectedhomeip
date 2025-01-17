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
#include <sl_si91x_common_flash_intf.h>

#include <FreeRTOS.h>
#include <task.h>

#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/CodeUtils.h>

#include <lib/support/CodeUtils.h>
#if SILABS_LOG_ENABLED
#include "silabs_utils.h"

#endif // SILABS_LOG_ENABLED

// TODO add includes ?
extern "C" {
#include "em_core.h"
#include "rsi_board.h"
#include "sl_event_handler.h"

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

#ifdef ENABLE_WSTK_LEDS
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include "sl_si91x_rgb_led.h"
#include "sl_si91x_rgb_led_config.h"
#include "sl_si91x_rgb_led_instances.h"
#define SL_LED_COUNT SL_SI91X_RGB_LED_COUNT
const sl_rgb_led_t * ledPinArray[SL_LED_COUNT] = { &led_led0 };
#define SL_RGB_LED_INSTANCE(n) (ledPinArray[n])
#else
#include "sl_si91x_led.h"
#include "sl_si91x_led_config.h"
#include "sl_si91x_led_instances.h"
#define SL_LED_COUNT SL_SI91x_LED_COUNT
uint8_t ledPinArray[SL_LED_COUNT] = { SL_LED_LED0_PIN, SL_LED_LED1_PIN };
#endif // SL_MATTER_RGB_LED_ENABLED
#endif // ENABLE_WSTK_LEDS

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 0
void soc_pll_config(void);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
}

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
#include "SEGGER_SYSVIEW.h"
#endif

#if SILABS_LOG_OUT_UART || ENABLE_CHIP_SHELL
#include "uart.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace {

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
uint8_t sButtonStates[SL_SI91x_BUTTON_COUNT] = { 0 };
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

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

#if SILABS_LOG_OUT_UART || ENABLE_CHIP_SHELL
    uartConsoleInit();
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
    VerifyOrReturnError(led < SL_LED_COUNT, CHIP_ERROR_INVALID_ARGUMENT);
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    (state) ? sl_si91x_simple_rgb_led_on(SL_RGB_LED_INSTANCE(led)) : sl_si91x_simple_rgb_led_off(SL_RGB_LED_INSTANCE(led));
#else
    (state) ? sl_si91x_led_set(ledPinArray[led]) : sl_si91x_led_clear(ledPinArray[led]);
#endif // defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED
    return CHIP_NO_ERROR;
}

bool SilabsPlatform::GetLedState(uint8_t led)
{
    // TODO
    return SilabsPlatformAbstractionBase::GetLedState(led);
}

CHIP_ERROR SilabsPlatform::ToggleLed(uint8_t led)
{
    VerifyOrReturnError(led < SL_LED_COUNT, CHIP_ERROR_INVALID_ARGUMENT);
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    sl_si91x_simple_rgb_led_toggle(SL_RGB_LED_INSTANCE(led));
#else
    sl_si91x_led_toggle(ledPinArray[led]);
#endif // defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED
    return CHIP_NO_ERROR;
}
#endif // ENABLE_WSTK_LEDS

void SilabsPlatform::StartScheduler()
{
    vTaskStartScheduler();
}

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
extern "C" void sl_button_on_change(uint8_t btn, uint8_t btnAction)
{
#if SL_ICD_ENABLED
    // This is to make sure we get a one-press and one-release event for the button
    // Hardware modification will be required for this to work permanently
    // Currently the btn0 is pull-up resistor due to which is sends a release event on every wakeup
    if (btn == SL_BUTTON_BTN0_NUMBER)
    {
        // if the btn was not pressed and only a release event came, ignore it
        // if the btn was already pressed and another press event came, ignore it
        // essentially, if both of them are in the same state then ignore it.
        VerifyOrReturn(btnAction != btn0_pressed);

        if ((btnAction == BUTTON_PRESSED) && (btn0_pressed == false))
        {
            btn0_pressed = true;
        }
        else if ((btnAction == BUTTON_RELEASED) && (btn0_pressed == true))
        {
            btn0_pressed = false;
        }
    }
#endif // SL_ICD_ENABLED
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

uint8_t SilabsPlatform::GetButtonState(uint8_t button)
{
    return (button < SL_SI91x_BUTTON_COUNT) ? sButtonStates[button] : 0;
}
#else
uint8_t SilabsPlatform::GetButtonState(uint8_t button)
{
    return 0;
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

CHIP_ERROR SilabsPlatform::FlashInit()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SilabsPlatform::FlashErasePage(uint32_t addr)
{
    rsi_flash_erase_sector((uint32_t *) addr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SilabsPlatform::FlashWritePage(uint32_t addr, const uint8_t * data, size_t size)
{
    rsi_flash_write((uint32_t *) addr, (unsigned char *) data, size);
    return CHIP_NO_ERROR;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
