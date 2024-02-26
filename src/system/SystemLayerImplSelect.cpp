/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file implements Layer using select().
 */

#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <platform/LockTracker.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplSelect.h>

#include <errno.h>

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)
#define PTHREAD_NULL 0
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
// older libev do not yet have ev_io_modify
#ifndef ev_io_modify
#define ev_io_modify(ev, events_)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        (ev)->events = ((ev)->events & EV__IOFDSET) | (events_);                                                                   \
    } while (0)
#endif // ev_io_modify
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

namespace chip {
namespace System {

constexpr Clock::Seconds64 kDefaultMinSleepPeriod = Clock::Seconds64(60 * 60 * 24 * 30); // Month [sec]

CHIP_ERROR LayerImplSelect::Init()
{
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    RegisterPOSIXErrorFormatter();

    for (auto & w : mSocketWatchPool)
    {
        w.Clear();
    }

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    // Create an event to allow an arbitrary thread to wake the thread in the select loop.
    ReturnErrorOnFailure(mWakeEvent.Open(*this));
#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV

    VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void LayerImplSelect::Shutdown()
{
    VerifyOrReturn(mLayerState.SetShuttingDown());

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    TimerList::Node * timer;
    while ((timer = mTimerList.PopEarliest()) != nullptr)
    {
        if (timer->mTimerSource != nullptr)
        {
            dispatch_source_cancel(timer->mTimerSource);
            dispatch_release(timer->mTimerSource);
        }
    }
    mTimerPool.ReleaseAll();

    for (auto & w : mSocketWatchPool)
    {
        w.DisableAndClear();
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    TimerList::Node * timer;
    while ((timer = mTimerList.PopEarliest()) != nullptr)
    {
        if (ev_is_active(&timer->mLibEvTimer))
        {
            ev_timer_stop(mLibEvLoopP, &timer->mLibEvTimer);
        }
    }
    mTimerPool.ReleaseAll();

    for (auto & w : mSocketWatchPool)
    {
        w.DisableAndClear();
    }
#else
    mTimerList.Clear();
    mTimerPool.ReleaseAll();
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    mWakeEvent.Close(*this);
#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV

    mLayerState.ResetFromShuttingDown(); // Return to uninitialized state to permit re-initialization.
}

void LayerImplSelect::Signal()
{
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    ChipLogError(DeviceLayer, "Signal() should not be called in CHIP_SYSTEM_CONFIG_USE_LIBEV builds (might be ok in tests)");
#else
    /*
     * Wake up the I/O thread by writing a single byte to the wake pipe.
     *
     * If this is being called from within an I/O event callback, then writing to the wake pipe can be skipped,
     * since the I/O thread is already awake.
     *
     * Furthermore, we don't care if this write fails as the only reasonably likely failure is that the pipe is full, in which
     * case the select calling thread is going to wake up anyway.
     */
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    if (pthread_equal(mHandleSelectThread, pthread_self()))
    {
        return;
    }
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Send notification to wake up the select call.
    CHIP_ERROR status = mWakeEvent.Notify();
    if (status != CHIP_NO_ERROR)
    {

        ChipLogError(chipSystemLayer, "System wake event notify failed: %" CHIP_ERROR_FORMAT, status.Format());
    }
#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV
}

CHIP_ERROR LayerImplSelect::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = System::Clock::kZero);

    CancelTimer(onComplete, appState);

    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = GetDispatchQueue();
    if (dispatchQueue)
    {
        (void) mTimerList.Add(timer);
        dispatch_source_t timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, dispatchQueue);
        VerifyOrDie(timerSource != nullptr);

        timer->mTimerSource = timerSource;
        dispatch_source_set_timer(
            timerSource, dispatch_walltime(nullptr, static_cast<int64_t>(Clock::Milliseconds64(delay).count() * NSEC_PER_MSEC)),
            DISPATCH_TIME_FOREVER, 2 * NSEC_PER_MSEC);
        dispatch_source_set_event_handler(timerSource, ^{
            dispatch_source_cancel(timerSource);
            dispatch_release(timerSource);

            this->HandleTimerComplete(timer);
        });
        dispatch_resume(timerSource);
        return CHIP_NO_ERROR;
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    VerifyOrDie(mLibEvLoopP != nullptr);
    ev_timer_init(&timer->mLibEvTimer, &LayerImplSelect::HandleLibEvTimer, 1, 0);
    timer->mLibEvTimer.data = timer;
    auto t                  = Clock::Milliseconds64(delay).count();
    // Note: libev uses the time when events started processing as the "now" reference for relative timers,
    //   for efficiency reasons. This point in time is represented by ev_now().
    //   The real time is represented by ev_time().
    //   Without correction, this leads to timers firing a bit too early relative to the time StartTimer()
    //   is called. So the relative value passed to ev_timer_set() is adjusted (increased) here.
    // Note: Still, slightly early (and of course, late) firing timers are something the caller MUST be prepared for,
    //   because edge cases like system clock adjustments may cause them even with the correction applied here.
    ev_timer_set(&timer->mLibEvTimer, (static_cast<double>(t) / 1E3) + ev_time() - ev_now(mLibEvLoopP), 0.);
    (void) mTimerList.Add(timer);
    ev_timer_start(mLibEvLoopP, &timer->mLibEvTimer);
    return CHIP_NO_ERROR;
#endif
#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    // Note: dispatch based implementation needs this as fallback, but not LIBEV (and dead code is not allowed with -Werror)
    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        Signal();
    }
    return CHIP_NO_ERROR;
#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV
}

CHIP_ERROR LayerImplSelect::ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(delay.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);

