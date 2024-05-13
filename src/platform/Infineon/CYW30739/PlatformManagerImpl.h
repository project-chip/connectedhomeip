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
 *          Provides an implementation of the PlatformManager object.
 *          for the platform.
 */

#pragma once

#include <hal/wiced_timer.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#include "EventFlags.h"

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the platform.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

public:
    inline bool IsCurrentTask(void) { return wiced_rtos_is_current_thread(mThread) == WICED_SUCCESS; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask(void);
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout durationMS);
    void _Shutdown(void);

    void HandleTimerEvent(void);
    void HandlePostEvent(void);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);

    wiced_thread_t * mThread;
    EventFlags mEventFlags;
    wiced_queue_t * mEventQueue;
    wiced_timer_t mTimer;
    wiced_mutex_t * mMutex;

    static void EventLoopTaskMain(uint32_t arg);
    static void TimerCallback(WICED_TIMER_PARAM_TYPE params);
    static int GetEntropy(void * data, unsigned char * output, size_t len, size_t * olen);
    static PlatformManagerImpl sInstance;
    static constexpr uint32_t kTimerEventFlag = 1 << 0;
    static constexpr uint32_t kPostEventFlag  = 1 << 1;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the CYW30739 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
