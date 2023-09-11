/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONFIG_BOARD_NRF52840DONGLE_NRF52840
#error "The command for triggerring the DFU is available for nRF52840 Dongle only"
#endif

#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

namespace {
constexpr const char * kGPIOController = "GPIO_0";
constexpr gpio_pin_t kGPIOResetPin     = 19;

int cmd_dfu(const struct shell * shell, size_t argc, char ** argv)
{
    // nRF52840 Dongle contains immutable bootloader which supports the DFU over a serial port.
    // Normally, a user must press the Reset button to reboot into the bootloader, but it can
    // also be done programatically by setting an appropriate GPIO pin.

    const device * gpioController = device_get_binding(kGPIOController);

    if (!gpioController)
    {
        shell_fprintf(shell, SHELL_NORMAL, "Cannot find GPIO controller");
        return -ENOEXEC;
    }

    if (gpio_pin_configure(gpioController, kGPIOResetPin, GPIO_OUTPUT))
    {
        shell_fprintf(shell, SHELL_NORMAL, "Cannot configure GPIO reset pin");
        return -ENOEXEC;
    }

    if (gpio_pin_set_raw(gpioController, kGPIOResetPin, 0))
    {
        shell_fprintf(shell, SHELL_NORMAL, "Cannot set GPIO reset pin");
        return -ENOEXEC;
    }

    return 0;
}
} // namespace

SHELL_CMD_ARG_REGISTER(dfu, NULL, "Trigger DFU over serial port", cmd_dfu, 0, 0);
