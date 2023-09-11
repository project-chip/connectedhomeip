/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        P6_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        return;
    }
    while (true)
    {
        P6_LOG("Running Tests:");
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
    init_p6Platform();

    P6_LOG("=============================================\n");
    P6_LOG("chip-p6-persistent-storage-example starting\n");
    P6_LOG("=============================================\n");

    P6_LOG("Starting FreeRTOS scheduler");

    vTaskStartScheduler();

    // Should never get here.
    P6_LOG("vTaskStartScheduler() failed");
    return -1;
}
