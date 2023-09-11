/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"

#include <zephyr/logging/log.h>

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)
static int InitUSB()
{
    int err = usb_enable(nullptr);

    if (err)
    {
        LOG_ERR("Failed to initialize USB device");
        return err;
    }

    const struct device * dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    uint32_t dtr              = 0;

    while (!dtr)
    {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        k_sleep(K_MSEC(100));
    }

    return 0;
}
#endif

int main()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)
    err = System::MapErrorZephyr(InitUSB());
#endif

    if (err == CHIP_NO_ERROR)
    {
        err = AppTask::Instance().StartApp();
    }

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
