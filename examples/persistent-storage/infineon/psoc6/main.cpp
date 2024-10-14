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

#include "AppConfig.h"
#include "KeyValueStorageTest.h"
#include "init_psoc6Platform.h"
#include <FreeRTOS.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <task.h>

using namespace chip;

static TaskHandle_t sTestTaskHandle;
void TestTask(void * pvParameter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    if (err != CHIP_NO_ERROR)
    {
        PSOC6_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        return;
    }
    while (true)
    {
        PSOC6_LOG("Running Tests:");
        chip::RunKvsTest();
        vTaskDelay(60000); // Run every minute
    }
}

extern "C" void vApplicationDaemonTaskStartupHook()
{
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    // Run tests
    xTaskCreate(TestTask, "Test", 2048, NULL, 1, &sTestTaskHandle);
}

int main(int argc, char * argv[])
{
    init_psoc6Platform();

    PSOC6_LOG("=============================================\n");
    PSOC6_LOG("chip-psoc6-persistent-storage-example starting\n");
    PSOC6_LOG("=============================================\n");

    PSOC6_LOG("Starting FreeRTOS scheduler");

    vTaskStartScheduler();

    // Should never get here.
    PSOC6_LOG("vTaskStartScheduler() failed");
    return -1;
}
