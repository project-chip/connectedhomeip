/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. If you wish to use our
 * Amazon FreeRTOS name, please do so in a fair use way that does not cause
 * confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
    See http://www.freertos.org/a00110.html for an explanation of the
    definitions contained in this file.
******************************************************************************/

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
 * http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Constants related to the behaviour or the scheduler. */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configTICK_RATE_HZ ((TickType_t) 100000)
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 0
#define configMAX_PRIORITIES (10UL)
#define configIDLE_SHOULD_YIELD 0
#define configUSE_16_BIT_TICKS 0 /* Only for 8 and 16-bit hardware. */

/* Constants that describe the hardware and memory usage. */
#define configCPU_CLOCK_HZ ((unsigned long) 48000000)
#define configMINIMAL_STACK_SIZE ((unsigned short) 512)
#define configMAX_TASK_NAME_LEN (12)

/* FreeRTOS heap size is 0 because currently "bget" heap is the
 only heap available, for both FreeRTOS and application */
//#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 0 ) )
#define configTOTAL_HEAP_SIZE ((size_t)(0))
#define configSUPPORT_STATIC_ALLOCATION 1
#define configAPPLICATION_ALLOCATED_HEAP 1

/* Default stack size for TI-POSIX threads (in words) */
#define configPOSIX_STACK_SIZE ((unsigned short) 1024)

/* Constants that build features in or out. */
#define configUSE_MUTEXES 1
#define configUSE_TICKLESS_IDLE 1
#define configUSE_APPLICATION_TASK_TAG 1 /* Need by POSIX/pthread */
#define configUSE_CO_ROUTINES 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_QUEUE_SETS 0
#define configUSE_TASK_NOTIFICATIONS 1

/* Constants that define which hook (callback) functions should be used. */
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configUSE_MALLOC_FAILED_HOOK 0

/* Constants provided for debugging and optimisation assistance. */
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configASSERT(x)                                                                                                            \
    if ((x) == 0)                                                                                                                  \
    {                                                                                                                              \
        taskDISABLE_INTERRUPTS();                                                                                                  \
        for (;;)                                                                                                                   \
            ;                                                                                                                      \
    }
#define configQUEUE_REGISTRY_SIZE 0

/* Minimum FreeRTOS tick periods of idle before invoking Power policy */
/* TODO: find way to reduce this; FreeRTOS requires it to be 2 or more */
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 2

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (6)
#define configTIMER_QUEUE_LENGTH (20)
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE)
#define configIDLE_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE)

#define configENABLE_BACKWARD_COMPATIBILITY 1

#if defined(__TI_COMPILER_VERSION__) || defined(__ti_version__)
#include <ti/posix/freertos/PTLS.h>
#define traceTASK_DELETE(pxTCB) PTLS_taskDeleteHook(pxTCB)
#elif defined(__IAR_SYSTEMS_ICC__)
#ifndef __IAR_SYSTEMS_ASM__
#include <ti/posix/freertos/Mtx.h>
#define traceTASK_DELETE(pxTCB) Mtx_taskDeleteHook(pxTCB)
#endif
#endif

/*
 *  Enable thread local storage
 *
 *  Assign TLS array index ownership here to avoid collisions.
 *  TLS storage is needed to implement thread-safe errno with
 *  TI and IAR compilers. With GNU compiler, we enable newlib.
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__ti_version__) || defined(__IAR_SYSTEMS_ICC__)

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 2

#if defined(__TI_COMPILER_VERSION__) || defined(__ti_version__)
#define PTLS_TLS_INDEX 0 /* ti.posix.freertos.PTLS */
#elif defined(__IAR_SYSTEMS_ICC__)
#define MTX_TLS_INDEX 0 /* ti.posix.freertos.Mtx */
#endif

#define NDK_TLS_INDEX 1 /* Reserve an index for NDK TLS */

#elif defined(__GNUC__)

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

#define NDK_TLS_INDEX 0 /* Reserve an index for NDK TLS */

/* note: system locks required by newlib are not implemented */
#define configUSE_NEWLIB_REENTRANT 1
#endif

/*
 * Set the following definitions to 1 to include the API function, or zero
 * to exclude the API function.  NOTE:  Setting an INCLUDE_ parameter to 0 is
 * only necessary if the linker does not automatically remove functions that
 * are not referenced anyway.
 */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle 0
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTaskResumeFromISR 0
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xSemaphoreGetMutexHolder 0
#define INCLUDE_xTimerPendFunctionCall 0

/* Cortex-M3/4 interrupt priority configuration follows...................... */

/* Use the system definition, if there is one. */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 3 /* 8 priority levels */
#endif

/*
 * The lowest interrupt priority that can be used in a call to a "set priority"
 * function.
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x07

/*
 * The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT
 * CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A
 * HIGHER PRIORITY THAN THIS! (higher priorities are lower numeric values.
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

/*
 * Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions.
 */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/*
 * !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html.
 *
 * Priority 1 (shifted 5 since only the top 3 bits are implemented).
 * Priority 1 is the second highest priority.
 * Priority 0 is the highest priority.
 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/*
 * The trace facility is turned on to make some functions available for use in
 * CLI commands.
 */
#define configUSE_TRACE_FACILITY 1

/*
 * Runtime Object View is a Texas Instrument host tool that helps visualize
 * the application. When enabled, the ISR stack will be initialized in the
 * startup_<device>_<compiler>.c file to 0xa5a5a5a5. The stack peak can then
 * be displayed in Runtime Object View.
 */
#define configENABLE_ISR_STACK_INIT 1

#endif /* FREERTOS_CONFIG_H */
