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

#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "AppConfig.h"
#include "KeyValueStorageTest.h"
#include "init_efrPlatform.h"
#include "sl_system_kernel.h"
#include <platform/KeyValueStoreManager.h>

static TaskHandle_t sTestTaskHandle;
void TestTask(void * pvParameter)
{
    while (1)
    {
        EFR32_LOG("Running Tests:");
        chip::RunKvsTest();
        vTaskDelay(60000); // Run every minute
    }
}

int main(void)
{
    init_efrPlatform();

    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-persitent-storage-example starting");
    EFR32_LOG("==================================================");

    // Run tests
    xTaskCreate(TestTask, "Test", 2048, NULL, 1, &sTestTaskHandle);
    EFR32_LOG("Starting FreeRTOS scheduler");
    sl_system_kernel_start();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    return -1;
}
