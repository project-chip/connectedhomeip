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

/**
 *    @file
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl_POSIX<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_POSIX<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.ipp>

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

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
namespace {
System::LayerSocketsLoop & SystemLayerSocketsLoop()
{
    return static_cast<System::LayerSocketsLoop &>(DeviceLayer::SystemLayer());
}
} // anonymous namespace
#endif

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_InitChipStack()
{
    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    ReturnErrorOnFailure(GenericPlatformManagerImpl<ImplClass>::_InitChipStack());

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV

    mShouldRunEventLoop.store(true, std::memory_order_relaxed);

    int ret = pthread_cond_init(&mEventQueueStoppedCond, nullptr);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_POSIX(ret));

    ret = pthread_mutex_init(&mStateLock, nullptr);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_POSIX(ret));
#endif

    return CHIP_NO_ERROR;
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
    if (!mChipStackIsLocked)
    {
        ChipLogError(DeviceLayer, "_UnlockChipStack while unlocked");
#if CHIP_STACK_LOCK_TRACKING_ERROR_FATAL
        chipDie();
#endif
    }
    mChipStackIsLocked        = false;
    mChipStackLockOwnerThread = pthread_t();
#endif

    int err = pthread_mutex_unlock(&mChipStackLock);
    assert(err == 0);
}

#if CHIP_STACK_LOCK_TRACKING_ENABLED
template <class ImplClass>
bool GenericPlatformManagerImpl_POSIX<ImplClass>::_IsChipStackLockedByCurrentThread() const
{
    // If no Matter thread is currently running we do not have to worry about
    // locking. Hence, this function always returns true in that case.
    if (mState.load(std::memory_order_relaxed) == State::kStopped)
        return true;
    return mChipStackIsLocked && (pthread_equal(pthread_self(), mChipStackLockOwnerThread));
}
#endif

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartChipTimer(System::Clock::Timeout delay)
{
    // Let System::LayerSocketsLoop.PrepareEvents() handle timers.
    return CHIP_NO_ERROR;
}

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_DispatchEventViaScheduleWork(System::Layer * aLayer, void * appState)
{
    auto * event = static_cast<const ChipDeviceEvent *>(appState);
    PlatformMgrImpl().DispatchEvent(event);
    delete event;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    // Note: PostEvent() is documented to allow being called "from any thread".
    //   In the libev mainloop case however, calling from another thread is NOT supported.
    //   Introducing this restriction is OK because the very goal of using libev is to avoid
    //   multiple threads by running matter and all application code in the same thread on the
    //   libev mainloop. So getting called from another thread here is very likely a
    //   application design error.
    VerifyOrDieWithMsg(_IsChipStackLockedByCurrentThread(), DeviceLayer, "PostEvent() not allowed from outside chip stack lock");

    // Schedule dispatching this event via System Layer's ScheduleWork
    ChipDeviceEvent * eventCopyP = new ChipDeviceEvent;
    VerifyOrDie(eventCopyP != nullptr);
    *eventCopyP = *event;
    SystemLayer().ScheduleWork(&_DispatchEventViaScheduleWork, eventCopyP);
    return CHIP_NO_ERROR;
#else
    mChipEventQueue.Push(*event);

    SystemLayerSocketsLoop().Signal(); // Trigger wake select on CHIP thread
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV
}

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV

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
void GenericPlatformManagerImpl_POSIX<ImplClass>::_RunEventLoop()
{
#if CHIP_SYSTEM_CONFIG_USE_LIBEV

    VerifyOrDieWithMsg(false, DeviceLayer, "libev based app should never try to run a separate event loop");

#else

    pthread_mutex_lock(&mStateLock);

    //
    // If we haven't set mInternallyManagedChipTask by now, it means that the application did not call
    // StartEventLoopTask and consequently, are running the event loop from their own, externally managed
    // task.
    //
    if (!mInternallyManagedChipTask)
    {
        mChipTask = pthread_self();
        mState.store(State::kRunning, std::memory_order_relaxed);
        mShouldRunEventLoop.store(true, std::memory_order_relaxed);
    }

    pthread_mutex_unlock(&mStateLock);

    Impl()->LockChipStack();

    SystemLayerSocketsLoop().EventLoopBegins();
    do
    {
        SystemLayerSocketsLoop().PrepareEvents();

        Impl()->UnlockChipStack();
        SystemLayerSocketsLoop().WaitForEvents();
        Impl()->LockChipStack();

        SystemLayerSocketsLoop().HandleEvents();

        ProcessDeviceEvents();
    } while (mShouldRunEventLoop.load(std::memory_order_relaxed));
    SystemLayerSocketsLoop().EventLoopEnds();

    Impl()->UnlockChipStack();

    pthread_mutex_lock(&mStateLock);
    mState.store(State::kStopping, std::memory_order_relaxed);
    pthread_mutex_unlock(&mStateLock);

    //
    // Wake up anyone blocked waiting for the event queue to stop in
    // StopEventLoopTask().
    //
    pthread_cond_signal(&mEventQueueStoppedCond);

    //
    // Mark event loop as truly stopped. After that line, we can not use any
    // non-simple type member variables, because they can be destroyed by the
    // Shutdown() method.
    //
    mState.store(State::kStopped, std::memory_order_relaxed);
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV
}

template <class ImplClass>
void * GenericPlatformManagerImpl_POSIX<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_POSIX<ImplClass> *>(arg)->Impl()->RunEventLoop();
    return nullptr;
}

#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StartEventLoopTask()
{

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    // Note: With libev, we dont need our own mainloop.
    //   Still, we set State::kRunning to activate lock checking, because
    //   calls to ScheduleWork and some System Layer methods may not
    //   occur from other threads (which usually don't exist in a
    //   libev app)
    mState.store(State::kRunning, std::memory_order_relaxed);
    return CHIP_NO_ERROR;
#else

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

    mShouldRunEventLoop.store(true, std::memory_order_relaxed);

    //
    // We need to grab the lock here since we have to protect setting
    // mHasValidChipTask, which will be read right away upon creating the
    // thread below.
    //
    pthread_mutex_lock(&mStateLock);

    err = pthread_create(&mChipTask, &mChipTaskAttr, EventLoopTaskMain, this);
    if (err == 0)
    {
        mInternallyManagedChipTask = true;
        mState.store(State::kRunning, std::memory_order_relaxed);
    }

    pthread_mutex_unlock(&mStateLock);

    return CHIP_ERROR_POSIX(err);
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV
}

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
// fallback implementation
void __attribute__((weak)) ExitExternalMainLoop()
{
    // FIXME: implement better exit
    VerifyOrDieWithMsg(false, DeviceLayer, "Missing custom ExitExternalMainLoop() implementation for clean shutdown -> just die");
}
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_POSIX<ImplClass>::_StopEventLoopTask()
{

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    // with libev, the mainloop is set up and managed externally
    mState.store(State::kStopping, std::memory_order_relaxed);
    ExitExternalMainLoop(); // this callback needs to be implemented.
    mState.store(State::kStopped, std::memory_order_relaxed);
    return CHIP_NO_ERROR;
#else

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
    auto isRunning = mState.load(std::memory_order_relaxed) == State::kRunning;
    if (isRunning && (pthread_equal(pthread_self(), mChipTask) == 0))
    {
        pthread_mutex_unlock(&mStateLock);

        //
        // We need to grab the lock to protect critical sections accessed by the WakeSelect() call within
        // System::Layer.
        //
        Impl()->LockChipStack();
        SystemLayerSocketsLoop().Signal();
        Impl()->UnlockChipStack();

        pthread_mutex_lock(&mStateLock);

        while (mState.load(std::memory_order_relaxed) == State::kRunning)
        {
            err = pthread_cond_wait(&mEventQueueStoppedCond, &mStateLock);
            VerifyOrExit(err == 0, );
        }

        pthread_mutex_unlock(&mStateLock);

        //
        // Wait further for the thread to terminate if we had previously created it.
        //
        if (mInternallyManagedChipTask)
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
    return CHIP_ERROR_POSIX(err);
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV
}

template <class ImplClass>
void GenericPlatformManagerImpl_POSIX<ImplClass>::_Shutdown()
{
    //
    // We cannot shutdown the stack while the event loop is still running. This can lead
    // to use after free errors - here we are destroying mutex and condition variable that
    // are still in use by the event loop!
    //
    VerifyOrDie(mState.load(std::memory_order_relaxed) == State::kStopped);

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    pthread_mutex_destroy(&mStateLock);
    pthread_cond_destroy(&mEventQueueStoppedCond);
#endif

    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_POSIX_CPP
