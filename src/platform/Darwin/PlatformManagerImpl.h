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
#include <platform/internal/GenericPlatformManagerImpl.h>

#include <atomic>
#include <dispatch/dispatch.h>

namespace chip {
namespace DeviceLayer {

class BleScannerDelegate;

/**
 * Concrete implementation of the PlatformManager singleton object for Darwin platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    dispatch_queue_t GetWorkQueue() { return mWorkQueue; }
    bool IsWorkQueueCurrentQueue() const;

    CHIP_ERROR StartBleScan(BleScannerDelegate * delegate = nullptr);
    CHIP_ERROR StopBleScan();
    CHIP_ERROR PrepareCommissioning();

    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.
    CHIP_ERROR _InitChipStack();
    void _Shutdown();

    CHIP_ERROR _StartChipTimer(System::Clock::Timeout delay) { return CHIP_ERROR_NOT_IMPLEMENTED; };
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StopEventLoopTask();

    void _RunEventLoop();
    void _LockChipStack(){};
    bool _TryLockChipStack() { return false; };
    void _UnlockChipStack(){};
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);

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
