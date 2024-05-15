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

#define WIFI_DEV_NAME "MediaTek-Light"

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
    while (1)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

/***************************************************************************
 * FORWARD DECLARATIONS
 ****************************************************************************/

extern "C" void system_init(void);

void vStartTask(void * pvParameters);

/***************************************************************************
 * FreeRTOS callback functions
 ****************************************************************************/

#if (configUSE_DAEMON_TASK_STARTUP_HOOK == 1)

extern "C" void vApplicationDaemonTaskStartupHook(void)
{
#define START_TASK_STACK_SIZE (1000)
    BaseType_t xReturned;

    xReturned = xTaskCreate(vStartTask,            /* Function that implements the task. */
                            "startTask",           /* Text name for the task. */
                            START_TASK_STACK_SIZE, /* Stack size in words, not bytes. */
                            (void *) 0,            /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,      /* Priority at which the task is created. */
                            NULL);                 /* Used to pass out the created task's handle. */

    configASSERT(xReturned == pdPASS);
}

#endif /* configUSE_DAEMON_TASK_STARTUP_HOOK */

#if (configUSE_IDLE_HOOK == 1)

/* for idle task feed wdt (DO NOT enter sleep mode)*/
extern "C" void vApplicationIdleHook(void)
{
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
    hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif
#endif
}

#endif /* configUSE_IDLE_HOOK */

#if (configSUPPORT_STATIC_ALLOCATION == 1)

extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                               uint32_t * pulTimerTaskStackSize)
{
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
    *ppxTimerTaskTCBBuffer = (StaticTask_t *) pvPortMalloc(sizeof(StaticTask_t));
    if (*ppxTimerTaskTCBBuffer != NULL)
    {
        *ppxTimerTaskStackBuffer = (StackType_t *) pvPortMalloc((((size_t) *pulTimerTaskStackSize) * sizeof(StackType_t)));
    }
}

extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                              uint32_t * pulIdleTaskStackSize)
{
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    *ppxIdleTaskTCBBuffer = (StaticTask_t *) pvPortMalloc(sizeof(StaticTask_t));
    if (*ppxIdleTaskTCBBuffer != NULL)
    {
        *ppxIdleTaskStackBuffer = (StackType_t *) pvPortMalloc((((size_t) *pulIdleTaskStackSize) * sizeof(StackType_t)));
    }
}

#endif /* configSUPPORT_STATIC_ALLOCATION */

/*******************************************************************************
 * DECLARATIONS
 ******************************************************************************/

void mt793xLog(const char * aFormat, ...)
{
    va_list vargs;

    va_start(vargs, aFormat);
    vprintf(aFormat, vargs);
    va_end(vargs);
    printf("\n");
}

void mt793xLogRedirectCallback(const char * module, uint8_t category, const char * msg, va_list args)
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

void mt793xSwdPortConfig(void)
{
    *(volatile uint32_t *) 0x30404358 = 0x00070700;
    *(volatile uint32_t *) 0x30404354 = 0x00020200;
    *(volatile uint32_t *) 0x304030e0 = 0x1e8210;
    *(volatile uint32_t *) 0x304030d4 = 0;
}

/***************************************************************************
 * Button Callback
 ****************************************************************************/

void vButtonCallback(const filogic_button_t * button_event)
{
    GetAppTask().ButtonHandler(*button_event);
}

/***************************************************************************
 * Supplicant Log Redirect
 ****************************************************************************/

extern "C" {
void mt793x_wpa_log_cb(void * ctx, int level, int type, const char * txt, size_t len)
{
    /* ignore ctx, type, len */
    (void) ctx;
    (void) level;
    (void) len;

    ChipLogProgress(DeviceLayer, "[%lu] FILOGIC supp %s", xTaskGetTickCount(), txt);
}

void wpa_msg_register_cb(void *);
}

/***************************************************************************
 * Startup task
 ****************************************************************************/

void vStartRunning(void)
{
    CHIP_ERROR error;

    chip::Logging::SetLogRedirectCallback(mt793xLogRedirectCallback);

    wpa_msg_register_cb((void *) mt793x_wpa_log_cb);

    assert(chip::Platform::MemoryInit() == CHIP_NO_ERROR);

    assert(chip::DeviceLayer::PlatformMgr().InitChipStack() == CHIP_NO_ERROR);

    // Wi-Fi ?
    // chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(BLE_DEV_NAME);

    assert(chip::DeviceLayer::PlatformMgr().StartEventLoopTask() == CHIP_NO_ERROR);

    assert(GetAppTask().StartAppTask() == CHIP_NO_ERROR);

    assert(filogic_button_set_callback(vButtonCallback));

    assert(filogic_button_init());

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
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    system_init();

    mt793xSwdPortConfig();

    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();

    // Should never get here.
    while (1)
        ;

    return 0;
}
