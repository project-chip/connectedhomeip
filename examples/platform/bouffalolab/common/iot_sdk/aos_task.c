/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#ifdef SYS_AOS_LOOP_ENABLE

#include <FreeRTOS.h>
#include <timers.h>

#include <vfs.h>
#ifdef CFG_USB_CDC_ENABLE
#include <device/vfs_uart.h>
#include <hal_board.h>
#include <stdint.h>
#endif

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>

#include <plat.h>
#include <uart.h>

static void aos_loop_proc(void * pvParameters)
{
    aos_loop_init();

#ifdef CFG_USB_CDC_ENABLE
    uartStartUsbCdc();
#endif

    aos_loop_run();

    vTaskDelete(NULL);
}

void aos_loop_start(void)
{
    static TaskHandle_t aos_loop_proc_task;

    vfs_init();
    vfs_device_init();

#ifdef CFG_USB_CDC_ENABLE
    uint32_t fdt = 0, offset = 0;
    const char * uart_node[] = {
        "uart@4000A000",
        "uart@4000A100",
    };

    if (0 == hal_board_get_dts_addr("uart", &fdt, &offset))
    {
        vfs_uart_init(fdt, offset, uart_node, 2);
    }
#endif

    xTaskCreate(aos_loop_proc, (char *) "event_loop", 4096, NULL, APP_TASK_PRIORITY, &aos_loop_proc_task);
}
#endif
