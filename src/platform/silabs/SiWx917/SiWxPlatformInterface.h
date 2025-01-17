/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/icd/server/ICDServerConfig.h>

namespace {
#ifdef ENABLE_CHIP_SHELL
bool ps_requirement_added = false;
#endif // ENABLE_CHIP_SHELL
} // namespace

#ifdef __cplusplus
extern "C" {
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if SLI_SI91X_MCU_INTERFACE
#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_power_manager.h"

/**
 * @brief      invoked when button press event is received when in sleep
 * @param[in]  pin_intr GPIO pin interrupt number.
 * @return     none.
 * @note       this is a callback from the Wiseconnect SDK
 */
void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
    // UULP_GPIO_2 is used to detect the button 0 press
    VerifyOrReturn(pin_intr == RTE_UULP_GPIO_2_PIN, ChipLogError(DeviceLayer, "invalid pin interrupt: %ld", pin_intr));
    sl_status_t status      = SL_STATUS_OK;
    uint8_t pin_intr_status = sl_si91x_gpio_get_uulp_npss_pin(pin_intr);
    if (pin_intr_status == LOW)
    {
        // BTN_0 is pressed
        // NOTE: the GPIO is masked since the interrupt is invoked before scheduler is started, thus this is required to hand over
        // control to scheduler, the PIN is unmasked in the power manager flow before going to sleep
        status = sl_si91x_gpio_driver_mask_uulp_npss_interrupt(BIT(pin_intr));
        VerifyOrReturn(status == SL_STATUS_OK, ChipLogError(DeviceLayer, "failed to mask interrupt: %ld", status));
    }
}

#endif // SLI_SI91X_MCU_INTERFACE
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#ifdef __cplusplus
}
#endif

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace SiWxPlatformInterface {

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if SLI_SI91X_MCU_INTERFACE
/**
 * @brief      Required to invoke button press event during sleep as falling edge is not detected
 * @param[in]  none.
 * @note       flow is GPIO wakeup due to BTN0 press -> check button state in idle task required as the GPIO interrupt is not
 * detected during sleep for BUTTON RELEASED
 */
inline void sl_si91x_btn_event_handler()
{
    sl_button_on_change(SL_BUTTON_BTN0_NUMBER,
                        (sl_si91x_gpio_get_uulp_npss_pin(SL_BUTTON_BTN0_PIN) == LOW) ? BUTTON_PRESSED : BUTTON_RELEASED);
}

/**
 * @brief      Required to enable MATTER shell UART with ICD feature flag
 * @param[in]  none.
 * @note       this requires hardware jumping of the GPIO PINs to work with the baseboard.
 */
void sl_si91x_uart_power_requirement_handler()
{
#ifdef ENABLE_CHIP_SHELL
    // Checking the UULP PIN 1 status to reinit the UART and not allow the device to go to sleep
    if (sl_si91x_gpio_get_uulp_npss_pin(RTE_UULP_GPIO_1_PIN))
    {
        if (!ps_requirement_added)
        {
            sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
            ps_requirement_added = true;
        }
    }
    else
    {
        if (ps_requirement_added)
        {
            sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
            ps_requirement_added = false;
        }
    }
#endif // ENABLE_CHIP_SHELL
}

#endif // SLI_SI91X_MCU_INTERFACE
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
} // namespace SiWxPlatformInterface
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
