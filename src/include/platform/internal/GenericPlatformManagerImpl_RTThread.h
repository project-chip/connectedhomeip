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
 *          Provides a generic implementation of PlatformManager features
 *          for use on RT-Thread platforms.
 */

#pragma once

#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#include <rtthread.h>
#include <rthw.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on RT-Thread platforms.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on RT-Thread-based platforms. It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_RTThread : public GenericPlatformManagerImpl<ImplClass>
{
protected:
    rt_event_t mChipEventFlag;
    rt_thread_t mEventLoopThread;
    rt_mq_t mChipEventQueue;
    bool mChipTimerActive;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack();
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask();
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown(void);

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const;
#endif // CHIP_STACK_LOCK_TRACKING_ENABLED

    // ===== Methods available to the implementation subclass.

    void PostEventFromISR(const ChipDeviceEvent * event, bool & yieldRequired);

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    static void EventLoopThreadMain(void * arg);
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_RTThread<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip