/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/***************************************************************************
 * # License
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is Third Party Software licensed by Silicon Labs from a third party
 * and is governed by the sections of the MSLA applicable to Third Party
 * Software and the additional terms set forth below.
 *
 ******************************************************************************/
/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "em_assert.h"
#include "em_device.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

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

/* Set configCREATE_LOW_POWER_DEMO as follows:
 *
 * 0: Build the full test and demo application.
 * 1: Build the simple blinky tickless low power demo, generating the tick
 *    interrupt from the RTCC.  EM2 will be entered.  The LXFO clock is used.
 *  See the comments at the top of main.c, main_full.c and main_low_power.c for
 *  more information.
 */

#define configCREATE_LOW_POWER_DEMO (0)

/* Some configuration is dependent on the demo being built. */
#if (configCREATE_LOW_POWER_DEMO == 0)

/* Tickless mode is not used. */

/* Some of the standard demo test tasks assume a tick rate of 1KHz, even
though that is faster than would normally be warranted by a real
application. */
#define configTICK_RATE_HZ (1000)

/* Energy saving modes. */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#define configUSE_TICKLESS_IDLE 1
#else
#define configUSE_TICKLESS_IDLE 0
#endif

/* Definition used by Keil to replace default system clock source. */
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION 1

/* Hook function related definitions. */
#define configUSE_TICK_HOOK (1)
#define configCHECK_FOR_STACK_OVERFLOW (2)
#define configUSE_MALLOC_FAILED_HOOK (1)
#define configUSE_IDLE_HOOK (1)

#define configENERGY_MODE (sleepEM1)

#else

/* Tickless idle mode, generating RTOS tick interrupts from the RTC, fed
by the LXFO clock. */

/* The slow clock used to generate the tick interrupt in the low power demo
runs at 32768/8=4096Hz.  Ensure the tick rate is a multiple of the clock. */
#define configTICK_RATE_HZ (128)

/* The low power demo uses the tickless idle feature. */
#define configUSE_TICKLESS_IDLE (1)
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION (1)

/* Hook function related definitions. */
#define configUSE_TICK_HOOK (0)
#define configCHECK_FOR_STACK_OVERFLOW (0)
#define configUSE_MALLOC_FAILED_HOOK (0)
#define configUSE_IDLE_HOOK (1)

#define configENERGY_MODE (sleepEM3)
#endif

/* Main functions*/
/* Run time stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS (0)

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES (0)
#define configMAX_CO_ROUTINE_PRIORITIES (1)

/* Software timer related definitions. */
#define configUSE_TIMERS (1)
#define configTIMER_TASK_PRIORITY (40) /* Highest priority */
#define configTIMER_QUEUE_LENGTH (10)
#define configTIMER_TASK_STACK_DEPTH (1024)

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (255)
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 48
#define configENABLE_FPU 0
#define configENABLE_MPU 0
/* FreeRTOS Secure Side Only and TrustZone Security Extension */
#define configRUN_FREERTOS_SECURE_ONLY 1
#define configENABLE_TRUSTZONE 0
/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS (0)

#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configUSE_PREEMPTION (1)
#define configUSE_TIME_SLICING (1)
#define configUSE_PORT_OPTIMISED_TASK_SELECTION (0)
#define configUSE_TICKLESS_IDLE_SIMPLE_DEBUG (1) /* See into vPortSuppressTicksAndSleep source code for explanation */
#define configMAX_PRIORITIES (56)
#define configMINIMAL_STACK_SIZE (140) /* Number of words to use for Idle and Timer stacks */
#define configMAX_TASK_NAME_LEN (10)
#define configUSE_16_BIT_TICKS (0)
#define configIDLE_SHOULD_YIELD (1)
#define configUSE_MUTEXES (1)
#define configUSE_RECURSIVE_MUTEXES (1)
#define configUSE_COUNTING_SEMAPHORES (1)
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_TRACE_FACILITY 1
#define configQUEUE_REGISTRY_SIZE (10)
#define configUSE_QUEUE_SETS (0)
#define configUSE_NEWLIB_REENTRANT (1)
#define configENABLE_BACKWARD_COMPATIBILITY (1)
#define configSUPPORT_STATIC_ALLOCATION (1)
#define configSUPPORT_DYNAMIC_ALLOCATION (1)
#define configTOTAL_HEAP_SIZE ((size_t)(20 * 1024))

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet (1)
#define INCLUDE_uxTaskPriorityGet (1)
#define INCLUDE_vTaskDelete (1)
#define INCLUDE_vTaskSuspend (1)
#define INCLUDE_xResumeFromISR (1)
#define INCLUDE_vTaskDelayUntil (1)
#define INCLUDE_vTaskDelay (1)
#define INCLUDE_xTaskGetSchedulerState (1)
#define INCLUDE_xTaskGetCurrentTaskHandle (1)
#define INCLUDE_uxTaskGetStackHighWaterMark (1)
#define INCLUDE_xTaskGetIdleTaskHandle (1)
#define INCLUDE_xTimerGetTimerDaemonTaskHandle (1)
#define INCLUDE_pcTaskGetTaskName (1)
#define INCLUDE_eTaskGetState (1)
#define INCLUDE_xEventGroupSetBitFromISR (1)
#define INCLUDE_xEventGroupSetBitsFromISR (1)
#define INCLUDE_xSemaphoreGetMutexHolder (1)
#define INCLUDE_xTimerPendFunctionCall (1)
#define INCLUDE_xTaskGetHandle (1)

/* Stop if an assertion fails. */
#define configASSERT(x)                                                                                                            \
    if ((x) == 0)                                                                                                                  \
    {                                                                                                                              \
        taskDISABLE_INTERRUPTS();                                                                                                  \
        printf("\nFREERTOS ASSERT ( %s )\n", #x);                                                                                  \
        for (;;)                                                                                                                   \
            ;                                                                                                                      \
    }
#define configASSERTNULL(x)                                                                                                        \
    if ((x) == NULL)                                                                                                               \
    {                                                                                                                              \
        taskDISABLE_INTERRUPTS();                                                                                                  \
        for (;;)                                                                                                                   \
            ;                                                                                                                      \
    }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
/* Ensure Cortex-M port compatibility. */
#define SysTick_Handler xPortSysTickHandler

/* Thread local storage pointers used by the SDK */
#ifndef configNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS
#define configNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS 0
#endif

#if defined(__GNUC__)
/* For the linker. */
#define fabs __builtin_fabs
#endif

#ifdef __cplusplus
}
#endif
