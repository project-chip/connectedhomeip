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
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl_POSIX<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP

#include "system/SystemError.h"
#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_POSIX<> template inherits.
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
#include "lib/mdns/platform/Mdns.h"
#endif
#include <platform/internal/GenericPlatformManagerImpl.cpp>

#include <system/SystemLayer.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <sys/select.h>
#include <unistd.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
namespace chip {
namespace Mdns {
void UpdateMdnsDataset(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout);
void ProcessMdns(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet);
} // namespace Mdns
} // namespace chip
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_InitChipStack()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret        = 0;

    mChipStackLock = PTHREAD_MUTEX_INITIALIZER;

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

    mShouldRunEventLoop.store(true, std::memory_order_relaxed);

    ret = pthread_cond_init(&mEventQueueStoppedCond, nullptr);
    SuccessOrExit(ret);

    ret = pthread_mutex_init(&mStateLock, nullptr);
    SuccessOrExit(ret);

    mHasValidChipTask = false;

exit:
    if (ret != 0)
    {
        err = System::MapErrorPOSIX(ret);
    }

    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_LockChipStack()
{
    int err = pthread_mutex_lock(&mChipStackLock);
    assert(err == 0);

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    mChipStackIsLocked        = true;
    mChipStackLockOwnerThread = pthread_self();
#endif
}

