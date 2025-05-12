/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <mbedtls/threading.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include <AppTask.h>

#include "AppConfig.h"
#include <app/server/Server.h>

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#include <mbedtls/platform.h>

#ifdef ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif

#define WIFI_DEV_NAME "SCM-Light"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

volatile int apperror_cnt;

/***************************************************************************
 * Application Error hang
 ****************************************************************************/

void appError(int err)
{
    printf("!!!!!!!!!!!! Application Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (true)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

/***************************************************************************
 * FORWARD DECLARATIONS
 ****************************************************************************/

void vStartTask(void * pvParameters);

/*******************************************************************************
 * DECLARATIONS
 ******************************************************************************/

void scm1612sLog(const char * aFormat, ...)
{
    va_list vargs;

    va_start(vargs, aFormat);
    vprintf(aFormat, vargs);
    va_end(vargs);
    printf("\n");
}

void scm1612sLogRedirectCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    int len;
    char * buf;

    len = strlen(module) + 1 + vsnprintf(NULL, 0, msg, args) + 1;
    buf = (char *) malloc(len);
    if (buf)
    {
        len = sprintf(buf, "%s ", module);
        vsprintf(buf + len, msg, args);
        printf("%s\n", buf);
        free(buf);
    }
}

/***************************************************************************
 * Button Callback
 ****************************************************************************/

#ifdef __no_stub__
void vButtonCallback(const filogic_button_t * button_event)
{
    GetAppTask().ButtonHandler(*button_event);
}
#endif /* __no_stub__ */

/***************************************************************************
 * Supplicant Log Redirect
 ****************************************************************************/

/***************************************************************************
 * Startup task
 ****************************************************************************/

void vStartRunning(void)
{
    CHIP_ERROR error;

    chip::Logging::SetLogRedirectCallback(scm1612sLogRedirectCallback);

    assert(chip::Platform::MemoryInit() == CHIP_NO_ERROR);

    assert(chip::DeviceLayer::PlatformMgr().InitChipStack() == CHIP_NO_ERROR);

    // Wi-Fi ?
    // chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(BLE_DEV_NAME);

    assert(chip::DeviceLayer::PlatformMgr().StartEventLoopTask() == CHIP_NO_ERROR);

    assert(GetAppTask().StartAppTask() == CHIP_NO_ERROR);

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

}

void vStartTask(void * pvParameters)
{
    (void) pvParameters;

    vStartRunning();

    vTaskDelete(NULL);
}

/***************************************************************************
 * Main Function
 ****************************************************************************/

extern "C" int main(void)
{
#define START_TASK_STACK_SIZE (1000)
    BaseType_t xReturned;

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    xReturned = xTaskCreate(vStartTask,            /* Function that implements the task. */
                            "startTask",           /* Text name for the task. */
                            START_TASK_STACK_SIZE, /* Stack size in words, not bytes. */
                            (void *) 0,            /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,      /* Priority at which the task is created. */
                            NULL);                 /* Used to pass out the created task's handle. */

    configASSERT(xReturned == pdPASS);

    return 0;
}