    assertChipStackLockedByCurrentThread();

    Clock::Timeout remainingTime = mTimerList.GetRemainingTime(onComplete, appState);
    if (remainingTime.count() < delay.count())
    {
        if (remainingTime == Clock::kZero)
        {
            // If remaining time is Clock::kZero, it might possible that our timer is in
            // the mExpiredTimers list and about to be fired. Remove it from that list, since we are extending it.
            mExpiredTimers.Remove(onComplete, appState);
        }
        return StartTimer(delay, onComplete, appState);
    }

    return CHIP_NO_ERROR;
}

bool LayerImplSelect::IsTimerActive(TimerCompleteCallback onComplete, void * appState)
{
    bool timerIsActive = (mTimerList.GetRemainingTime(onComplete, appState) > Clock::kZero);

    if (!timerIsActive)
    {
        // check if the timer is in the mExpiredTimers list about to be fired.
        for (TimerList::Node * timer = mExpiredTimers.Earliest(); timer != nullptr; timer = timer->mNextTimer)
        {
            if (timer->GetCallback().GetOnComplete() == onComplete && timer->GetCallback().GetAppState() == appState)
            {
                return true;
            }
        }
    }

    return timerIsActive;
}

Clock::Timeout LayerImplSelect::GetRemainingTime(TimerCompleteCallback onComplete, void * appState)
{
    return mTimerList.GetRemainingTime(onComplete, appState);
}

void LayerImplSelect::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mLayerState.IsInitialized());

    TimerList::Node * timer = mTimerList.Remove(onComplete, appState);
    if (timer == nullptr)
    {
        // The timer was not in our "will fire in the future" list, but it might
        // be in the "we're about to fire these" chunk we already grabbed from
        // that list.  Check for it there too, and if found there we still want
        // to cancel it.
        timer = mExpiredTimers.Remove(onComplete, appState);
    }
    VerifyOrReturn(timer != nullptr);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (timer->mTimerSource != nullptr)
    {
        dispatch_source_cancel(timer->mTimerSource);
        dispatch_release(timer->mTimerSource);
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    VerifyOrDie(mLibEvLoopP != nullptr);
    ev_timer_stop(mLibEvLoopP, &timer->mLibEvTimer);
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

    mTimerPool.Release(timer);
#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    // LIBEV has no I/O wakeup thread, so must not call Signal()
    Signal();
#endif
}

