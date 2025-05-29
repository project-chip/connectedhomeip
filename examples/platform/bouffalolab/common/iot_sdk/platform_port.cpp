/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/CHIPDeviceConfig.h>

#include <FreeRTOS.h>

#include <blog.h>

#if CHIP_DEVICE_LAYER_TARGET_BL702
#include <bl702_glb.h>
#include <bl702_hbn.h>
#elif CHIP_DEVICE_LAYER_TARGET_BL702L
#include <bl702l_glb.h>
#include <bl702l_hbn.h>
#endif

#ifdef CFG_USE_PSRAM
#include <bl_psram.h>
#endif
#include <bl_timer.h>

extern "C" {
#include <bl_irq.h>
#include <bl_rtc.h>
#include <bl_sec.h>
#include <bl_sys.h>
#include <hal_board.h>
#include <hal_boot2.h>
#if CHIP_DEVICE_LAYER_TARGET_BL602
#include <wifi_mgmr_ext.h>
#elif CHIP_DEVICE_LAYER_TARGET_BL702L
#include <bl_flash.h>
#endif
}

#include <hosal_uart.h>

#include <uart.h>

#include "mboard.h"
#include <plat.h>

HOSAL_UART_DEV_DECL(uart_stdio, CHIP_UART_PORT, CHIP_UART_PIN_TX, CHIP_UART_PIN_RX, CHIP_UART_BAUDRATE);

#ifdef SYS_AOS_LOOP_ENABLE
extern "C" void aos_loop_start(void);
#endif

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
    printf("Stack Overflow checked. Stack name %s", pcTaskName);
    while (true)
    {
        /*empty here*/
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes", xPortGetFreeHeapSize());
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

#if (configUSE_TICK_HOOK != 0)
extern "C" void vApplicationTickHook(void)
{
#if defined(CFG_USB_CDC_ENABLE)
    extern void usb_cdc_monitor(void);
    usb_cdc_monitor();
#endif
}
#endif

#if (configUSE_TICKLESS_IDLE != 0)
extern "C" __WEAK void vApplicationSleep(TickType_t xExpectedIdleTime) {}
#endif

static StaticTask_t xIdleTaskTCB;
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                              uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = (StaticTask_t *) pvPortMalloc(sizeof(StaticTask_t));

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * configMINIMAL_STACK_SIZE);

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

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = (StaticTask_t *) pvPortMalloc(sizeof(StaticTask_t));

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * configTIMER_TASK_STACK_DEPTH);

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

extern "C" void vAssertCalled(void)
{
    void * ra = (void *) __builtin_return_address(0);

    taskDISABLE_INTERRUPTS();

    if (xPortIsInsideInterrupt())
    {
        printf("vAssertCalled, ra = %p in ISR\r\n", (void *) ra);
    }
    else
    {
        printf("vAssertCalled, ra = %p in task %s\r\n", (void *) ra, pcTaskGetName(NULL));
    }

    portABORT();

    while (true)
        ;
}

extern "C" void user_vAssertCalled(void) __attribute__((weak, alias("vAssertCalled")));
extern "C" void bflb_assert(void) __attribute__((weak, alias("vAssertCalled")));

// ================================================================================
// Main Code
// ================================================================================
extern "C" uint8_t _heap_start;
extern "C" size_t _heap_size; // @suppress("Type cannot be resolved")

#if CHIP_DEVICE_LAYER_TARGET_BL602
extern "C" uint8_t _heap_wifi_start;
extern "C" uint8_t _heap_wifi_size; // @suppress("Type cannot be resolved")
static const HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (unsigned int) &_heap_size }, // set on runtime
    { &_heap_wifi_start, (unsigned int) &_heap_wifi_size },
    { NULL, 0 } /* Terminates the array. */
};
#elif CHIP_DEVICE_LAYER_TARGET_BL702
extern "C" uint8_t _heap2_start;
extern "C" uint8_t _heap2_size; // @suppress("Type cannot be resolved")
static const HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (size_t) &_heap_size },   // set on runtime
    { &_heap2_start, (size_t) &_heap2_size }, // set on runtime
    { NULL, 0 }                               /* Terminates the array. */
};
#elif CHIP_DEVICE_LAYER_TARGET_BL702L
static const HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (size_t) &_heap_size }, // set on runtime
    { NULL, 0 }                             /* Terminates the array. */
};
#endif

