/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>

#define configSUPPORT_STATIC_ALLOCATION 1
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ ((uint32_t) (1 * 1000 * 1000))
#define configTICK_RATE_HZ ((TickType_t) 1000)
#define configMAX_PRIORITIES (32)
#define configMINIMAL_STACK_SIZE ((unsigned short) 512)
#define configTOTAL_HEAP_SIZE ((size_t) (100 * 1024))
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
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
// #define configUSE_TICKLESS_IDLE                  0
#define configUSE_POSIX_ERRNO 1
#define portasmHAS_F_EXTENSION 1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 4
#define configTIMER_TASK_STACK_DEPTH (1024)

#ifndef uartPRIMARY_PRIORITY
#define uartPRIMARY_PRIORITY (configMAX_PRIORITIES - 3)
#endif

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
#define INCLUDE_xTaskGetIdleTaskHandle 1

#if defined(__cplusplus)
extern "C" void vApplicationMallocFailedHook(void);
extern "C" void vAssertCalled(void);
#else
void vApplicationMallocFailedHook(void);
void vAssertCalled(void);
#endif

#define configASSERT(x)                                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((x) == 0)                                                                                                              \
        {                                                                                                                          \
            printf("file [%s]\r\n", __FILE__);                                                                                     \
            printf("func [%s]\r\n", __FUNCTION__);                                                                                 \
            printf("line [%d]\r\n", __LINE__);                                                                                     \
            printf("%s\r\n", (const char *) (#x));                                                                                 \
            vAssertCalled();                                                                                                       \
        }                                                                                                                          \
    } while (0)

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
#include "portmacro.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" void tickless_debug_who_wake_me(const char * name, TickType_t ticks);
#else
extern void tickless_debug_who_wake_me(const char * name, TickType_t ticks);
#endif
#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING(x)                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        TCB_t * next_wake_tcb = (TCB_t *) ((uint32_t) pxDelayedTaskList->xListEnd.pxNext - 4);                                     \
        tickless_debug_who_wake_me(next_wake_tcb->pcTaskName, next_wake_tcb->xStateListItem.xItemValue);                           \
    } while (0);
#endif
#endif

#endif /* FREERTOS_CONFIG_H */
