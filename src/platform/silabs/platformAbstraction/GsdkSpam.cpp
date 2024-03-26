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

#include "em_rmu.h"
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
    mRebootCause = RMU_ResetCauseGet();
    // Clear register so it does accumualate the causes of each reset
    RMU_ResetCauseClear();

#if CHIP_ENABLE_OPENTHREAD
    sl_ot_sys_init();
#endif

#ifdef SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();
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
#endif

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
