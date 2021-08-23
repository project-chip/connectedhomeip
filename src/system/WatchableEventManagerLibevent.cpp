/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements WatchableEventManager using libevent.
 */

#include <platform/CHIPDeviceBuildConfig.h>
#include <support/CodeUtils.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/WatchableEventManager.h>
#include <system/WatchableSocket.h>

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

namespace chip {
namespace Mdns {
void GetMdnsTimeout(timeval & timeout);
void HandleMdnsTimeout();
} // namespace Mdns
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

#ifndef CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS
#define CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS 1
#endif

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)
#define PTHREAD_NULL 0
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING && !defined(PTHREAD_NULL)

namespace chip {
namespace System {

namespace {

System::SocketEvents SocketEventsFromLibeventFlags(short eventFlags)
{
    return System::SocketEvents()
        .Set(SocketEventFlags::kRead, eventFlags & EV_READ)
        .Set(SocketEventFlags::kWrite, eventFlags & EV_WRITE);
}

void TimeoutCallbackHandler(evutil_socket_t fd, short eventFlags, void * data) {}

} // anonymous namespace

CHIP_ERROR WatchableEventManager::Init(System::Layer & systemLayer)
{
    RegisterPOSIXErrorFormatter();

#if CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS
    static bool enabled_event_debug_mode = false;
    if (!enabled_event_debug_mode)
    {
        enabled_event_debug_mode = true;
        event_enable_debug_mode();
    }
#endif // CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS

    mEventBase     = event_base_new();
    mTimeoutEvent  = evtimer_new(mEventBase, TimeoutCallbackHandler, event_self_cbarg());
    mActiveSockets = nullptr;
    mSystemLayer   = &systemLayer;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    return mTimerList.Init();
}

void WatchableEventManager::PrepareEvents()
{
    const Clock::MonotonicMilliseconds currentTime = Clock::GetMonotonicMilliseconds();
    Clock::MonotonicMilliseconds awakenTime        = currentTime;

    Timer * timer = mTimerList.Earliest();
    if (timer && Clock::IsEarlier(timer->mAwakenTime, awakenTime))
    {
        awakenTime = timer->mAwakenTime;
    }

    const Clock::MonotonicMilliseconds sleepTime = (awakenTime > currentTime) ? (awakenTime - currentTime) : 0;
    timeval nextTimeout;
    MillisecondsToTimeval(sleepTime, nextTimeout);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    chip::Mdns::GetMdnsTimeout(nextTimeout);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

    if (nextTimeout.tv_sec || nextTimeout.tv_usec)
    {
        evtimer_add(mTimeoutEvent, &nextTimeout);
    }
}

void WatchableEventManager::WaitForEvents()
{
    VerifyOrDie(mEventBase != nullptr);
    event_base_loop(mEventBase, EVLOOP_ONCE);
}

void WatchableEventManager::HandleEvents()
{
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

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
    chip::Mdns::HandleMdnsTimeout();
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

    while (mActiveSockets != nullptr)
    {
        WatchableSocket * const watcher = mActiveSockets;
        mActiveSockets                  = watcher->mActiveNext;
        watcher->InvokeCallback();
    }

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

CHIP_ERROR WatchableEventManager::Shutdown()
{
    event_base_loopbreak(mEventBase);
    event_free(mTimeoutEvent);
    mTimeoutEvent = nullptr;
    event_base_free(mEventBase);
    mEventBase = nullptr;

    Timer * timer;
    while ((timer = mTimerList.PopEarliest()) != nullptr)
    {
        timer->Clear();
        timer->Release();
    }

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
    // Note: the libevent implementation currently uses a single libevent timer, playing the same role as the select() timeout.
    // A more ‘native' implementation would have Timer contain a libevent timer and callback data for each CHIP timer.
    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delayMilliseconds = 0);

    CancelTimer(onComplete, appState);

    Timer * timer = Timer::New(*mSystemLayer, delayMilliseconds, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

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
    timer->Release();
    Signal();
}

// static
void WatchableEventManager::LibeventCallbackHandler(evutil_socket_t fd, short eventFlags, void * data)
{
    WatchableSocket * const watcher = reinterpret_cast<WatchableSocket *>(data);
    VerifyOrDie(watcher != nullptr);
    VerifyOrDie(watcher->mFD == fd);

    watcher->mPendingIO = SocketEventsFromLibeventFlags(eventFlags);

    // Add to active list.
    WatchableSocket ** pp = &watcher->mSharedState->mActiveSockets;
    while (*pp != nullptr)
    {
        if (*pp == watcher)
        {
            return;
        }
        pp = &(*pp)->mActiveNext;
    }
    *pp                  = watcher;
    watcher->mActiveNext = nullptr;
}

void WatchableEventManager::RemoveFromQueueIfPresent(WatchableSocket * watcher)
{
    VerifyOrDie(watcher != nullptr);
    VerifyOrDie(watcher->mSharedState == this);

    WatchableSocket ** pp = &mActiveSockets;
    while (*pp != nullptr)
    {
        if (*pp == watcher)
        {
            *pp = watcher->mActiveNext;
            return;
        }
        pp = &(*pp)->mActiveNext;
    }
}

} // namespace System
} // namespace chip
