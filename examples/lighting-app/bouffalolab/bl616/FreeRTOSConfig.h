/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#define configMTIME_BASE_ADDRESS (0xE0000000UL + 0xBFF8UL)
#define configMTIMECMP_BASE_ADDRESS (0xE0000000UL + 0x4000UL)

#define configSUPPORT_STATIC_ALLOCATION 1
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ ((uint32_t) (1 * 1000 * 1000))
#define configTICK_RATE_HZ ((TickType_t) 1000)
#define configMAX_PRIORITIES (32)
#define configMINIMAL_STACK_SIZE                                                                                                   \
    ((unsigned short) 128) /* Only needs to be this high as some demo tasks also use this constant.  In production only the idle   \
                              task would use this. */
#define configTOTAL_HEAP_SIZE ((size_t) 100 * 1024)
#define configMAX_TASK_NAME_LEN (16)
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 0
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_MALLOC_FAILED_HOOK 1
#define configUSE_APPLICATION_TASK_TAG 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configGENERATE_RUN_TIME_STATS 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE 0
#define configUSE_POSIX_ERRNO 1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 4
#define configTIMER_TASK_STACK_DEPTH (1024)

/* Task priorities.  Allow these to be overridden. */
#ifndef uartPRIMARY_PRIORITY
#define uartPRIMARY_PRIORITY (configMAX_PRIORITIES - 3)
#endif

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_xTaskAbortDelay 1
#define INCLUDE_xTaskGetHandle 1
#define INCLUDE_xSemaphoreGetMutexHolder 1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
void vApplicationMallocFailedHook(void);

#if __cplusplus
extern "C" void vAssertCalled(void);
#else
void vAssertCalled(void);
#endif
#define configASSERT(x)                                                                                                            \
    if ((x) == 0)                                                                                                                  \
    {                                                                                                                              \
        printf("file [%s]\r\n", __FILE__);                                                                                         \
        printf("func [%s]\r\n", __FUNCTION__);                                                                                     \
        printf("line [%d]\r\n", __LINE__);                                                                                         \
        printf("%s\r\n", (const char *) (#x));                                                                                     \
        vAssertCalled();                                                                                                           \
    }

#if (configUSE_TICKLESS_IDLE != 0)
void vApplicationSleep(uint32_t xExpectedIdleTime);
#define portSUPPRESS_TICKS_AND_SLEEP(xExpectedIdleTime) vApplicationSleep(xExpectedIdleTime)
#endif

// #define portUSING_MPU_WRAPPERS

#if (configGENERATE_RUN_TIME_STATS == 1)
#ifdef __cplusplus
extern "C" uint64_t bflb_mtimer_get_time_us();
#else
extern uint64_t bflb_mtimer_get_time_us();
#endif
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ((void) 0)
#define portGET_RUN_TIME_COUNTER_VALUE() bflb_mtimer_get_time_us()
#endif

#endif /* FREERTOS_CONFIG_H */