#ifdef CFG_USE_PSRAM
extern "C" uint8_t __psram_bss_init_start;
extern "C" uint8_t __psram_bss_init_end;

static uint32_t __attribute__((section(".rsvd_data"))) psram_reset_count;

extern "C" uint8_t _heap3_start;
extern "C" size_t _heap3_size; // @suppress("Type cannot be resolved")
static const HeapRegion_t xPsramHeapRegions[] = {
    { &_heap3_start, (size_t) &_heap3_size }, { NULL, 0 } /* Terminates the array. */
};

extern "C" size_t get_heap3_size(void)
{
    return (size_t) &_heap3_size;
}

void do_psram_test()
{
    static const char teststr[] = "bouffalolab psram test string";

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

        memset(&__psram_bss_init_start, 0, &__psram_bss_init_end - &__psram_bss_init_start);

        psram_reset_count = 0xffffff00;
        return;
    } while (0);

    if ((psram_reset_count & 0xffffff00) != 0xffffff00)
    {
        psram_reset_count = 0xffffff00;
    }

    if ((psram_reset_count & 0x000000ff) > 3)
    {
        printf("PSRAM is still failed to initialize after %ld system reset", psram_reset_count & 0x000000ff);
        vAssertCalled();
    }

    psram_reset_count = ((psram_reset_count & 0x000000ff) + 1) | 0xffffff00;
    bl_sys_reset_system();
}
#endif

extern "C" void setup_heap()
{
    bl_sys_init();

#if CHIP_DEVICE_LAYER_TARGET_BL702 || CHIP_DEVICE_LAYER_TARGET_BL702L
    bl_sys_em_config();
#endif

    vPortDefineHeapRegions(xHeapRegions);

    bl_sys_early_init();

#ifdef CFG_USE_PSRAM
    bl_psram_init();
    do_psram_test();
    vPortDefineHeapRegionsPsram(xPsramHeapRegions);
#endif
}

extern "C" size_t get_heap_size(void)
{
    return (size_t) &_heap_size;
}

static void bl_show_component_version(void)
{
    extern uint8_t _version_info_section_start;
    extern uint8_t _version_info_section_end;
    char ** version_str_p = NULL;

    puts("Version of used components:\r\n");
    for (version_str_p = (char **) &_version_info_section_start; version_str_p < (char **) &_version_info_section_end;
         version_str_p++)
    {
        puts("\tVersion: ");
        puts(*version_str_p);
        puts("\r\n");
    }
}

extern "C" void app_init(void)
{
#if CHIP_DEVICE_LAYER_TARGET_BL702L
    bl_flash_init();
#endif

    hosal_uart_init(&uart_stdio);

    blog_init();
    bl_irq_init();
#if CHIP_DEVICE_LAYER_TARGET_BL702L
    bl_rtc_init();
#endif
    bl_sec_init();
#if CHIP_DEVICE_LAYER_TARGET_BL702
    bl_timer_init();
#endif

    hal_boot2_init();

    /* board config is set after system is init*/
    hal_board_cfg(0);

#if CHIP_DEVICE_LAYER_TARGET_BL702 && (CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET)
    hosal_dma_init();
#endif
#if CHIP_DEVICE_LAYER_TARGET_BL602
    wifi_td_diagnosis_init();
#endif

    bl_show_component_version();
}

extern "C" void platform_port_init(void)
{
    app_init();

#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED
    uartInit();
#endif

#ifdef SYS_AOS_LOOP_ENABLE
    aos_loop_start();
#else
#if defined(CFG_USB_CDC_ENABLE)
    extern void usb_cdc_start(int fd_console);
    usb_cdc_start(-1);
#endif
#endif
}
