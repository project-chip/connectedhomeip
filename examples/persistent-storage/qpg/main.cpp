/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "KeyValueStorageTest.h"
#include <platform/KeyValueStoreManager.h>

#define APP_NAME "KVS-Test"
#define APP_TASK_STACK_SIZE (3 * 1024)

#define LOG_MODULE_ID 1

namespace {
StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;
} // namespace

void TestTask(void * pvParameter)
{
    while (true)
    {
        qvCHIP_Printf(LOG_MODULE_ID, "Running Tests:");
        chip::RunKvsTest();
        vTaskDelay(60000); // Run every minute
    }
}

void Application_Init(void)
{
    /* Launch application task */
    qvCHIP_Printf(LOG_MODULE_ID, "============================");
    qvCHIP_Printf(LOG_MODULE_ID, "Qorvo " APP_NAME " Launching");
    qvCHIP_Printf(LOG_MODULE_ID, "============================");

    // Run tests
    xTaskCreateStatic(TestTask, APP_NAME, 2048, NULL, 1, appStack, &appTaskStruct);
}

int main(void)
{

    int result;

    /* Initialize Qorvo stack */
    result = qvCHIP_init(Application_Init);
    if (result < 0)
    {
        goto exit;
    }

    qvCHIP_Printf(LOG_MODULE_ID, "Starting FreeRTOS scheduler");
    vTaskStartScheduler();

    // Should never get here.
    qvCHIP_Printf(LOG_MODULE_ID, "vTaskStartScheduler() failed");

exit:
    return 0;
}
