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
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>

#define configSUPPORT_STATIC_ALLOCATION          1
#define configUSE_PREEMPTION                     1
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configCPU_CLOCK_HZ                       ((uint32_t)(1 * 1000 * 1000))
#define configTICK_RATE_HZ                       ((TickType_t)1000)
#define configMAX_PRIORITIES                     (32)
#define configMINIMAL_STACK_SIZE                 ((unsigned short)512)
#define configTOTAL_HEAP_SIZE                    ((size_t)(100 * 1024))
#define configMAX_TASK_NAME_LEN                  (16)
#define configUSE_TRACE_FACILITY                 1
#define configUSE_STATS_FORMATTING_FUNCTIONS     1
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  0
#define configUSE_MUTEXES                        1
#define configQUEUE_REGISTRY_SIZE                8
#define configCHECK_FOR_STACK_OVERFLOW           2
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_MALLOC_FAILED_HOOK             1
#define configUSE_APPLICATION_TASK_TAG           0
#define configUSE_COUNTING_SEMAPHORES            1
#define configGENERATE_RUN_TIME_STATS            0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  1
// #define configUSE_TICKLESS_IDLE                  0
#define configUSE_POSIX_ERRNO                    1
#define portasmHAS_F_EXTENSION                   1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES            0
#define configMAX_CO_ROUTINE_PRIORITIES  (2)

/* Software timer definitions. */
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     4
#define configTIMER_TASK_STACK_DEPTH (1024)

#ifndef uartPRIMARY_PRIORITY
#define uartPRIMARY_PRIORITY (configMAX_PRIORITIES - 3)
#endif

#define INCLUDE_vTaskPrioritySet          1
#define INCLUDE_uxTaskPriorityGet         1
#define INCLUDE_vTaskDelete               1
#define INCLUDE_vTaskCleanUpResources     1
#define INCLUDE_vTaskSuspend              1
#define INCLUDE_vTaskDelayUntil           1
#define INCLUDE_vTaskDelay                1
#define INCLUDE_eTaskGetState             1
#define INCLUDE_xTimerPendFunctionCall    1
#define INCLUDE_xTaskAbortDelay           1
#define INCLUDE_xTaskGetHandle            1
#define INCLUDE_xSemaphoreGetMutexHolder  1
#define INCLUDE_xTaskGetIdleTaskHandle    1

#if defined(__cplusplus)
extern "C" void vApplicationMallocFailedHook(void);
extern "C" void vAssertCalled(void);
#else
void vApplicationMallocFailedHook(void);
void vAssertCalled(void);
#endif

#define configASSERT(x)                                  \
    if ((x) == 0)                                        \
    {                                                    \
        printf("file [%s]\r\n", __FILE__);               \
        printf("func [%s]\r\n", __FUNCTION__);           \
        printf("line [%d]\r\n", __LINE__);               \
        printf("%s\r\n", (const char *)(#x));            \
        vAssertCalled();                                 \
    }

#if (configUSE_TICKLESS_IDLE != 0)
#ifdef __cplusplus
extern "C" void vApplicationSleep(uint32_t xExpectedIdleTime);
#else
extern void vApplicationSleep(uint32_t xExpectedIdleTime);
#endif
#define portSUPPRESS_TICKS_AND_SLEEP(xExpectedIdleTime) vApplicationSleep(xExpectedIdleTime)

#ifndef configEXPECTED_IDLE_TIME_BEFORE_SLEEP
#ifdef __cplusplus
extern "C" uint32_t expected_idle_before_sleep(void);
#else
extern uint32_t expected_idle_before_sleep(void);
#endif
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP expected_idle_before_sleep()
#endif

#ifdef TICKLESS_DEBUG
#include <stdint.h>
#include "portmacro.h"
#ifdef __cplusplus
extern "C" void tickless_debug_who_wake_me(const char * name, TickType_t ticks);
#else
extern void tickless_debug_who_wake_me(const char * name, TickType_t ticks);
#endif
#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING(x)                                                       \
    do                                                                                                         \
    {                                                                                                          \
        TCB_t * next_wake_tcb = (TCB_t *) ((uint32_t) pxDelayedTaskList->xListEnd.pxNext - 4);                 \
        tickless_debug_who_wake_me(next_wake_tcb->pcTaskName, next_wake_tcb->xStateListItem.xItemValue);       \
    } while (0);
#endif
#endif

#endif /* FREERTOS_CONFIG_H */
