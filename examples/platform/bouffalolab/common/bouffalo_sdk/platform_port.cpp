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

#include <FreeRTOS.h>
#include <task.h>

extern "C" {
#include <bflb_mtd.h>
#include <bl616dk/board.h>
}
#include <plat.h>

extern "C" void __libc_init_array(void);
extern "C" void shell_init_with_task(struct bflb_device_s * shell);

void platform_port_init(void)
{
    /*if need use xtal 32k please enable next API */
    board_init();
#if CONFIG_ENABLE_CHIP_SHELL
    struct bflb_device_s * uart0 = bflb_device_get_by_name("uart0");
    shell_init_with_task(uart0);
#endif

    __libc_init_array();

    bflb_mtd_init();
}

extern "C" void vAssertCalled(void)
{
    void * ra = (void *) __builtin_return_address(0);

    taskDISABLE_INTERRUPTS();
    if (xPortIsInsideInterrupt())
    {
        printf("vAssertCalled, ra = %p in ISR\r\n", (void *) ra);
    }
    else
    {
        printf("vAssertCalled, ra = %p in task %s\r\n", (void *) ra, pcTaskGetName(NULL));
    }

    abort();

    while (true)
        ;
}

extern "C" void bflb_assert(void) __attribute__((weak, alias("vAssertCalled")));
