/*
 *
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
 *          Provides an generic implementation of PlatformManager features
 *          for use on FreeRTOS platforms.
 */


#ifndef GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H
#define GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H

#include <Weave/DeviceLayer/internal/GenericPlatformManagerImpl.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#endif

namespace nl {
namespace Weave {
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
template<class ImplClass>
class GenericPlatformManagerImpl_FreeRTOS
    : public GenericPlatformManagerImpl<ImplClass>
{
protected:

    TimeOut_t mNextTimerBaseTime;
    TickType_t mNextTimerDurationTicks;
    SemaphoreHandle_t mWeaveStackLock;
    QueueHandle_t mWeaveEventQueue;
    TaskHandle_t mEventLoopTask;
    bool mWeaveTimerActive;

    // ===== Methods that implement the PlatformManager abstract interface.

    WEAVE_ERROR _InitWeaveStack();
    void _LockWeaveStack(void);
    bool _TryLockWeaveStack(void);
    void _UnlockWeaveStack(void);
    void _PostEvent(const WeaveDeviceEvent * event);
    void _RunEventLoop(void);
    WEAVE_ERROR _StartEventLoopTask(void);
    WEAVE_ERROR _StartWeaveTimer(uint32_t durationMS);

    // ===== Methods available to the implementation subclass.

    void PostEventFromISR(const WeaveDeviceEvent * event, BaseType_t & yieldRequired);

private:

    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass*>(this); }

    static void EventLoopTaskMain(void * arg);
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_H
