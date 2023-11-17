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

#include <stdbool.h>
#include <stdint.h>

#include <platform/CHIPDeviceLayer.h>

#include <AppTask.h>

#include <FreeRTOS.h>

/* Driver Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/NVS.h>

#include <ti/drivers/AESECB.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/SHA2.h>

#include <bget.h>
#define TOTAL_ICALL_HEAP_SIZE (0xc700)

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
    while (true)
    {
        ;
    }
}

/* Wrapper functions for using the queue registry regardless of whether it is enabled or disabled */
extern "C" void vQueueAddToRegistryWrapper(QueueHandle_t xQueue, const char * pcQueueName)
{
    /* This function is intentionally left empty as the Queue Registry is disabled */
}

extern "C" void vQueueUnregisterQueueWrapper(QueueHandle_t xQueue)
{
    /* This function is intentionally left empty as the Queue Registry is disabled */
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    CHIP_ERROR err;

    Board_init();
    bpool((void *) GlobalHeapZoneBuffer, TOTAL_ICALL_HEAP_SIZE);

    GPIO_init();

    NVS_init();

    ECDH_init();

    ECDSA_init();

    ECJPAKE_init();

    AESECB_init();

    SHA2_init();

    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        // can't log until the kernel is started
        // PLAT_LOG("GetAppTask().StartAppTask() failed");
        while (true)
            ;
    }

    vTaskStartScheduler();

    // Should never get here.
    while (true)
        ;
}
