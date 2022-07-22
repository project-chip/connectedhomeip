/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

#include <platform/CHIPDeviceLayer.h>

#include <AppConfig.h>
#include <AppTask.h>

#include <FreeRTOS.h>

/* Driver Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/UART.h>

#include <ti/drivers/AESECB.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/SHA2.h>

#include <bget.h>
#define TOTAL_ICALL_HEAP_SIZE (0xCB00)

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

__attribute__((section(".heap"))) uint8_t GlobalHeapZoneBuffer[TOTAL_ICALL_HEAP_SIZE];
uint32_t heapSize = TOTAL_ICALL_HEAP_SIZE;

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationStackOverflowHook(void)
{
    while (1)
    {
        ;
    }
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    Board_init();
    bpool((void *) GlobalHeapZoneBuffer, TOTAL_ICALL_HEAP_SIZE);

    GPIO_init();

    NVS_init();

    UART_init();

    ECDH_init();

    ECDSA_init();

    AESECB_init();

    SHA2_init();

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
