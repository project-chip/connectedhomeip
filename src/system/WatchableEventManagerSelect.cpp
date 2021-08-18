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
 *      This file implements WatchableEventManager using select().
 */

#include <platform/LockTracker.h>
#include <support/CodeUtils.h>
#include <support/TimeUtils.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/WatchableEventManager.h>
#include <system/WatchableSocket.h>

#include <errno.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)
#define PTHREAD_NULL 0
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

namespace chip {
namespace Mdns {
void GetMdnsTimeout(timeval & timeout);
void HandleMdnsTimeout();
} // namespace Mdns
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

namespace chip {
namespace System {

CHIP_ERROR WatchableEventManager::Init(Layer & systemLayer)
{
    RegisterPOSIXErrorFormatter();

    mSystemLayer = &systemLayer;
    mMaxFd       = -1;
    FD_ZERO(&mRequest.mReadSet);
    FD_ZERO(&mRequest.mWriteSet);
    FD_ZERO(&mRequest.mErrorSet);

    ReturnErrorOnFailure(mTimerList.Init());

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Create an event to allow an arbitrary thread to wake the thread in the select loop.
    return mWakeEvent.Open(*this);
}

CHIP_ERROR WatchableEventManager::Shutdown()
{
    Timer * timer;
    while ((timer = mTimerList.PopEarliest()) != nullptr)
    {
        timer->Clear();

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
        if (timer->mTimerSource != nullptr)
        {
            dispatch_source_cancel(timer->mTimerSource);
            dispatch_release(timer->mTimerSource);
        }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

        timer->Release();
    }
    mWakeEvent.Close();
    mSystemLayer = nullptr;
    return CHIP_NO_ERROR;
}

void WatchableEventManager::Signal()
{
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
}

CHIP_ERROR WatchableEventManager::StartTimer(uint32_t delayMilliseconds, Timers::OnCompleteFunct onComplete, void * appState)
{
    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delayMilliseconds = 0);

    CancelTimer(onComplete, appState);

