/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an generic implementation of ThreadStackManager features
 *          for use on FreeRTOS platforms.
 */

#pragma once

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "timers.h"
#else
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#endif

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl;

namespace Internal {

/**
 * Provides a generic implementation of ThreadStackManager features that works on FreeRTOS platforms.
 *
 * This template contains implementations of select features from the ThreadStackManager abstract
 * interface that are suitable for use on FreeRTOS-based platforms.  It is intended to be
 * inherited, directly or indirectly, by the ThreadStackManagerImpl class, which also appears as
 * the template's ImplClass parameter.
 */
template <class ImplClass>
class GenericThreadStackManagerImpl_FreeRTOS
{

protected:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    CHIP_ERROR _StartThreadTask(void);
    void _LockThreadStack(void);
    bool _TryLockThreadStack(void);
    void _UnlockThreadStack(void);

    // ===== Members available to the implementation subclass.

    SemaphoreHandle_t mThreadStackLock;
    TaskHandle_t mThreadTask;

    CHIP_ERROR DoInit();
    void SignalThreadActivityPending();
    BaseType_t SignalThreadActivityPendingFromISR();

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void ThreadTaskMain(void * arg);

#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    StackType_t mThreadStack[CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE / sizeof(StackType_t)];
    StaticTask_t mThreadTaskStruct;
#endif

#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE) && CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE
    StaticSemaphore_t mThreadStackLockMutex;
#endif
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