CHIP_ERROR LayerImplSelect::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = GetDispatchQueue();
    if (dispatchQueue)
    {
        dispatch_async(dispatchQueue, ^{
            onComplete(this, appState);
        });
        return CHIP_NO_ERROR;
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    // schedule as timer with no delay, but do NOT cancel previous timers with same onComplete/appState!
    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp(), onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);
    VerifyOrDie(mLibEvLoopP != nullptr);
    ev_timer_init(&timer->mLibEvTimer, &LayerImplSelect::HandleLibEvTimer, 1, 0);
    timer->mLibEvTimer.data = timer;
    auto t                  = Clock::Milliseconds64(0).count();
    ev_timer_set(&timer->mLibEvTimer, static_cast<double>(t) / 1E3, 0.);
    (void) mTimerList.Add(timer);
    ev_timer_start(mLibEvLoopP, &timer->mLibEvTimer);
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV
#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
    // Note: dispatch based implementation needs this as fallback, but not LIBEV (and dead code is not allowed with -Werror)
    // Ideally we would not use a timer here at all, but if we try to just
    // ScheduleLambda the lambda needs to capture the following:
    // 1) onComplete
    // 2) appState
    // 3) The `this` pointer, because onComplete needs to be passed a pointer to
    //    the System::Layer.
    //
    // On a 64-bit system that's 24 bytes, but lambdas passed to ScheduleLambda
    // are capped at CHIP_CONFIG_LAMBDA_EVENT_SIZE which is 16 bytes.
    //
    // So for now use a timer as a poor-man's closure that captures `this` and
    // onComplete and appState in a single pointer, so we fit inside the size
    // limit.
    //
    // TODO: We could do something here where we compile-time condition on the
    // sizes of things and use a direct ScheduleLambda if it would fit and this
    // setup otherwise.
    //
    // TODO: But also, unit tests seem to do SystemLayer::ScheduleWork without
    // actually running a useful event loop (in the PlatformManager sense),
    // which breaks if we use ScheduleLambda here, since that does rely on the
    // PlatformManager event loop. So for now, keep scheduling an expires-ASAP
    // timer, but just make sure we don't cancel existing timers with the same
    // callback and appState, so ScheduleWork invocations don't stomp on each
    // other.
    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp(), onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        Signal();
    }
    return CHIP_NO_ERROR;
#endif // !CHIP_SYSTEM_CONFIG_USE_LIBEV
}

