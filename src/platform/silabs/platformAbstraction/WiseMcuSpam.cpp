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

#include <FreeRTOS.h>
#include <task.h>

#if SILABS_LOG_ENABLED
#include "silabs_utils.h"
#endif

// TODO add includes ?
extern "C" {
#include "em_core.h"
#include "rsi_board.h"
#include "sl_event_handler.h"
#include "sl_si91x_led.h"
#include "sl_si91x_led_config.h"
void soc_pll_config(void);
}

#if SILABS_LOG_ENABLED
#include "silabs_utils.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace Silabs {

SilabsPlatform SilabsPlatform::sSilabsPlatformAbstractionManager;
SilabsPlatform::SilabsButtonCb SilabsPlatform::mButtonCallback = nullptr;

CHIP_ERROR SilabsPlatform::Init(void)
{
    mButtonCallback = nullptr;

    // TODO: Setting the highest priority for SVCall_IRQn to avoid the HardFault issue
    NVIC_SetPriority(SVCall_IRQn, CORE_INTERRUPT_HIGHEST_PRIORITY);

#if !CHIP_CONFIG_ENABLE_ICD_SERVER
    // Configuration the clock rate
    soc_pll_config();
#endif

#if SILABS_LOG_ENABLED
    silabsInitLog();
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
    if (Silabs::GetPlatform().mButtonCallback == nullptr)
    {
        return;
    }

    Silabs::GetPlatform().mButtonCallback(btn, btnAction);
}
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
