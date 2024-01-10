/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --------------------------------------------------------------------------
 *
 * $Revision:   V10.2.0
 *
 * Project:     CMSIS-FreeRTOS
 * Title:       FreeRTOS configuration definitions
 *
 * --------------------------------------------------------------------------*/

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
* See http://www.freertos.org/a00110.html
*----------------------------------------------------------*/

#if !defined(__IAR_SYSTEMS_ASM__)
#if (defined(__ARMCC_VERSION) || defined(__GNUC__) || defined(__ICCARM__))
#include <stdint.h>

#include "RTE_Components.h"
#include CMSIS_device_header
#endif

#include "em_assert.h"
#include "em_device.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#endif

#if defined(SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT)
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

//  <o>Minimal stack size [words] <0-65535>
//  <i> Stack for idle task and default task stack in words.
//  <i> Default: 160
#define configMINIMAL_STACK_SIZE                160

//  <o>Total heap size [bytes] <0-0xFFFFFFFF>
//  <i> Heap memory size in bytes.
//  <i> Default: 8192
#define configTOTAL_HEAP_SIZE                   8192

//  <o>Kernel tick frequency [Hz] <0-0xFFFFFFFF>
//  <i> Kernel tick rate in Hz.
//  <i> Default: 1000
#define configTICK_RATE_HZ                      1000

//  <o>Timer task stack depth [words] <0-65535>
//  <i> Stack for timer task in words.
//  <i> Default: 160
#define configTIMER_TASK_STACK_DEPTH            160

//  <o>Timer task priority <0-56>
//  <i> Timer task priority.
//  <i> Default: 40 (High)
#define configTIMER_TASK_PRIORITY               40

//  <o>Timer queue length <0-1024>
//  <i> Timer command queue length.
//  <i> Default: 10
#define configTIMER_QUEUE_LENGTH                10

//  <o>Preemption interrupt priority
//  <i> Maximum priority of interrupts that are safe to call FreeRTOS API.
//  <i> Default: 48
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    48

//  <q>Use time slicing
//  <i> Enable setting to use timeslicing.
//  <i> Default: 1
#define configUSE_TIME_SLICING                  1

//  <q>Idle should yield
//  <i> Control Yield behaviour of the idle task.
//  <i> Default: 1
#define configIDLE_SHOULD_YIELD                 1

//  <o>Check for stack overflow
//    <0=>Disable <1=>Method one <2=>Method two
//  <i> Enable or disable stack overflow checking.
//  <i> Callback function vApplicationStackOverflowHook implementation is required when stack checking is enabled.
//  <i> Default: 2
#define configCHECK_FOR_STACK_OVERFLOW          2

//  <q>Use idle hook
//  <i> Enable callback function call on each idle task iteration.
//  <i> Callback function vApplicationIdleHook implementation is required when idle hook is enabled.
//  <i> Default: 0
#define configUSE_IDLE_HOOK                     0

//  <q>Use tick hook
//  <i> Enable callback function call during each tick interrupt.
//  <i> Callback function vApplicationTickHook implementation is required when tick hook is enabled.
//  <i> Default: 0
#define configUSE_TICK_HOOK                     0

//  <q>Use deamon task startup hook
//  <i> Enable callback function call when timer service starts.
//  <i> Callback function vApplicationDaemonTaskStartupHook implementation is required when deamon task startup hook is enabled.
//  <i> Default: 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

//  <q>Use malloc failed hook
//  <i> Enable callback function call when out of dynamic memory.
//  <i> Callback function vApplicationMallocFailedHook implementation is required when malloc failed hook is enabled.
//  <i> Default: 0
#define configUSE_MALLOC_FAILED_HOOK            0

//  <o>Queue registry size
//  <i> Define maximum number of queue objects registered for debug purposes.
//  <i> The queue registry is used by kernel aware debuggers to locate queue and semaphore structures and display associated text names.
//  <i> Default: 10
#define configQUEUE_REGISTRY_SIZE               10

