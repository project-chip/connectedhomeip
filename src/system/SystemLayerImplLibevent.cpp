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
 *      This file implements System::Layer using libevent.
 */

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceBuildConfig.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplLibevent.h>

#include <algorithm>
#include <fcntl.h>
#include <unistd.h>

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

} // anonymous namespace

CHIP_ERROR LayerImplLibevent::Init(System::Layer & systemLayer)
{
    VerifyOrReturnError(!mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    RegisterPOSIXErrorFormatter();

#if CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS
    static bool enabled_event_debug_mode = false;
    if (!enabled_event_debug_mode)
    {
        enabled_event_debug_mode = true;
        event_enable_debug_mode();
    }
#endif // CHIP_CONFIG_LIBEVENT_DEBUG_CHECKS

    mSystemLayer = &systemLayer;
    mEventBase   = event_base_new();
    VerifyOrReturnError(mEventBase != nullptr, CHIP_ERROR_NO_MEMORY);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
    mMdnsTimeoutEvent = evtimer_new(mEventBase, MdnsTimeoutCallbackHandler, this);
    VerifyOrReturnError(mMdnsTimeoutEvent != nullptr, CHIP_ERROR_NO_MEMORY);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    Mutex::Init(mTimerListMutex);

    VerifyOrReturnError(mLayerState.Init(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

// static
void LayerImplLibevent::MdnsTimeoutCallbackHandler(evutil_socket_t fd, short eventFlags, void * data)
{
    reinterpret_cast<LayerImplLibevent *>(data)->MdnsTimeoutCallbackHandler();
}

void LayerImplLibevent::MdnsTimeoutCallbackHandler()
{
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    chip::Mdns::HandleMdnsTimeout();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

CHIP_ERROR LayerImplLibevent::Shutdown()
{
    VerifyOrReturnError(mLayerState.Shutdown(), CHIP_ERROR_INCORRECT_STATE);

    event_base_loopbreak(mEventBase);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
    if (mMdnsTimeoutEvent != nullptr)
    {
        event_free(mMdnsTimeoutEvent);
        mMdnsTimeoutEvent = nullptr;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__

    mTimerListMutex.Lock();
    mTimers.clear();
    mTimerListMutex.Unlock();

    mSocketWatches.clear();

    event_base_free(mEventBase);
    mEventBase   = nullptr;
    mSystemLayer = nullptr;

    mLayerState.Reset(); // Return to uninitialized state to permit re-initialization.
    return CHIP_NO_ERROR;
}

void LayerImplLibevent::Signal()
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

CHIP_ERROR LayerImplLibevent::StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    std::lock_guard<Mutex> lock(mTimerListMutex);
    mTimers.push_back(std::make_unique<LibeventTimer>(this, onComplete, appState));
    LibeventTimer * timer = mTimers.back().get();
    if (timer == nullptr)
    {
        mTimers.pop_back();
        return CHIP_ERROR_NO_MEMORY;
    }

    event * e = evtimer_new(mEventBase, TimerCallbackHandler, timer);
    VerifyOrReturnError(e != nullptr, CHIP_ERROR_NO_MEMORY);
    timer->mEvent = e;

    timeval delay;
    MillisecondsToTimeval(delayMilliseconds, delay);
    int status = evtimer_add(e, &delay);
    VerifyOrReturnError(status == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void LayerImplLibevent::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    std::lock_guard<Mutex> lock(mTimerListMutex);
    auto it = std::find_if(mTimers.begin(), mTimers.end(), [onComplete, appState](const std::unique_ptr<LibeventTimer> & timer) {
        return timer->mOnComplete == onComplete && timer->mCallbackData == appState;
    });
    if (it != mTimers.end())
    {
        LibeventTimer * timer = it->get();
        mActiveTimers.remove(timer);
        mTimers.remove_if([timer](const std::unique_ptr<LibeventTimer> & t) { return t.get() == timer; });
    }
}

CHIP_ERROR LayerImplLibevent::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    return StartTimer(0, onComplete, appState);
}

// static
void LayerImplLibevent::TimerCallbackHandler(evutil_socket_t fd, short eventFlags, void * data)
{
    // Copy the necessary timer information and remove it from the list.
    LibeventTimer * timer            = reinterpret_cast<LibeventTimer *>(data);
    Layer * systemLayer              = timer->mEventManager->mSystemLayer;
    TimerCompleteCallback onComplete = timer->mOnComplete;
    void * callbackData              = timer->mCallbackData;
    systemLayer->CancelTimer(onComplete, callbackData);
    if (onComplete)
    {
        onComplete(systemLayer, callbackData);
    }
}

LayerImplLibevent::LibeventTimer::~LibeventTimer()
{
    mEventManager = nullptr;
    mOnComplete   = nullptr;
    mCallbackData = nullptr;
    if (mEvent)
    {
        if (evtimer_pending(mEvent, nullptr))
        {
            event_del(mEvent);
        }
        event_free(mEvent);
        mEvent = nullptr;
    }
};

CHIP_ERROR LayerImplLibevent::StartWatchingSocket(int fd, SocketWatchToken * tokenOut)
{
    mSocketWatches.push_back(std::make_unique<SocketWatch>(this, fd));
    SocketWatch * watch = mSocketWatches.back().get();
    if (watch == nullptr)
    {
        mSocketWatches.pop_back();
        return CHIP_ERROR_NO_MEMORY;
    }

    *tokenOut = reinterpret_cast<SocketWatchToken>(watch);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLibevent::SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    watch->mCallback     = callback;
    watch->mCallbackData = data;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLibevent::RequestCallbackOnPendingRead(SocketWatchToken token)
{
    return SetWatch(token, EV_READ);
}

CHIP_ERROR LayerImplLibevent::RequestCallbackOnPendingWrite(SocketWatchToken token)
{
    return SetWatch(token, EV_WRITE);
}

CHIP_ERROR LayerImplLibevent::ClearCallbackOnPendingRead(SocketWatchToken token)
{
    return ClearWatch(token, EV_READ);
}

CHIP_ERROR LayerImplLibevent::ClearCallbackOnPendingWrite(SocketWatchToken token)
{
    return ClearWatch(token, EV_WRITE);
}

CHIP_ERROR LayerImplLibevent::StopWatchingSocket(SocketWatchToken * tokenInOut)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(*tokenInOut);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    *tokenInOut = InvalidSocketWatchToken();

    mActiveSocketWatches.remove(watch);
    mSocketWatches.remove_if([watch](const std::unique_ptr<SocketWatch> & w) { return w.get() == watch; });
    return CHIP_NO_ERROR;
}

SocketWatchToken InvalidSocketWatchToken()
{
    return reinterpret_cast<SocketWatchToken>(nullptr);
}

CHIP_ERROR LayerImplLibevent::SetWatch(SocketWatchToken token, short eventFlags)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const short oldFlags = watch->mEvent ? event_get_events(watch->mEvent) : 0;
    return UpdateWatch(watch, static_cast<short>(EV_PERSIST | oldFlags | eventFlags));
}

CHIP_ERROR LayerImplLibevent::ClearWatch(SocketWatchToken token, short eventFlags)
{
    SocketWatch * watch = reinterpret_cast<SocketWatch *>(token);
    VerifyOrReturnError(watch != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const short oldFlags = watch->mEvent ? event_get_events(watch->mEvent) : 0;
    return UpdateWatch(watch, static_cast<short>(EV_PERSIST | (oldFlags & ~eventFlags)));
}

CHIP_ERROR LayerImplLibevent::UpdateWatch(SocketWatch * watch, short eventFlags)
{
    if (watch->mEvent != nullptr)
    {
        if (event_get_events(watch->mEvent) == eventFlags)
        {
            // No update needed.
            return CHIP_NO_ERROR;
        }
        if (event_pending(watch->mEvent, EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL, nullptr))
        {
            event_del(watch->mEvent);
        }
        event_free(watch->mEvent);
        watch->mEvent = nullptr;
    }

    if (eventFlags)
    {
        // libevent requires the socket to already be non-blocking.
        int flags = ::fcntl(watch->mFD, F_GETFL, 0);
        if ((flags & O_NONBLOCK) == 0)
        {
            int status = ::fcntl(watch->mFD, F_SETFL, flags | O_NONBLOCK);
            VerifyOrReturnError(status == 0, chip::System::MapErrorPOSIX(errno));
        }
        watch->mEvent = event_new(mEventBase, watch->mFD, eventFlags, SocketCallbackHandler, watch);
        VerifyOrReturnError(watch->mEvent != nullptr, CHIP_ERROR_NO_MEMORY);
        int status = event_add(watch->mEvent, nullptr);
        VerifyOrReturnError(status == 0, CHIP_ERROR_INTERNAL);
    }

    return CHIP_NO_ERROR;
}

// static
void LayerImplLibevent::SocketCallbackHandler(evutil_socket_t fd, short eventFlags, void * data)
{
    SocketWatch * const watch = reinterpret_cast<SocketWatch *>(data);
    VerifyOrDie(watch != nullptr);
    VerifyOrDie(watch->mFD == fd);

    watch->mPendingIO = SocketEventsFromLibeventFlags(eventFlags);
    watch->mEventManager->mActiveSocketWatches.push_back(watch);
}

LayerImplLibevent::SocketWatch::~SocketWatch()
{
    mEventManager = nullptr;
    mFD           = kInvalidFd;
    mCallback     = nullptr;
    mCallbackData = 0;
    if (mEvent)
    {
        if (event_pending(mEvent, EV_TIMEOUT | EV_READ | EV_WRITE | EV_SIGNAL, nullptr))
        {
            event_del(mEvent);
        }
        event_free(mEvent);
        mEvent = nullptr;
    }
}

void LayerImplLibevent::PrepareEvents()
{
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__ && !__MBED__
    timeval mdnsTimeout = { 0, 0 };
    chip::Mdns::GetMdnsTimeout(mdnsTimeout);
    if (mdnsTimeout.tv_sec || mdnsTimeout.tv_usec)
    {
        evtimer_add(mMdnsTimeoutEvent, &mdnsTimeout);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
}

void LayerImplLibevent::WaitForEvents()
{
    VerifyOrDie(mEventBase != nullptr);
    event_base_loop(mEventBase, EVLOOP_ONCE);
}

void LayerImplLibevent::HandleEvents()
{
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    while (!mActiveSocketWatches.empty())
    {
        SocketWatch * const watch = mActiveSocketWatches.front();
        mActiveSocketWatches.pop_front();
        if (watch->mPendingIO.HasAny() && watch->mCallback != nullptr)
        {
            watch->mCallback(watch->mPendingIO, watch->mCallbackData);
        }
    }

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

} // namespace System
} // namespace chip
