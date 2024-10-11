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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include <lib/core/Global.h>
#include <platform/Darwin/BleScannerDelegate.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <platform/internal/GenericPlatformManagerImpl.h>
#else
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#include <atomic>
#include <dispatch/dispatch.h>

namespace chip {
namespace DeviceLayer {

class BleScannerDelegate;

/**
 * Concrete implementation of the PlatformManager singleton object for Darwin platforms.
 */
class PlatformManagerImpl final : public PlatformManager,
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
                                  public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
#else
                                  public Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    dispatch_queue_t GetWorkQueue() { return mWorkQueue; }
    bool IsWorkQueueCurrentQueue() const;

    CHIP_ERROR StartBleScan(BleScannerDelegate * delegate, BleScanMode mode = BleScanMode::kDefault);
    CHIP_ERROR StopBleScan();

    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.
    CHIP_ERROR _InitChipStack();

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout delay) { return CHIP_ERROR_NOT_IMPLEMENTED; };
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StopEventLoopTask();

    void _RunEventLoop();
    void _LockChipStack(){};
    bool _TryLockChipStack() { return false; };
    void _UnlockChipStack(){};
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const;
#endif

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);

    friend AtomicGlobal<PlatformManagerImpl>;
    static AtomicGlobal<PlatformManagerImpl> sInstance;

    PlatformManagerImpl();

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    dispatch_queue_t mWorkQueue;

    enum class WorkQueueState
    {
        kSuspended,
        kRunning,
        kSuspensionPending,
    };

    std::atomic<WorkQueueState> mWorkQueueState = WorkQueueState::kSuspended;

    // Semaphore used to implement blocking behavior in _RunEventLoop.
    dispatch_semaphore_t mRunLoopSem;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance.get();
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance.get();
}

} // namespace DeviceLayer
} // namespace chip