// <h> Port Specific Features
// <i> Enable and configure port specific features.
// <i> Check FreeRTOS documentation for definitions that apply for the used port.

//  <q>Use Floating Point Unit
//  <i> Using Floating Point Unit (FPU) affects context handling.
//  <i> Enable FPU when application uses floating point operations.
//  <i> Default: 1
#define configENABLE_FPU                        1

//  <q>Use Memory Protection Unit
//  <i> Using Memory Protection Unit (MPU) requires detailed memory map definition.
//  <i> This setting is only releavant for MPU enabled ports.
//  <i> Default: 0
#define configENABLE_MPU                        0

//  <o>Minimal secure stack size [words] <0-65535>
//  <i> Stack for idle task Secure side context in words.
//  <i> This setting is only relevant when TrustZone extension is enabled.
//  <i> Default: 128
#define configMINIMAL_SECURE_STACK_SIZE         128
// </h>

// <h> Thread Local Storage Settings
//  <o>Thread local storage pointers
//  <i> Thread local storage (or TLS) allows the application writer to store
//  <i> values inside a task's control block, making the value specific to
//  <i> (local to) the task itself.
//  <i> Default: 0
#define configNUM_USER_THREAD_LOCAL_STORAGE_POINTERS 0
// </h>

//  <q> Use Threadsafe Errno
//  <i> Enable Threadsafe Errno support.
//  <i> Default: 0
#define configUSE_POSIX_ERRNO                 0

//------------- <<< end of configuration section >>> ---------------------------

/* Defines needed by FreeRTOS to implement CMSIS RTOS2 API. Do not change! */
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configUSE_PREEMPTION                    1
#define configUSE_TIMERS                        1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES                    56
#define configKERNEL_INTERRUPT_PRIORITY         255

/* Defines that include FreeRTOS functions which implement CMSIS RTOS2 API. Do not change! */
#define INCLUDE_xEventGroupSetBitsFromISR       1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTimerPendFunctionCall          1

/* Map the FreeRTOS port interrupt handlers to their CMSIS standard names. */
#define xPortPendSVHandler                      PendSV_Handler
#define vPortSVCHandler                         SVC_Handler

/* Ensure Cortex-M port compatibility. */
#define SysTick_Handler                         xPortSysTickHandler

/* Implement FreeRTOS configASSERT as emlib assert. */
#define configASSERT(x)                               EFM_ASSERT(x)

/* Energy saving modes. */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#define configUSE_TICKLESS_IDLE                       1
#else
#define configUSE_TICKLESS_IDLE                       0
#endif

/* Definition used by Keil to replace default system clock source. */
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION     1

/* Maximum size of task name. */
#define configMAX_TASK_NAME_LEN                       10

/* Use queue sets? */
#define configUSE_QUEUE_SETS                          0

/* Generate run-time statistics? */
#define configGENERATE_RUN_TIME_STATS                 0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                         0
#define configMAX_CO_ROUTINE_PRIORITIES               1

/* Optional resume from ISR functionality. */
#define INCLUDE_xResumeFromISR                        1

/* FreeRTOS Secure Side Only and TrustZone Security Extension */
#define configRUN_FREERTOS_SECURE_ONLY                1
#define configENABLE_TRUSTZONE                        0

/* Thread local storage pointers used by the SDK */
#ifndef configNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS
  #define configNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS 0
#endif

/* PRINT_STRING implementation. iostream_retarget_stdio or third party
   printf should be added if this is used */
#define configPRINT_STRING(X)                       printf(X)

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS (configNUM_USER_THREAD_LOCAL_STORAGE_POINTERS \
                                                 + configNUM_SDK_THREAD_LOCAL_STORAGE_POINTERS)

//#if defined(SL_CATALOG_SYSTEMVIEW_TRACE_PRESENT)
//#include "SEGGER_SYSVIEW_FreeRTOS.h"
//#endif
#endif /* FREERTOS_CONFIG_H */
