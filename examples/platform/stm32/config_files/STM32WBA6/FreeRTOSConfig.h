/* USER CODE BEGIN Header */
/*
 *
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
/* USER CODE END Header */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */

/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__GNUC__)
#include "stm32wba65xx.h"
#include <stdint.h>
extern uint32_t SystemCoreClock;
/* USER CODE BEGIN 0 */
// extern void configureTimerForRunTimeStats(void);
// extern unsigned long getRunTimeCounterValue(void);
// void Error_Handler(void);
/* USER CODE END 0 */
#endif
#ifndef CMSIS_device_header
#define CMSIS_device_header "stm32wbaxx.h"
#endif /* CMSIS_device_header */

/*-------------------- STM32WBA specific defines -------------------*/
#define configENABLE_TRUSTZONE 0
#define configRUN_FREERTOS_SECURE_ONLY 0
#define configENABLE_FPU 0
#define configENABLE_MPU 0

#define configUSE_PREEMPTION 1
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configTICK_RATE_HZ ((TickType_t) 100)
#define configMAX_PRIORITIES (56)
#define configMINIMAL_STACK_SIZE ((uint16_t) 128)
#define configTOTAL_HEAP_SIZE ((size_t) 60 * 1024)
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0
#define configMAX_TASK_NAME_LEN (32)
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 0
#define configUSE_16_BIT_TICKS 0
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE 2
#define configUSE_TASK_NOTIFICATIONS 1
#define configRECORD_STACK_HIGH_ADDRESS 0
#define configHEAP_CLEAR_MEMORY_ON_FREE 0
#define configUSE_MINI_LIST_ITEM 0
#define configUSE_SB_COMPLETED_CALLBACK 0
/* USER CODE BEGIN MESSAGE_BUFFER_LENGTH_TYPE */
/* Defaults to size_t for backward compatibility, but can be changed
   if lengths will always be less than the number of bytes in a size_t. */
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t
/* USER CODE END MESSAGE_BUFFER_LENGTH_TYPE */

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (2)
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH 256

/* CMSIS-RTOS V2 flags */
#define configUSE_OS2_THREAD_SUSPEND_RESUME 1
#define configUSE_OS2_THREAD_ENUMERATE 1
#define configUSE_OS2_EVENTFLAGS_FROM_ISR 1
#define configUSE_OS2_THREAD_FLAGS 1
#define configUSE_OS2_TIMER 1
#define configUSE_OS2_MUTEX 1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_xQueueGetMutexHolder 1
// #define INCLUDE_xSemaphoreGetMutexHolder     1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_eTaskGetState 1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
/* USER CODE BEGIN 1 */
#define configASSERT(x)                                                                                                            \
    if ((x) == 0)                                                                                                                  \
    {                                                                                                                              \
        taskDISABLE_INTERRUPTS();                                                                                                  \
        for (;;)                                                                                                                   \
            ;                                                                                                                      \
    }
/* USER CODE END 1 */

#define SysTick_Handler xPortSysTickHandler

/* USER CODE BEGIN 2 */
/* Definitions needed when configGENERATE_RUN_TIME_STATS is on */
// #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS configureTimerForRunTimeStats
// #define portGET_RUN_TIME_COUNTER_VALUE getRunTimeCounterValue
/* USER CODE END 2 */

#define USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 0
#define configSYSTICK_CLOCK_HZ (32768)

#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 3
/* USER CODE END Defines */

#endif /* FREERTOS_CONFIG_H */
