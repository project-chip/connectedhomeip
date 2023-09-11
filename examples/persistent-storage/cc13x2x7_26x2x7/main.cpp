/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform/CHIPDeviceLayer.h>

#include "KeyValueStorageTest.h"
#include <AppConfig.h>

#include <FreeRTOS.h>
#include <task.h>

/* Driver Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/UART.h>

#include <bget.h>
#define TOTAL_ICALL_HEAP_SIZE (0xf000)

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

// ================================================================================
// Main Code
// ================================================================================

static TaskHandle_t sTestTaskHandle;
void TestTask(void * pvParameter)
{
    cc13x2_26x2LogInit();

    chip::DeviceLayer::Internal::CC13X2_26X2Config().Init();

    while (true)
    {
        PLAT_LOG("Running Tests:");
        chip::RunKvsTest();
        vTaskDelay(60 * configTICK_RATE_HZ); // Run every minute
    }
}

int main(void)
{
    Board_init();
    bpool((void *) GlobalHeapZoneBuffer, TOTAL_ICALL_HEAP_SIZE);

    NVS_init();

    UART_init();

    // Run tests
    xTaskCreate(TestTask, "Test", 2048, NULL, 1, &sTestTaskHandle);

    vTaskStartScheduler();

    // Should never get here.
    while (true)
        ;
}
