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

#include "platform.h"

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

#define configSUPPORT_STATIC_ALLOCATION 1

#define configCLINT_BASE_ADDRESS CLINT_CTRL_ADDR
#define configCLIC_TIMER_ENABLE_ADDRESS (0x02800407)
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 1
#define configUSE_TICK_HOOK 1
#define configUSE_TICKLESS_IDLE 1
#define configCPU_CLOCK_HZ (2000000) /*QEMU*/
#define configTICK_RATE_HZ ((TickType_t) 1000)
#define configMAX_PRIORITIES (32)
/* Creating idle task */
/* -1  -> prvInitialiseNewTask: (subtract 1 to get top of stack) pxTopOfStack = &( pxNewTCB->pxStack[ ulStackDepth - ( uint32_t ) 1
 * ] ); */
/* -1  -> prvInitialiseNewTask: (subtract 1 in case not matching 8 bytes alignment) pxTopOfStack = ( StackType_t * ) ( ( (
 * portPOINTER_SIZE_TYPE ) pxTopOfStack ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) ); */
/* Running idle task */
/* -12 -> prvIdleTask: addi sp, sp, -48 */
/* -16 -> prvIdleTask.xTaskResumeAll: addi sp, sp, -64 */
/* Here comes an interrupt after prvIdleTask.xTaskResumeAll.vTaskExitCritical */
/* -30 -> freertos_risc_v_trap_handler: addi sp, sp, -portCONTEXT_SIZE */
/* -34 -> portasmSAVE_ADDITIONAL_REGISTERS: addi sp, sp, -(portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE) */
/* Checking for stack overflow */
/*  4  -> taskCHECK_FOR_STACK_OVERFLOW: if( ( pulStack[ 0 ] != ulCheckValue ) || ( pulStack[ 1 ] != ulCheckValue ) || ( pulStack[ 2
 * ] != ulCheckValue ) || ( pulStack[ 3 ] != ulCheckValue ) ) */
#define configMINIMAL_STACK_SIZE ((unsigned short) 256) /* SIZE-1-1-12-16-30-34>=4 */
#define configTOTAL_HEAP_SIZE ((size_t) 14100)
#define configMAX_TASK_NAME_LEN (16)
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_MALLOC_FAILED_HOOK 1
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 4
#define configTIMER_TASK_STACK_DEPTH (400)

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
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle 1
#define INCLUDE_xTaskGetHandle 1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#ifdef __cplusplus
extern "C" void vAssertCalled(void);
#else
extern void vAssertCalled(void);
#endif

#define configASSERT(x)                                                                                                            \
    if ((x) == 0)                                                                                                                  \
    vAssertCalled()

/* Overwrite some of the stack sizes allocated to various test and demo tasks.
Like all task stack sizes, the value is the number of words, not bytes. */
#define bktBLOCK_TIME_TASK_STACK_SIZE 100
#define notifyNOTIFIED_TASK_STACK_SIZE 120
#define priSUSPENDED_RX_TASK_STACK_SIZE 90
#define tmrTIMER_TEST_TASK_STACK_SIZE 100
#define ebRENDESVOUS_TEST_TASK_STACK_SIZE 100
#define ebEVENT_GROUP_SET_BITS_TEST_TASK_STACK_SIZE 115
#define genqMUTEX_TEST_TASK_STACK_SIZE 90
#define genqGENERIC_QUEUE_TEST_TASK_STACK_SIZE 100
#define recmuRECURSIVE_MUTEX_TEST_TASK_STACK_SIZE 90

#if (configUSE_TICKLESS_IDLE != 0)
#include "portmacro.h"
#ifdef __cplusplus
extern "C" void vApplicationSleep(TickType_t xExpectedIdleTime);
#else
extern void vApplicationSleep(TickType_t xExpectedIdleTime);
#endif
#define portSUPPRESS_TICKS_AND_SLEEP(xExpectedIdleTime) vApplicationSleep(xExpectedIdleTime)
#endif

#endif /* FREERTOS_CONFIG_H */
