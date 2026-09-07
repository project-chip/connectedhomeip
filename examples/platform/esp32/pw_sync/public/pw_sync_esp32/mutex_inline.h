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
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "pw_assert/assert.h"
#include "pw_sync/mutex.h"
#include "sdkconfig.h"

namespace pw::sync {
namespace backend {

static_assert(configUSE_MUTEXES != 0, "FreeRTOS mutexes aren't enabled.");

static_assert(configSUPPORT_STATIC_ALLOCATION != 0, "FreeRTOS static allocations are required for this backend.");

} // namespace backend

inline Mutex::Mutex() : native_type_()
{
    const SemaphoreHandle_t handle = xSemaphoreCreateMutexStatic(&native_type_);
    // This should never fail since the pointer provided was not null and it
    // should return a pointer to the StaticSemaphore_t.
    PW_DASSERT(handle == reinterpret_cast<SemaphoreHandle_t>(&native_type_));
}

inline Mutex::~Mutex()
{
    vSemaphoreDelete(reinterpret_cast<SemaphoreHandle_t>(&native_type_));
}

inline void Mutex::lock()
{
    // Enforce the pw::sync::Mutex IRQ contract.
    PW_DASSERT(xPortInIsrContext() == pdFALSE);
#if INCLUDE_vTaskSuspend == 1 // This means portMAX_DELAY is indefinite.
    const BaseType_t result = xSemaphoreTake(reinterpret_cast<SemaphoreHandle_t>(&native_type_), portMAX_DELAY);
    PW_DASSERT(result == pdTRUE);
#else
    // In case we need to block for longer than the FreeRTOS delay can represent
    // repeatedly hit take until success.
    while (xSemaphoreTake(reinterpret_cast<SemaphoreHandle_t>(&native_type_), portMAX_DELAY) == pdFALSE)
    {
    }
#endif // INCLUDE_vTaskSuspend
}

inline bool Mutex::try_lock()
{
    // Enforce the pw::sync::Mutex IRQ contract.
    PW_DASSERT(xPortInIsrContext() == pdFALSE);
    return xSemaphoreTake(reinterpret_cast<SemaphoreHandle_t>(&native_type_), 0) == pdTRUE;
}

inline void Mutex::unlock()
{
    // Enforce the pw::sync::Mutex IRQ contract.
    PW_DASSERT(xPortInIsrContext() == pdFALSE);
    // Unlocking only fails if it was not locked first.
    PW_ASSERT(xSemaphoreGive(reinterpret_cast<SemaphoreHandle_t>(&native_type_)) == pdTRUE);
}

inline Mutex::native_handle_type Mutex::native_handle()
{
    return native_type_;
}

} // namespace pw::sync