CHIP_ERROR LayerImplSelect::StartWatchingSocket(int fd, SocketWatchToken * tokenOut)
{
    // Find a free slot.
    SocketWatch * watch = nullptr;
    for (auto & w : mSocketWatchPool)
    {
        if (w.mFD == fd)
        {
            // Duplicate registration is an error.
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if ((w.mFD == kInvalidFd) && (watch == nullptr))
        {
            watch = &w;
        }
    }
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_ENDPOINT_POOL_FULL);

    watch->mFD = fd;
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    ev_io_init(&watch->mIoWatcher, &LayerImplSelect::HandleLibEvIoWatcher, 0, 0);
    watch->mIoWatcher.data   = watch;
    watch->mLayerImplSelectP = this;
#endif

    *tokenOut = reinterpret_cast<SocketWatchToken>(watch);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mCallback     = callback;
    watch->mCallbackData = data;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::RequestCallbackOnPendingRead(SocketWatchToken token)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mPendingIO.Set(SocketEventFlags::kRead);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (watch->mRdSource == nullptr)
    {
        // First time requesting callback for read events: install a dispatch source
        dispatch_queue_t dispatchQueue = GetDispatchQueue();
        if (dispatchQueue == nullptr)
        {
            // Note: if no dispatch queue is available, callbacks most probably will not work,
            //       unless, as in some tests from a test-specific local loop,
            //       the select based event handling (Prepare/WaitFor/HandleEvents) is invoked.
            ChipLogError(DeviceLayer,
                         "RequestCallbackOnPendingRead with no dispatch queue: callback may not work (might be ok in tests)");
        }
        else
        {
            watch->mRdSource =
                dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, static_cast<uintptr_t>(watch->mFD), 0, dispatchQueue);
            ReturnErrorCodeIf(watch->mRdSource == nullptr, CHIP_ERROR_NO_MEMORY);
            dispatch_source_set_event_handler(watch->mRdSource, ^{
                if (watch->mPendingIO.Has(SocketEventFlags::kRead) && watch->mCallback != nullptr)
                {
                    SocketEvents events;
                    events.Set(SocketEventFlags::kRead);
                    watch->mCallback(events, watch->mCallbackData);
                }
            });
            // only now we are sure the source exists and can become active
            dispatch_activate(watch->mRdSource);
        }
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    VerifyOrDie(mLibEvLoopP != nullptr);
    int evs = (watch->mPendingIO.Has(SocketEventFlags::kRead) ? EV_READ : 0) |
        (watch->mPendingIO.Has(SocketEventFlags::kWrite) ? EV_WRITE : 0);
    if (!ev_is_active(&watch->mIoWatcher))
    {
        // First time actually using that watch
        ev_io_set(&watch->mIoWatcher, watch->mFD, evs);
        ev_io_start(mLibEvLoopP, &watch->mIoWatcher);
    }
    else
    {
        // already active, just change flags
        // Note: changing flags only reliably works when the watcher is stopped
        ev_io_stop(mLibEvLoopP, &watch->mIoWatcher);
        ev_io_modify(&watch->mIoWatcher, evs);
        ev_io_start(mLibEvLoopP, &watch->mIoWatcher);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::RequestCallbackOnPendingWrite(SocketWatchToken token)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mPendingIO.Set(SocketEventFlags::kWrite);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (watch->mWrSource == nullptr)
    {
        // First time requesting callback for read events: install a dispatch source
        dispatch_queue_t dispatchQueue = GetDispatchQueue();
        if (dispatchQueue == nullptr)
        {
            // Note: if no dispatch queue is available, callbacks most probably will not work,
            //       unless, as in some tests from a test-specific local loop,
            //       the select based event handling (Prepare/WaitFor/HandleEvents) is invoked.
            ChipLogError(DeviceLayer,
                         "RequestCallbackOnPendingWrite with no dispatch queue: callback may not work (might be ok in tests)");
        }
        else
        {
            watch->mWrSource =
                dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, static_cast<uintptr_t>(watch->mFD), 0, dispatchQueue);
            ReturnErrorCodeIf(watch->mWrSource == nullptr, CHIP_ERROR_NO_MEMORY);
            dispatch_source_set_event_handler(watch->mWrSource, ^{
                if (watch->mPendingIO.Has(SocketEventFlags::kWrite) && watch->mCallback != nullptr)
                {
                    SocketEvents events;
                    events.Set(SocketEventFlags::kWrite);
                    watch->mCallback(events, watch->mCallbackData);
                }
            });
            // only now we are sure the source exists and can become active
            dispatch_activate(watch->mWrSource);
        }
    }
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    VerifyOrDie(mLibEvLoopP != nullptr);
    int evs = (watch->mPendingIO.Has(SocketEventFlags::kRead) ? EV_READ : 0) |
        (watch->mPendingIO.Has(SocketEventFlags::kWrite) ? EV_WRITE : 0);
    if (!ev_is_active(&watch->mIoWatcher))
    {
        // First time actually using that watch
        ev_io_set(&watch->mIoWatcher, watch->mFD, evs);
        ev_io_start(mLibEvLoopP, &watch->mIoWatcher);
    }
    else
    {
        // already active, just change flags
        // Note: changing flags only reliably works when the watcher is stopped
        ev_io_stop(mLibEvLoopP, &watch->mIoWatcher);
        ev_io_modify(&watch->mIoWatcher, evs);
        ev_io_start(mLibEvLoopP, &watch->mIoWatcher);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::ClearCallbackOnPendingRead(SocketWatchToken token)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mPendingIO.Clear(SocketEventFlags::kRead);

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    if (ev_is_active(&watch->mIoWatcher) && watch->mPendingIO.Raw() == 0)
    {
        // all flags cleared now, stop watching
        ev_io_stop(mLibEvLoopP, &watch->mIoWatcher);
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::ClearCallbackOnPendingWrite(SocketWatchToken token)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mPendingIO.Clear(SocketEventFlags::kWrite);

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    if (ev_is_active(&watch->mIoWatcher) && watch->mPendingIO.Raw() == 0)
    {
        // all flags cleared now, stop watching
        ev_io_stop(mLibEvLoopP, &watch->mIoWatcher);
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplSelect::StopWatchingSocket(SocketWatchToken * tokenInOut)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(*tokenInOut);
    *tokenInOut         = InvalidSocketWatchToken();

    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(watch->mFD >= 0, CHIP_ERROR_INCORRECT_STATE);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH || CHIP_SYSTEM_CONFIG_USE_LIBEV
    watch->DisableAndClear();
#else
    watch->Clear();

    // Wake the thread calling select so that it stops selecting on the socket.
    Signal();
#endif

    return CHIP_NO_ERROR;
}

/**
 *  Set the read, write or exception bit flags for the specified socket based on its status in
 *  the corresponding file descriptor sets.
 *
 *  @param[in]    socket    The file descriptor for which the bit flags are being set.
 *
 *  @param[in]    readfds   A pointer to the set of readable file descriptors.
 *
 *  @param[in]    writefds  A pointer to the set of writable file descriptors.
 *
 *  @param[in]    exceptfds  A pointer to the set of file descriptors with errors.
 */
SocketEvents LayerImplSelect::SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds,
                                                  const fd_set & exceptfds)
{
    SocketEvents res;

    if (socket >= 0)
    {
        // POSIX does not define the fd_set parameter of FD_ISSET() as const, even though it isn't modified.
        if (FD_ISSET(socket, const_cast<fd_set *>(&readfds)))
            res.Set(SocketEventFlags::kRead);
        if (FD_ISSET(socket, const_cast<fd_set *>(&writefds)))
            res.Set(SocketEventFlags::kWrite);
        if (FD_ISSET(socket, const_cast<fd_set *>(&exceptfds)))
            res.Set(SocketEventFlags::kExcept);
    }

    return res;
}

void LayerImplSelect::PrepareEvents()
{
    assertChipStackLockedByCurrentThread();

    const Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();
    Clock::Timestamp awakenTime        = currentTime + kDefaultMinSleepPeriod;

    TimerList::Node * timer = mTimerList.Earliest();
    if (timer && timer->AwakenTime() < awakenTime)
    {
        awakenTime = timer->AwakenTime();
    }

    const Clock::Timestamp sleepTime = (awakenTime > currentTime) ? (awakenTime - currentTime) : Clock::kZero;
    Clock::ToTimeval(sleepTime, mNextTimeout);

    mMaxFd = -1;

    // NOLINTBEGIN(clang-analyzer-security.insecureAPI.bzero)
    //
    // NOTE: darwin uses bzero to clear out FD sets. This is not a security concern.
    FD_ZERO(&mSelected.mReadSet);
    FD_ZERO(&mSelected.mWriteSet);
    FD_ZERO(&mSelected.mErrorSet);
    // NOLINTEND(clang-analyzer-security.insecureAPI.bzero)

    for (auto & w : mSocketWatchPool)
    {
        if (w.mFD != kInvalidFd)
        {
            if (mMaxFd < w.mFD)
            {
                mMaxFd = w.mFD;
            }
            if (w.mPendingIO.Has(SocketEventFlags::kRead))
            {
                FD_SET(w.mFD, &mSelected.mReadSet);
            }
            if (w.mPendingIO.Has(SocketEventFlags::kWrite))
            {
                FD_SET(w.mFD, &mSelected.mWriteSet);
            }
        }
    }
}

void LayerImplSelect::WaitForEvents()
{
    mSelectResult = select(mMaxFd + 1, &mSelected.mReadSet, &mSelected.mWriteSet, &mSelected.mErrorSet, &mNextTimeout);
}

void LayerImplSelect::HandleEvents()
{
    assertChipStackLockedByCurrentThread();

    if (!IsSelectResultValid())
    {
        ChipLogError(DeviceLayer, "Select failed: %" CHIP_ERROR_FORMAT, CHIP_ERROR_POSIX(errno).Format());
        return;
    }

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
    // since that could result in infinite handling of new timers blocking any other progress.
    VerifyOrDieWithMsg(mExpiredTimers.Empty(), DeviceLayer, "Re-entry into HandleEvents from a timer callback?");
    mExpiredTimers          = mTimerList.ExtractEarlier(Clock::Timeout(1) + SystemClock().GetMonotonicTimestamp());
    TimerList::Node * timer = nullptr;
    while ((timer = mExpiredTimers.PopEarliest()) != nullptr)
    {
        mTimerPool.Invoke(timer);
    }

    for (auto & w : mSocketWatchPool)
    {
        if (w.mFD != kInvalidFd)
        {
            SocketEvents events = SocketEventsFromFDs(w.mFD, mSelected.mReadSet, mSelected.mWriteSet, mSelected.mErrorSet);
            if (events.HasAny() && w.mCallback != nullptr)
            {
                w.mCallback(events, w.mCallbackData);
            }
        }
    }

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH

void LayerImplSelect::HandleTimerComplete(TimerList::Node * timer)
{
    mTimerList.Remove(timer);
    mTimerPool.Invoke(timer);
}

#elif CHIP_SYSTEM_CONFIG_USE_LIBEV

void LayerImplSelect::HandleLibEvTimer(EV_P_ struct ev_timer * t, int revents)
{
    TimerList::Node * timer = static_cast<TimerList::Node *>(t->data);
    VerifyOrDie(timer != nullptr);
    LayerImplSelect * layerP = dynamic_cast<LayerImplSelect *>(timer->mCallback.mSystemLayer);
    VerifyOrDie(layerP != nullptr);
    layerP->mTimerList.Remove(timer);
    layerP->mTimerPool.Invoke(timer);
}

void LayerImplSelect::HandleLibEvIoWatcher(EV_P_ struct ev_io * i, int revents)
{
    SocketWatch * watch = static_cast<SocketWatch *>(i->data);
    if (watch != nullptr && watch->mCallback != nullptr && watch->mLayerImplSelectP != nullptr)
    {
        SocketEvents events;
        if (revents & EV_READ)
        {
            events.Set(SocketEventFlags::kRead);
        }
        if (revents & EV_WRITE)
        {
            events.Set(SocketEventFlags::kWrite);
        }
        if (events.HasAny())
        {
            watch->mCallback(events, watch->mCallbackData);
        }
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

void LayerImplSelect::SocketWatch::Clear()
{
    mFD = kInvalidFd;
    mPendingIO.ClearAll();
    mCallback     = nullptr;
    mCallbackData = 0;
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    mRdSource = nullptr;
    mWrSource = nullptr;
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    mLayerImplSelectP = nullptr;
#endif
}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
void LayerImplSelect::SocketWatch::DisableAndClear()
{
    if (mRdSource)
    {
        dispatch_source_cancel(mRdSource);
        dispatch_release(mRdSource);
    }
    if (mWrSource)
    {
        dispatch_source_cancel(mWrSource);
        dispatch_release(mWrSource);
    }
    Clear();
}
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
void LayerImplSelect::SocketWatch::DisableAndClear()
{
    if (mLayerImplSelectP != nullptr && mLayerImplSelectP->mLibEvLoopP != nullptr)
    {
        ev_io_stop(mLayerImplSelectP->mLibEvLoopP, &mIoWatcher);
    }
    Clear();
}
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

} // namespace System
} // namespace chip
