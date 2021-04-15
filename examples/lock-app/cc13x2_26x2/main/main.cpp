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
#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/SHA2.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

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
    int ret = CHIP_ERROR_MAX;

    Board_init();

    GPIO_init();

    NVS_init();

    UART_init();

    ECJPAKE_init();

    AESECB_init();

    SHA2_init();

    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
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