template <class ImplClass>
bool GenericPlatformManagerImpl_POSIX<ImplClass>::_TryLockChipStack()
{
    bool locked = (pthread_mutex_trylock(&mChipStackLock) == 0);
#if CHIP_STACK_LOCK_TRACKING_ENABLED
    if (locked)
    {
        mChipStackIsLocked        = true;
        mChipStackLockOwnerThread = pthread_self();
    }
#endif
    return locked;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_UnlockChipStack()
{
#if CHIP_STACK_LOCK_TRACKING_ENABLED
    mChipStackIsLocked = false;
#endif

    int err = pthread_mutex_unlock(&mChipStackLock);
    assert(err == 0);
}

#if CHIP_STACK_LOCK_TRACKING_ENABLED
template <class ImplClass>
bool GenericPlatformManagerImpl_POSIX<ImplClass>::_IsChipStackLockedByCurrentThread() const
{
    return !mMainLoopStarted || (mChipStackIsLocked && (pthread_equal(pthread_self(), mChipStackLockOwnerThread)));
}
#endif

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartChipTimer(int64_t aMilliseconds)
{
    // Let SystemLayer.PrepareSelect() handle timers.
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    mChipEventQueue.Push(*event);
    SysOnEventSignal(this); // Trigger wake select on CHIP thread
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::ProcessDeviceEvents()
{
    while (!mChipEventQueue.Empty())
    {
        const ChipDeviceEvent event = mChipEventQueue.PopFront();
        Impl()->DispatchEvent(&event);
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysOnEventSignal(void * arg)
{
    SystemLayer.WakeSelect();
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysUpdate()
{
    FD_ZERO(&mReadSet);
    FD_ZERO(&mWriteSet);
    FD_ZERO(&mErrorSet);
    mMaxFd = 0;

    // Max out this duration and let CHIP set it appropriately.
    mNextTimeout.tv_sec  = DEFAULT_MIN_SLEEP_PERIOD;
    mNextTimeout.tv_usec = 0;

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }

#if !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }
#endif // !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    chip::Mdns::UpdateMdnsDataset(mReadSet, mWriteSet, mErrorSet, mMaxFd, mNextTimeout);
#endif
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::SysProcess()
{
    int selectRes;
    int64_t nextTimeoutMs;

    nextTimeoutMs = mNextTimeout.tv_sec * 1000 + mNextTimeout.tv_usec / 1000;
    _StartChipTimer(nextTimeoutMs);

    Impl()->UnlockChipStack();
    selectRes = select(mMaxFd + 1, &mReadSet, &mWriteSet, &mErrorSet, &mNextTimeout);
    Impl()->LockChipStack();

    if (selectRes < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

#if !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }
#endif // !(CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)

    ProcessDeviceEvents();
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    chip::Mdns::ProcessMdns(mReadSet, mWriteSet, mErrorSet);
#endif
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_RunEventLoop()
{
    pthread_mutex_lock(&mStateLock);

    //
    // If we haven't set mHasValidChipTask by now, it means that the application did not call StartEventLoopTask
    // and consequently, are running the event loop from their own, externally managed task.
    // Let's track his appropriately since we need this info later when stopping the event queues.
    //
    if (!mHasValidChipTask)
    {
        mHasValidChipTask = true;
        mChipTask         = pthread_self();
        mTaskType         = kExternallyManagedTask;
    }

    mEventQueueHasStopped = false;
    pthread_mutex_unlock(&mStateLock);

    Impl()->LockChipStack();

    do
    {
        SysUpdate();
        SysProcess();
    } while (mShouldRunEventLoop.load(std::memory_order_relaxed));

    Impl()->UnlockChipStack();

    pthread_mutex_lock(&mStateLock);
    mEventQueueHasStopped = true;
    pthread_mutex_unlock(&mStateLock);

    //
    // Wake up anyone blocked waiting for the event queue to stop in
    // StopEventLoopTask().
    //
    pthread_cond_signal(&mEventQueueStoppedCond);

    return;
}

template <class ImplClass>
void * GenericPlatformManagerImpl_POSIX<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(arg)->Impl()->mMainLoopStarted = true;
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(arg)->Impl()->RunEventLoop();
    return nullptr;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartEventLoopTask()
{
    int err;
    err = pthread_attr_init(&mChipTaskAttr);
    SuccessOrExit(err);
    err = pthread_attr_getschedparam(&mChipTaskAttr, &mChipTaskSchedParam);
    SuccessOrExit(err);
    err = pthread_attr_setschedpolicy(&mChipTaskAttr, SCHED_RR);
    SuccessOrExit(err);

    //
    // We need to grab the lock here since we have to protect setting
    // mHasValidChipTask, which will be read right away upon creating the
    // thread below.
    //
    pthread_mutex_lock(&mStateLock);

    err = pthread_create(&mChipTask, &mChipTaskAttr, EventLoopTaskMain, this);
    if (err == 0)
    {
        mHasValidChipTask = true;
        mTaskType         = kInternallyManagedTask;
    }

    pthread_mutex_unlock(&mStateLock);

exit:
    return System::MapErrorPOSIX(err);
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StopEventLoopTask()
{
    int err = 0;

    //
    // Signal to the runloop to stop.
    //
    mShouldRunEventLoop.store(false, std::memory_order_relaxed);

    pthread_mutex_lock(&mStateLock);

    //
    // If we're calling this from a different thread than the one running chip, then
    // we need to wait till the event queue has completely stopped before proceeding.
    //
    if (mHasValidChipTask && (pthread_equal(pthread_self(), mChipTask) == 0))
    {
        pthread_mutex_unlock(&mStateLock);

        //
        // We need to grab the lock to protect critical sections accessed by the WakeSelect() call within
        // SystemLayer.
        //
        Impl()->LockChipStack();
        SystemLayer.WakeSelect();
        Impl()->UnlockChipStack();

        pthread_mutex_lock(&mStateLock);

        while (!mEventQueueHasStopped)
        {
            err = pthread_cond_wait(&mEventQueueStoppedCond, &mStateLock);
            SuccessOrExit(err);
        }

        pthread_mutex_unlock(&mStateLock);

        //
        // Wait further for the thread to terminate if we had previously created it.
        //
        if (mTaskType == kInternallyManagedTask)
        {
            err = pthread_join(mChipTask, nullptr);
            SuccessOrExit(err);
        }
    }
    else
    {
        pthread_mutex_unlock(&mStateLock);
    }

exit:
    pthread_mutex_destroy(&mStateLock);
    pthread_cond_destroy(&mEventQueueStoppedCond);
    mHasValidChipTask = false;
    return System::MapErrorPOSIX(err);
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_Shutdown()
{
    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    return System::MapErrorPOSIX(GenericPlatformManagerImpl<ImplClass>::_Shutdown());
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP
