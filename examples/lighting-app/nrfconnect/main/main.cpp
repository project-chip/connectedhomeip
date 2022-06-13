/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"

#include <system/SystemError.h>

#include <zephyr/logging/log.h>

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(app, CONFIG_MATTER_LOG_LEVEL);

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

#ifdef CONFIG_CHIP_PW_RPC
    rpc::Init();
#endif

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)
    err = System::MapErrorZephyr(InitUSB());
#endif

    if (err == CHIP_NO_ERROR)
    {
        err = GetAppTask().StartApp();
    }

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
