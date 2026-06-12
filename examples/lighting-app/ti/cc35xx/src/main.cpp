/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdbool.h>
#include <stdint.h>

#include "AppConfig.h"
#include "AppTask.h"

#include <FreeRTOS.h>

/* Driver Header files */
#include <ti/drivers/Board.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

extern "C" void initialize_mbedtls_threading(void);

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
// FreeRTOS Hook Functions
// Note: vQueueAddToRegistryWrapper and vQueueUnregisterQueueWrapper are
// provided by the SysConfig-generated ti_freertos_config.c

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    PLAT_LOG("Stack overflow detected in task: %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for(;;);
}

extern "C" void vApplicationMallocFailedHook(void) {
    PLAT_LOG("Malloc failed!\n");
    taskDISABLE_INTERRUPTS();
    for(;;);
}


// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    Board_init();

    /* This initializes mutex related hook funcs for threading
       Should be called first before any mbedTLS / PSA funcs
    */
    initialize_mbedtls_threading();

    int ret = GetAppTask().StartAppTask();
    if (ret != 0)
    {
        // can't log until the kernel is started
        // PLAT_LOG("GetAppTask().StartAppTask() failed");
        while (1)
            ;
    }

    vTaskStartScheduler();

    // Should never get here.
    while (1)
        ;
}
