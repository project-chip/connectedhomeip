/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include "SoftwareFaultReports.h"
#include "FreeRTOSConfig.h"
#include "silabs_utils.h"
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/util/attribute-storage.h>
#include <cmsis_os2.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

#if !defined(SLI_SI91X_MCU_INTERFACE) || !defined(SLI_SI91X_ENABLE_BLE)
#include "rail_types.h"

#ifdef RAIL_ASSERT_DEBUG_STRING
#include "rail_assert_error_codes.h"
#endif
#endif // !defined(SLI_SI91X_MCU_INTERFACE) || !defined(SLI_SI91X_ENABLE_BLE)

#if defined(SLI_SI91X_MCU_INTERFACE) && SLI_SI91X_MCU_INTERFACE

#include "core_cm4.h"
#endif // defined(SLI_SI91X_MCU_INTERFACE) && SLI_SI91X_MCU_INTERFACE

// Technically FaultRecording is an octstr up to 1024 bytes.
// We currently only report short strings. 100 char will more than enough for now.
constexpr uint8_t kMaxFaultStringLen = 100;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;
using namespace chip::DeviceLayer;

namespace chip {
namespace DeviceLayer {
namespace Silabs {

void OnSoftwareFaultEventHandler(const char * faultRecordString)
{
#ifdef MATTER_DM_PLUGIN_SOFTWARE_DIAGNOSTICS_SERVER
    EnabledEndpointsWithServerCluster enabledEndpoints(SoftwareDiagnostics::Id);
    VerifyOrReturn(enabledEndpoints.begin() != enabledEndpoints.end());

    TaskStatus_t taskDetails;
    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();
    vTaskGetInfo(taskHandle, &taskDetails, pdFALSE, eInvalid);

    char threadName[kMaxThreadNameLength + 1];
    Platform::CopyString(threadName, taskDetails.pcTaskName);

    SoftwareDiagnostics::Events::SoftwareFault::Type softwareFault;
    softwareFault.name.SetValue(CharSpan::fromCharString(threadName));
    softwareFault.id = taskDetails.xTaskNumber;
    softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(faultRecordString), strlen(faultRecordString)));

    SystemLayer().ScheduleLambda([&softwareFault] { SoftwareDiagnosticsServer::Instance().OnSoftwareFaultDetect(softwareFault); });
    // Allow some time for the Fault event to be sent as the next action after exiting this function
    // is typically an assert or reboot.
    // Depending on the task at fault, it is possible the event can't be transmitted.
    osDelay(pdMS_TO_TICKS(1000));
#endif // MATTER_DM_PLUGIN_SOFTWARE_DIAGNOSTICS_SERVER
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip

#if HARD_FAULT_LOG_ENABLE
/**
 * Log register contents to UART when a hard fault occurs.
 */
extern "C" __attribute__((used)) void debugHardfault(uint32_t * sp)
{
#if SILABS_LOG_ENABLED
    [[maybe_unused]] uint32_t cfsr  = SCB->CFSR;
    [[maybe_unused]] uint32_t hfsr  = SCB->HFSR;
    [[maybe_unused]] uint32_t mmfar = SCB->MMFAR;
    [[maybe_unused]] uint32_t bfar  = SCB->BFAR;
    [[maybe_unused]] uint32_t r0    = sp[0];
    [[maybe_unused]] uint32_t r1    = sp[1];
    [[maybe_unused]] uint32_t r2    = sp[2];
    [[maybe_unused]] uint32_t r3    = sp[3];
    [[maybe_unused]] uint32_t r12   = sp[4];
    [[maybe_unused]] uint32_t lr    = sp[5];
    [[maybe_unused]] uint32_t pc    = sp[6];
    [[maybe_unused]] uint32_t psr   = sp[7];

    ChipLogError(NotSpecified, "HardFault:");
    ChipLogError(NotSpecified, "SCB->CFSR   0x%08lx", cfsr);
    ChipLogError(NotSpecified, "SCB->HFSR   0x%08lx", hfsr);
    ChipLogError(NotSpecified, "SCB->MMFAR  0x%08lx", mmfar);
    ChipLogError(NotSpecified, "SCB->BFAR   0x%08lx", bfar);
    ChipLogError(NotSpecified, "SCB->BFAR   0x%08lx", bfar);
    ChipLogError(NotSpecified, "SP          0x%08lx", (uint32_t) sp);
    ChipLogError(NotSpecified, "R0          0x%08lx", r0);
    ChipLogError(NotSpecified, "R1          0x%08lx", r1);
    ChipLogError(NotSpecified, "R2          0x%08lx", r2);
    ChipLogError(NotSpecified, "R3          0x%08lx", r3);
    ChipLogError(NotSpecified, "R12         0x%08lx", r12);
    ChipLogError(NotSpecified, "LR          0x%08lx", lr);
    ChipLogError(NotSpecified, "PC          0x%08lx", pc);
    ChipLogError(NotSpecified, "PSR         0x%08lx", psr);
#endif // SILABS_LOG_ENABLED

    configASSERTNULL(NULL);
}

/**
 * Override default hard-fault handler
 */
extern "C" __attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile("tst lr, #4                                    \n"
                   "ite eq                                        \n"
                   "mrseq r0, msp                                 \n"
                   "mrsne r0, psp                                 \n"
                   "ldr r1, debugHardfault_address                \n"
                   "bx r1                                         \n"
                   "debugHardfault_address: .word debugHardfault  \n");
}

extern "C" void vApplicationMallocFailedHook(void)
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    const char * faultMessage = "Failed to allocate memory on HEAP.";
#if SILABS_LOG_ENABLED
    ChipLogError(NotSpecified, "%s", faultMessage);
#endif
    Silabs::OnSoftwareFaultEventHandler(faultMessage);

    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}
