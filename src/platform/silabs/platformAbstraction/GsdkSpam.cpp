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

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_rmu.h"
#else
#include "sl_hal_emu.h"
#endif // _SILICON_LABS_32B_SERIES_2
#include "sl_system_kernel.h"

#ifdef ENABLE_WSTK_LEDS
extern "C" {
#include "sl_simple_led_instances.h"
}
#endif

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button_instances.h"
#endif

extern "C" {
#include <mbedtls/platform.h>

#if CHIP_ENABLE_OPENTHREAD
#include "platform-efr32.h"

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#include "openthread/heap.h"
#endif // OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE

#endif // CHIP_ENABLE_OPENTHREAD

#include "sl_component_catalog.h"
#include "sl_mbedtls.h"
#if SILABS_LOG_OUT_UART || ENABLE_CHIP_SHELL || CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_iostream.h"
#include "sl_iostream_stdio.h"
#endif //
#include "uart.h"
#endif

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
#include "SEGGER_SYSVIEW.h"
#endif
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
#ifdef _SILICON_LABS_32B_SERIES_2
    // Read the cause of last reset.
    mRebootCause = RMU_ResetCauseGet();

    // Clear the register, as the causes cumulate over resets.
    RMU_ResetCauseClear();
#else
    // Read the cause of last reset.
    mRebootCause = sl_hal_emu_get_reset_cause();

    // Clear the register, as the causes cumulate over resets.
    sl_hal_emu_clear_reset_cause();
#endif // _SILICON_LABS_32B_SERIES_2

#if SILABS_LOG_OUT_UART && defined(SL_CATALOG_CLI_PRESENT)
    sl_iostream_set_default(sl_iostream_stdio_handle);
#endif

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
    SEGGER_SYSVIEW_Conf();
#endif

#if SILABS_LOG_OUT_UART || ENABLE_CHIP_SHELL || CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
    uartConsoleInit();
#endif

#if SILABS_LOG_ENABLED
    silabsInitLog();
#endif
    return CHIP_NO_ERROR;
}

#ifdef ENABLE_WSTK_LEDS
void SilabsPlatform::InitLed(void)
{
    sl_simple_led_init_instances();
}

CHIP_ERROR SilabsPlatform::SetLed(bool state, uint8_t led)
{
    if (led >= SL_SIMPLE_LED_COUNT)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    (state) ? sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(led)) : sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(led));
    return CHIP_NO_ERROR;
}

bool SilabsPlatform::GetLedState(uint8_t led)
{
    if (led >= SL_SIMPLE_LED_COUNT)
    {
        return false;
    }

    return sl_led_get_state(SL_SIMPLE_LED_INSTANCE(led));
}

CHIP_ERROR SilabsPlatform::ToggleLed(uint8_t led)
{
    if (led >= SL_SIMPLE_LED_COUNT)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    sl_led_toggle(SL_SIMPLE_LED_INSTANCE(led));
    return CHIP_NO_ERROR;
}
#endif // ENABLE_WSTK_LEDS

void SilabsPlatform::StartScheduler()
{
    sl_system_kernel_start();
}

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
extern "C" {
void sl_button_on_change(const sl_button_t * handle)
{
    if (Silabs::GetPlatform().mButtonCallback == nullptr)
    {
        return;
    }

    for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++)
    {
        if (SL_SIMPLE_BUTTON_INSTANCE(i) == handle)
        {
            Silabs::GetPlatform().mButtonCallback(i, sl_button_get_state(handle));
            break;
        }
    }
}
}

uint8_t SilabsPlatform::GetButtonState(uint8_t button)
{
    const sl_button_t * handle = SL_SIMPLE_BUTTON_INSTANCE(button);
    return nullptr == handle ? 0 : sl_button_get_state(handle);
}

#else
uint8_t SilabsPlatform::GetButtonState(uint8_t button)
{
    return 0;
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
