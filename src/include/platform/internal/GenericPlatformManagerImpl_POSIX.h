/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an generic implementation of PlatformManager features
 *          for use on Linux platforms.
 */

#pragma once

#include <platform/DeviceSafeQueue.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#include <fcntl.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <pthread.h>
#include <queue>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on any OSAL platform.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on OSAL-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_POSIX : public GenericPlatformManagerImpl<ImplClass>
{
protected:
    // OS-specific members (pthread)
    pthread_mutex_t mChipStackLock = PTHREAD_MUTEX_INITIALIZER;

    enum TaskType
    {
        kExternallyManagedTask = 0,
        kInternallyManagedTask = 1
    };

    pthread_t mChipTask;
    bool mHasValidChipTask = false;
    TaskType mTaskType;
    pthread_cond_t mEventQueueStoppedCond;
    pthread_mutex_t mStateLock;

    //
    // TODO: This variable is very similar to mMainLoopIsStarted, track the
    // cleanup and consolidation in this issue:
    //
    bool mEventQueueHasStopped = false;

    pthread_attr_t mChipTaskAttr;
    struct sched_param mChipTaskSchedParam;

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool mChipStackIsLocked = false;
    pthread_t mChipStackLockOwnerThread;
#endif

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR
    _InitChipStack();
    void _LockChipStack();
    bool _TryLockChipStack();
    void _UnlockChipStack();
    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop();
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StopEventLoopTask();
    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown();

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    bool _IsChipStackLockedByCurrentThread() const;
#endif

    // ===== Methods available to the implementation subclass.

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    void ProcessDeviceEvents();

    DeviceSafeQueue mChipEventQueue;
    std::atomic<bool> mShouldRunEventLoop;
    static void * EventLoopTaskMain(void * arg);
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
