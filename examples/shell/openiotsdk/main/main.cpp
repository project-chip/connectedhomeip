/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os2.h"

static void demo_thread(void * argument)
{
    for (int i = 0; i < 5; ++i)
    {
        printf("Message from demo thread\r\n");
        osDelay(500);
    }
}

int main()
{
    printf("CHIP demo example\r\n");
    printf("CHIP port: %d\r\n", CHIP_PORT);

    osStatus_t ret = osKernelInitialize();
    if (ret != osOK)
    {
        printf("osKernelInitialize failed: %d\r\n", ret);
        return EXIT_FAILURE;
    }

    static const osThreadAttr_t thread_attr = {
        .stack_size = 1024 // Allocate our threads with enough stack for printf
    };
    osThreadId_t thread = osThreadNew(demo_thread, NULL, &thread_attr);
    if (thread == NULL)
    {
        printf("Failed to create thread\r\n");
        return EXIT_FAILURE;
    }

    osKernelState_t state = osKernelGetState();
    if (state == osKernelReady)
    {
        printf("Starting kernel\r\n");
        ret = osKernelStart();
        if (ret != osOK)
        {
            printf("Failed to start kernel: %d\r\n", ret);
            return EXIT_FAILURE;
        }
    }
    else
    {
        printf("Kernel not ready: %d\r\n", state);
        return EXIT_FAILURE;
    }

    return 0;
}
