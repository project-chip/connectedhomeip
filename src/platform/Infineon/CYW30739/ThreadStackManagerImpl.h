/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides an implementation of the ThreadStackManager object
 *          for the platforms using the OpenThread stack.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>

#include "EventFlags.h"

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ThreadStackManager singleton object for the platform.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>
{
    // Allow the ThreadStackManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ThreadStackManager;

public:
    // ===== Methods that implement the ThreadStackManager abstract interface.
    CHIP_ERROR _InitThreadStack();

    void SignalThreadActivityPending();
    void SignalThreadActivityPendingFromISR();
    inline bool IsCurrentTask(void) { return wiced_rtos_is_current_thread(mThread) == WICED_SUCCESS; }

protected:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    CHIP_ERROR _StartThreadTask();
    void _LockThreadStack();
    bool _TryLockThreadStack();
    void _UnlockThreadStack();

private:
    // ===== Members for internal use by the following friends.

    friend ThreadStackManager & ::chip::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::chip::DeviceLayer::ThreadStackMgrImpl(void);

    wiced_thread_t * mThread;
    EventFlags mEventFlags;
    wiced_mutex_t * mMutex;
    static ThreadStackManagerImpl sInstance;

    // ===== Private members for use by this class only.

    void ThreadTaskMain(void);

    static void ThreadTaskMain(uint32_t arg);
    static constexpr uint32_t kActivityPendingEventFlag        = 1 << 0;
    static constexpr uint32_t kActivityPendingFromISREventFlag = 1 << 1;
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * Chip applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
inline ThreadStackManager & ThreadStackMgr(void)
{
    return ThreadStackManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * Chip applications can use this to gain access to features of the ThreadStackManager
 * that are specific to the platforms.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
