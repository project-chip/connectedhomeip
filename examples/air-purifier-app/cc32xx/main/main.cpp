/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <platform/CHIPDeviceLayer.h>

#include <AppConfig.h>
#include <AppTask.h>

#include <FreeRTOS.h>

/* Driver Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/NVS.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

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
int main(void)
{
    Board_init();
    GPIO_init();

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    int ret = GetAppTask().StartAppTask();
    if (ret != 0)
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
