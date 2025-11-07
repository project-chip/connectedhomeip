// Copyright 2025 Project CHIP Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "pw_sync/interrupt_spin_lock.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "pw_assert/check.h"

namespace pw::sync {

#if (INCLUDE_xTaskGetSchedulerState != 1) && (configUSE_TIMERS != 1)
#error "xTaskGetSchedulerState is required for pw::sync::InterruptSpinLock"
#endif

// Static mutex for critical sections
static portMUX_TYPE s_interrupt_spin_lock_mux = portMUX_INITIALIZER_UNLOCKED;

// ESP-IDF specific interrupt context detection
static bool InInterruptContext()
{
    return xPortInIsrContext() != pdFALSE;
}

void InterruptSpinLock::lock()
{
    if (InInterruptContext())
    {
        portENTER_CRITICAL_ISR(&s_interrupt_spin_lock_mux);
    }
    else
    {
        // Task context
        vPortEnterCritical(&s_interrupt_spin_lock_mux);
    }
    // NOTE: This recursion check is flawed. A recursive call on the same core
    // will deadlock before this check is reached.
    // We can't deadlock here so crash instead.
    PW_DCHECK(!native_type_.locked, "Recursive InterruptSpinLock::lock() detected");
    native_type_.locked = true;
}

void InterruptSpinLock::unlock()
{
    native_type_.locked = false;
    if (InInterruptContext())
    {
        portEXIT_CRITICAL_ISR(&s_interrupt_spin_lock_mux);
    }
    else
    {
        // Task context
        vPortExitCritical(&s_interrupt_spin_lock_mux);
    }
}

} // namespace pw::sync