    Timer * timer = Timer::New(*mSystemLayer, delayMilliseconds, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = GetDispatchQueue();
    if (dispatchQueue)
    {
        (void) mTimerList.Add(timer);
        dispatch_source_t timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatchQueue);
        if (timerSource == nullptr)
        {
            chipDie();
        }

        timer->mTimerSource = timerSource;
        dispatch_source_set_timer(timerSource, dispatch_walltime(NULL, delayMilliseconds * NSEC_PER_MSEC), 0, 100 * NSEC_PER_MSEC);
        dispatch_source_set_event_handler(timerSource, ^{
            dispatch_source_cancel(timerSource);
            dispatch_release(timerSource);

            this->HandleTimerComplete(timer);
        });
        dispatch_resume(timerSource);
        return CHIP_NO_ERROR;
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        Signal();
    }
    return CHIP_NO_ERROR;
}

void WatchableEventManager::CancelTimer(Timers::OnCompleteFunct onComplete, void * appState)
{
    Timer * timer = mTimerList.Remove(onComplete, appState);
    VerifyOrReturn(timer != nullptr);

    timer->Clear();

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (timer->mTimerSource != nullptr)
    {
        dispatch_source_cancel(timer->mTimerSource);
        dispatch_release(timer->mTimerSource);
    }
#endif

    timer->Release();
    Signal();
}

CHIP_ERROR WatchableEventManager::ScheduleWork(Timers::OnCompleteFunct onComplete, void * appState)
{
    CancelTimer(onComplete, appState);

    Timer * timer = Timer::New(*mSystemLayer, 0, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = GetDispatchQueue();
    if (dispatchQueue)
    {
        (void) mTimerList.Add(timer);
        dispatch_async(dispatchQueue, ^{
            this->HandleTimerComplete(timer);
        });
        return CHIP_NO_ERROR;
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        Signal();
    }
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
SocketEvents WatchableEventManager::SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds,
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

bool WatchableEventManager::HasAnyRequest(int fd)
{
    return FD_ISSET(fd, &mRequest.mReadSet) || FD_ISSET(fd, &mRequest.mWriteSet) || FD_ISSET(fd, &mRequest.mErrorSet);
}

CHIP_ERROR WatchableEventManager::SetRequest(int fd, fd_set * fds)
{
    FD_SET(fd, fds);
    if (fd > mMaxFd)
    {
        mMaxFd = fd;
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    Signal();
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableEventManager::ClearRequest(int fd, fd_set * fds)
{
    FD_CLR(fd, fds);
    if (fd == mMaxFd)
    {
        MaybeLowerMaxFd();
    }
    // Wake the thread calling select so that it starts selecting on the new socket.
    Signal();
    return CHIP_NO_ERROR;
}

void WatchableEventManager::ResetRequests(int fd)
{
    FD_CLR(fd, &mRequest.mReadSet);
    FD_CLR(fd, &mRequest.mWriteSet);
    FD_CLR(fd, &mRequest.mErrorSet);
    if (fd == mMaxFd)
    {
        MaybeLowerMaxFd();
    }
}

void WatchableEventManager::MaybeLowerMaxFd()
{
    int fd;
    for (fd = mMaxFd; fd >= 0; --fd)
    {
        if (HasAnyRequest(fd))
        {
            break;
        }
    }
    mMaxFd = fd;
}

void WatchableEventManager::PrepareEvents()
{
    assertChipStackLockedByCurrentThread();

    constexpr Clock::MonotonicMilliseconds kMaxTimeout =
        static_cast<Clock::MonotonicMilliseconds>(DEFAULT_MIN_SLEEP_PERIOD) * kMillisecondsPerSecond;
    const Clock::MonotonicMilliseconds currentTime = Clock::GetMonotonicMilliseconds();
    Clock::MonotonicMilliseconds awakenTime        = currentTime + kMaxTimeout;

    Timer * timer = mTimerList.Earliest();
    if (timer && Clock::IsEarlier(timer->mAwakenTime, awakenTime))
    {
        awakenTime = timer->mAwakenTime;
    }

    const Clock::MonotonicMilliseconds sleepTime = (awakenTime > currentTime) ? (awakenTime - currentTime) : 0;
    MillisecondsToTimeval(sleepTime, mNextTimeout);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    chip::Mdns::GetMdnsTimeout(mNextTimeout);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

    mSelected = mRequest;
}

void WatchableEventManager::WaitForEvents()
{
    mSelectResult = select(mMaxFd + 1, &mSelected.mReadSet, &mSelected.mWriteSet, &mSelected.mErrorSet, &mNextTimeout);
}

void WatchableEventManager::HandleEvents()
{
    assertChipStackLockedByCurrentThread();

    if (mSelectResult < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    VerifyOrDie(mSystemLayer != nullptr);

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
    // since that could result in infinite handling of new timers blocking any other progress.
    Timer::List expiredTimers(mTimerList.ExtractEarlier(1 + Clock::GetMonotonicMilliseconds()));
    Timer * timer = nullptr;
    while ((timer = expiredTimers.PopEarliest()) != nullptr)
    {
        timer->HandleComplete();
    }

    for (WatchableSocket * watchable = mAttachedSockets; watchable != nullptr; watchable = watchable->mAttachedNext)
    {
        watchable->SetPendingIO(
            SocketEventsFromFDs(watchable->GetFD(), mSelected.mReadSet, mSelected.mWriteSet, mSelected.mErrorSet));
    }

    WatchableSocket * nextWatchableSocket = mAttachedSockets;
    while (nextWatchableSocket != nullptr)
    {
        WatchableSocket * currentWatchable = nextWatchableSocket;
        nextWatchableSocket                = nextWatchableSocket->mAttachedNext;

        if (currentWatchable->mPendingIO.HasAny())
        {
            currentWatchable->InvokeCallback();
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    chip::Mdns::HandleMdnsTimeout();
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
void WatchableEventManager::HandleTimerComplete(Timer * timer)
{
    mTimerList.Remove(timer);
    timer->HandleComplete();
}
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

} // namespace System
} // namespace chip
