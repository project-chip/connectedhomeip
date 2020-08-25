/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an generic implementation of PlatformManager features
 *          for use on FreeRTOS platforms.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H
#define GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H

#include <platform/internal/GenericPlatformManagerImpl.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#else
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on FreeRTOS platforms.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on FreeRTOS-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_FreeRTOS : public GenericPlatformManagerImpl<ImplClass>
{
protected:
    TimeOut_t mNextTimerBaseTime;
    TickType_t mNextTimerDurationTicks;
    SemaphoreHandle_t mChipStackLock;
    QueueHandle_t mChipEventQueue;
    TaskHandle_t mEventLoopTask;
    bool mChipTimerActive;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    void _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StartChipTimer(uint32_t durationMS);
    CHIP_ERROR _Shutdown(void);

    // ===== Methods available to the implementation subclass.

    void PostEventFromISR(const ChipDeviceEvent * event, BaseType_t & yieldRequired);

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void EventLoopTaskMain(void * arg);
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H