/*-----------------------------------------------------------*/

extern "C" void vApplicationStackOverflowHook(TaskHandle_t pxTask, char * pcTaskName)
{
    (void) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    char faultMessage[kMaxFaultStringLen] = { 0 };
    snprintf(faultMessage, sizeof faultMessage, "%s Task overflowed", pcTaskName);
#if SILABS_LOG_ENABLED
    ChipLogError(NotSpecified, "%s", faultMessage);
#endif
    Silabs::OnSoftwareFaultEventHandler(faultMessage);

    /* Force an assert. */
    configASSERT((volatile void *) NULL);
}

extern "C" void vApplicationTickHook(void) {}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
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
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer,
                                               uint32_t * pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#if !defined(SLI_SI91X_MCU_INTERFACE) || !defined(SLI_SI91X_ENABLE_BLE)
extern "C" void RAILCb_AssertFailed(RAIL_Handle_t railHandle, uint32_t errorCode)
{
    char faultMessage[kMaxFaultStringLen] = { 0 };
    snprintf(faultMessage, sizeof faultMessage, "RAIL Assert:%ld", errorCode);
#if SILABS_LOG_ENABLED
#ifdef RAIL_ASSERT_DEBUG_STRING
    static const char * railErrorMessages[] = RAIL_ASSERT_ERROR_MESSAGES;
    const char * errorMessage               = "Unknown";

    // If this error code is within the range of known error messages then use the appropriate error message.
    if (errorCode < (sizeof(railErrorMessages) / sizeof(char *)))
    {
        errorMessage = railErrorMessages[errorCode];
    }
    ChipLogError(NotSpecified, "%s - %s", faultMessage, errorMessage);
#else
    ChipLogError(NotSpecified, "%s", faultMessage);
#endif // RAIL_ASSERT_DEBUG_STRING
#endif // SILABS_LOG_ENABLED
    Silabs::OnSoftwareFaultEventHandler(faultMessage);

    chipAbort();
}
#endif // !defined(SLI_SI91X_MCU_INTERFACE) || !defined(SLI_SI91X_ENABLE_BLE)
#endif // HARD_FAULT_LOG_ENABLE
