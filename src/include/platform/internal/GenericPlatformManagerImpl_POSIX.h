/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

    enum class State
    {
        kStopped  = 0,
        kRunning  = 1,
        kStopping = 2,
    };

    pthread_t mChipTask;
    bool mInternallyManagedChipTask = false;
    std::atomic<State> mState{ State::kStopped };

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    pthread_cond_t mEventQueueStoppedCond;
    pthread_mutex_t mStateLock;

    pthread_attr_t mChipTaskAttr;
    struct sched_param mChipTaskSchedParam;
#endif

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

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    static void _DispatchEventViaScheduleWork(System::Layer * aLayer, void * appState);
#else

    DeviceSafeQueue mChipEventQueue;
    std::atomic<bool> mShouldRunEventLoop{ true };
    static void * EventLoopTaskMain(void * arg);
#endif
    void ProcessDeviceEvents();
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
// with external libev mainloop, this should be implemented externally to terminate the mainloop cleanly
// (Note that there is a weak default implementation that just calls chipDie() when the external implementation is missing)
extern void ExitExternalMainLoop();
#endif
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
