/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppConfig.h"
#include <AppTask.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

#include <blog.h>
extern "C" {
#include <bl702_glb.h>
#include <bl702_hbn.h>
#include <bl_irq.h>
#include <bl_psram.h>
#include <bl_rtc.h>
#include <bl_sec.h>
#include <bl_sys.h>
#include <bl_timer.h>
#include <hal_board.h>
#include <hal_boot2.h>

#include "board.h"
}

#include <hosal_uart.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#define UNUSED_PARAMETER(a) (a = a)

HOSAL_UART_DEV_DECL(uart_stdio, CHIP_UART_PORT, CHIP_UART_PIN_TX, CHIP_UART_PIN_RX, CHIP_UART_BAUDRATE);

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    ChipLogProgress(NotSpecified, "!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (true)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" unsigned int sleep(unsigned int seconds)
{
    const TickType_t xDelay = 1000 * seconds / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    return 0;
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    ChipLogProgress(NotSpecified, "Stack Overflow checked. Stack name %s", pcTaskName);
    while (true)
    {
        /*empty here*/
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    ChipLogProgress(NotSpecified, "Memory Allocate Failed. Current left size is %d bytes", xPortGetFreeHeapSize());
    while (true)
    {
        /*empty here*/
    }
}

extern "C" void vApplicationIdleHook(void)
{
    //    bl_wdt_feed();
    __asm volatile("   wfi     ");
}

extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                              uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                               uint32_t * pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configMINIMAL_STACK_SIZE * 3 / 2];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE * 3 / 2;
}

#if (configUSE_TICK_HOOK != 0)
extern "C" void vApplicationTickHook(void) {}
#endif

extern "C" void vApplicationSleep(TickType_t xExpectedIdleTime) {}

extern "C" void user_vAssertCalled(void) __attribute__((weak, alias("vAssertCalled")));
extern "C" void vAssertCalled(void)
{
    void * ra = (void *) __builtin_return_address(0);

    taskDISABLE_INTERRUPTS();
    ChipLogProgress(NotSpecified, "vAssertCalled, ra= %p", ra);
    while (true)
        ;
}

// ================================================================================
// Main Code
// ================================================================================
extern "C" uint8_t _heap_start;
extern "C" size_t _heap_size; // @suppress("Type cannot be resolved")

extern "C" uint32_t __psram_bss_init_start;
extern "C" uint32_t __psram_bss_init_end;

static uint32_t __attribute__((section(".rsvd_data"))) psram_reset_count;

static constexpr HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (size_t) &_heap_size }, // set on runtime
    { NULL, 0 }                             /* Terminates the array. */
};

extern "C" void setup_heap()
{
    bl_sys_em_config();
    vPortDefineHeapRegions(xHeapRegions);
}

extern "C" size_t get_heap_size(void)
{
    return (size_t) &_heap_size;
}

#ifdef CFG_USE_PSRAM
extern "C" uint8_t _heap3_start;
extern "C" size_t _heap3_size; // @suppress("Type cannot be resolved")
static constexpr HeapRegion_t xPsramHeapRegions[] = {
    { &_heap3_start, (size_t) &_heap3_size }, { NULL, 0 } /* Terminates the array. */
};

extern "C" size_t get_heap3_size(void)
{
    return (size_t) &_heap3_size;
}

extern "C" void do_psram_test()
{
    static constexpr char teststr[] = "bouffalolab psram test string";

    do
    {

        uint8_t * pheap = &_heap3_start;
        size_t size     = (size_t) &_heap3_size;

        size = size > sizeof(teststr) ? sizeof(teststr) : size;
        memset(pheap, 0, size);
        if (pheap[0] != 0)
        {
            break;
        }

        memcpy(pheap, teststr, size);
        if (0 != memcmp(pheap, teststr, size))
        {
            break;
        }

        arch_memset4(&__psram_bss_init_start, 0, &__psram_bss_init_end - &__psram_bss_init_start);

        psram_reset_count = 0xffffff00;
        return;
    } while (0);

    if ((psram_reset_count & 0xffffff00) != 0xffffff00)
    {
        psram_reset_count = 0xffffff00;
    }

    if ((psram_reset_count & 0x000000ff) > 3)
    {
        ChipLogError(NotSpecified, "PSRAM is still failed to initialize after %ld system reset", psram_reset_count & 0x000000ff);
        vAssertCalled();
    }

    psram_reset_count = ((psram_reset_count & 0x000000ff) + 1) | 0xffffff00;
    bl_sys_reset_system();
}
#endif

extern "C" void bl702_init(void)
{
    bl_sys_early_init();

    hosal_uart_init(&uart_stdio);

    ChipLogProgress(NotSpecified, "==================================================");
    ChipLogProgress(NotSpecified, "chip-bl702-lighting-example, built at " __DATE__ " " __TIME__);
    ChipLogProgress(NotSpecified, "==================================================");

    blog_init();
    bl_irq_init();
    bl_sec_init();
    bl_timer_init();
#ifdef CFG_USE_PSRAM
    bl_psram_init();
    do_psram_test();
#endif

    // bl_rtc_init();
    hal_boot2_init();

    /* board config is set after system is init*/
    hal_board_cfg(0);
    hosal_dma_init();

#ifdef CFG_USE_PSRAM
    vPortDefineHeapRegionsPsram(xPsramHeapRegions);
#endif

    ChipLogProgress(NotSpecified,
                    "Heap %u@%p"
#if defined(CFG_USE_PSRAM)
                    ", %u@%p"
#endif /*CFG_USE_PSRAM*/
                    ,
                    (unsigned int) &_heap_size, &_heap_start
#if defined(CFG_USE_PSRAM)
                    ,
                    (unsigned int) &_heap3_size, &_heap3_start
#endif /*CFG_USE_PSRAM*/
    );
}

extern "C" void bl702_main(void)
{
    bl702_init();

    ChipLogProgress(NotSpecified, "Init CHIP Memory");
    chip::Platform::MemoryInit(NULL, 0);

    ChipLogProgress(NotSpecified, "Starting App Task");
    StartAppTask();

    ChipLogProgress(NotSpecified, "Starting OS Scheduler...");
    vTaskStartScheduler();

    // Should never get here.
    ChipLogError(NotSpecified, "Starting OS Scheduler failed");
    appError(0);
}
