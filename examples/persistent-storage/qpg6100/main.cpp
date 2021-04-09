/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#define LOG_MODULE_ID 1

static TaskHandle_t sTestTaskHandle;
void TestTask(void * pvParameter)
{
    while (1)
    {
        qvCHIP_Printf(LOG_MODULE_ID, "Running Tests:");
        chip::RunKvsTest();
        vTaskDelay(60000); // Run every minute
    }
}

int main(void)
{

    int result;

    /* Initialize Qorvo stack */
    result = qvCHIP_init();
    if (result < 0)
    {
        goto exit;
    }

    /* Launch application task */
    qvCHIP_Printf(LOG_MODULE_ID, "============================");
    qvCHIP_Printf(LOG_MODULE_ID, "Qorvo " APP_NAME " Launching");
    qvCHIP_Printf(LOG_MODULE_ID, "============================");

    // Run tests
    xTaskCreate(TestTask, "Test", 2048, NULL, 1, &sTestTaskHandle);
    qvCHIP_Printf(LOG_MODULE_ID, "Starting FreeRTOS scheduler");
    vTaskStartScheduler();

    // Should never get here.
    qvCHIP_Printf(LOG_MODULE_ID, "vTaskStartScheduler() failed");

exit:
    return 0;
}
