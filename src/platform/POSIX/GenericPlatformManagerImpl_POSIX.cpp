/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/POSIX/GenericPlatformManagerImpl_POSIX.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sched.h>
#include <unistd.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR GenericPlatformManagerImpl_POSIX::InitChipStackInner()
{
    mChipStackLock = PTHREAD_MUTEX_INITIALIZER;

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    ReturnErrorOnFailure(GenericPlatformManagerImpl::InitChipStackInner());

    mShouldRunEventLoop.store(true, std::memory_order_relaxed);

    int ret = pthread_cond_init(&mEventQueueStoppedCond, nullptr);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_POSIX(ret));

    ret = pthread_mutex_init(&mStateLock, nullptr);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_POSIX(ret));

    mHasValidChipTask = false;

    return CHIP_NO_ERROR;
}

void GenericPlatformManagerImpl_POSIX::LockChipStack()
{
    int err = pthread_mutex_lock(&mChipStackLock);
    assert(err == 0);

#if CHIP_STACK_LOCK_TRACKING_ENABLED
    mChipStackIsLocked        = true;
    mChipStackLockOwnerThread = pthread_self();
#endif
}

bool GenericPlatformManagerImpl_POSIX::TryLockChipStack()
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

void GenericPlatformManagerImpl_POSIX::UnlockChipStack()
{
#if CHIP_STACK_LOCK_TRACKING_ENABLED
    mChipStackIsLocked = false;
#endif

    int err = pthread_mutex_unlock(&mChipStackLock);
    assert(err == 0);
}

#if CHIP_STACK_LOCK_TRACKING_ENABLED
bool GenericPlatformManagerImpl_POSIX::IsChipStackLockedByCurrentThread() const
{
    return !mMainLoopStarted || (mChipStackIsLocked && (pthread_equal(pthread_self(), mChipStackLockOwnerThread)));
}
#endif

CHIP_ERROR GenericPlatformManagerImpl_POSIX::StartChipTimer(System::Clock::Timeout delay)
{
    // Let System::LayerSocketsLoop.PrepareEvents() handle timers.
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericPlatformManagerImpl_POSIX::PostEvent(const ChipDeviceEvent * event)
{
    mChipEventQueue.Push(*event);

    DeviceLayer::SystemLayerSocketsLoop().Signal(); // Trigger wake select on CHIP thread
    return CHIP_NO_ERROR;
}

void GenericPlatformManagerImpl_POSIX::ProcessDeviceEvents()
{
    while (!mChipEventQueue.Empty())
    {
        const ChipDeviceEvent event = mChipEventQueue.PopFront();
        DispatchEvent(&event);
    }
}

void GenericPlatformManagerImpl_POSIX::RunEventLoop()
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

    LockChipStack();

    DeviceLayer::SystemLayerSocketsLoop().EventLoopBegins();
    do
    {
        DeviceLayer::SystemLayerSocketsLoop().PrepareEvents();

        UnlockChipStack();
        DeviceLayer::SystemLayerSocketsLoop().WaitForEvents();
        LockChipStack();

        DeviceLayer::SystemLayerSocketsLoop().HandleEvents();

        ProcessDeviceEvents();
    } while (mShouldRunEventLoop.load(std::memory_order_relaxed));
    DeviceLayer::SystemLayerSocketsLoop().EventLoopEnds();

    UnlockChipStack();

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

void * GenericPlatformManagerImpl_POSIX::EventLoopTaskMain(void * arg)
{
    GenericPlatformManagerImpl_POSIX * me = static_cast<GenericPlatformManagerImpl_POSIX *>(arg);
    ChipLogDetail(DeviceLayer, "CHIP task running");
    me->mMainLoopStarted = true;
    me->RunEventLoop();
    return nullptr;
}

CHIP_ERROR GenericPlatformManagerImpl_POSIX::StartEventLoopTask()
{
    int err;
    err = pthread_attr_init(&mChipTaskAttr);
    VerifyOrReturnError(err == 0, CHIP_ERROR_POSIX(err));
    err = pthread_attr_getschedparam(&mChipTaskAttr, &mChipTaskSchedParam);
    VerifyOrReturnError(err == 0, CHIP_ERROR_POSIX(err));

#if CHIP_DEVICE_CONFIG_RUN_AS_ROOT
    // set SCHED_RR need root/admin on Android
    err = pthread_attr_setschedpolicy(&mChipTaskAttr, SCHED_RR);
    VerifyOrReturnError(err == 0, CHIP_ERROR_POSIX(err));
#endif

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

    return CHIP_ERROR_POSIX(err);
}

CHIP_ERROR GenericPlatformManagerImpl_POSIX::StopEventLoopTask()
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
        // System::Layer.
        //
        LockChipStack();
        DeviceLayer::SystemLayerSocketsLoop().Signal();
        UnlockChipStack();

        pthread_mutex_lock(&mStateLock);

        while (!mEventQueueHasStopped)
        {
            err = pthread_cond_wait(&mEventQueueStoppedCond, &mStateLock);
            VerifyOrExit(err == 0, );
        }

        pthread_mutex_unlock(&mStateLock);

        //
        // Wait further for the thread to terminate if we had previously created it.
        //
        if (mTaskType == kInternallyManagedTask)
        {
            err = pthread_join(mChipTask, nullptr);
            VerifyOrExit(err == 0, );
        }
    }
    else
    {
        pthread_mutex_unlock(&mStateLock);
    }

exit:
    mHasValidChipTask = false;
    return CHIP_ERROR_POSIX(err);
}

CHIP_ERROR GenericPlatformManagerImpl_POSIX::ShutdownInner()
{
    pthread_mutex_destroy(&mStateLock);
    pthread_cond_destroy(&mEventQueueStoppedCond);

    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    return GenericPlatformManagerImpl::ShutdownInner();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
