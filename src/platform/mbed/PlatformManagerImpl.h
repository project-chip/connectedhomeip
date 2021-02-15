/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include "events/EventQueue.h"
#include "rtos/Mutex.h"
#include "rtos/Thread.h"
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the nRF Connect SDK platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    /* none so far */

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _LockChipStack();
    bool _TryLockChipStack();
    void _UnlockChipStack();
    void _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop();
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StartChipTimer(int64_t durationMS);
    CHIP_ERROR _Shutdown();

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class Internal::BLEManagerImpl;

    static PlatformManagerImpl sInstance;

    // ===== Members for internal use.
    static CHIP_ERROR TranslateOsStatus(osStatus status);
    bool IsLoopActive();

    bool mInitialized = false;
    rtos::Thread mLoopTask{ osPriorityNormal, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
                            /* memory provided */ nullptr, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME };
    rtos::Mutex mChipStackMutex;
    static const size_t event_size = EVENTS_EVENT_SIZE + sizeof(void *) + sizeof(ChipDeviceEvent *);
    events::EventQueue mQueue      = { event_size * CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE };
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}
} // namespace DeviceLayer
} // namespace chip
