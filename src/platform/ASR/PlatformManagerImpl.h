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
 *          for the ASR platform.
 */

#pragma once
#if CONFIG_ENABLE_ASR_LEGA_RTOS
#include <platform/internal/GenericPlatformManagerImpl.h>
#else
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.h>
#endif
#include <atomic>
#include <lega_rtos_api.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the ASR platform.
 */
#if CONFIG_ENABLE_ASR_LEGA_RTOS
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
#else
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>
#endif
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;
#if CONFIG_ENABLE_ASR_LEGA_RTOS
public:
    inline bool IsCurrentTask(void) { return lega_rtos_is_current_thread(&mThread) == TRUE; }
#else
    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    CHIP_ERROR InitLwIPCoreLock(void);
#endif
    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _Shutdown();

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    static PlatformManagerImpl sInstance;
#if CONFIG_ENABLE_ASR_LEGA_RTOS
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StopEventLoopTask(void);
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout durationMS);
    std::atomic<bool> mShouldRunEventLoop;
    bool mChipTimerActive;
    lega_thread_t mThread    = NULL;
    lega_queue_t mEventQueue = NULL;
    lega_mutex_t mChipMutex  = NULL;
    lega_timeout_t mNextTimerBaseTime;
    lega_tick_t mNextTimerDurationTicks;
    static void EventLoopTaskMain(uint32_t arg);
    void RunEventLoopInternal(void);
#endif
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
 * that are specific to the ASR platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
