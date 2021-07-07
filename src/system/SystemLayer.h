/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file contains declarations of the
 *      chip::System::Layer class and its related types, data and
 *      functions.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

#include <core/CHIPCallback.h>

#include <support/DLLUtil.h>
#include <system/SystemError.h>
#include <system/SystemEvent.h>
#include <system/SystemObject.h>

// Include dependent headers
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SystemSockets.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

namespace chip {
namespace System {

class Layer;
class Timer;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
class Object;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace Platform {
namespace Layer {

using ::chip::System::Layer;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
using ::chip::System::Object;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

extern CHIP_ERROR WillInit(Layer & aLayer, void * aContext);
extern CHIP_ERROR WillShutdown(Layer & aLayer, void * aContext);

extern void DidInit(Layer & aLayer, void * aContext, CHIP_ERROR aStatus);
extern void DidShutdown(Layer & aLayer, void * aContext, CHIP_ERROR aStatus);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
extern CHIP_ERROR PostEvent(Layer & aLayer, void * aContext, Object & aTarget, EventType aType, uintptr_t aArgument);
extern CHIP_ERROR DispatchEvents(Layer & aLayer, void * aContext);
extern CHIP_ERROR DispatchEvent(Layer & aLayer, void * aContext, Event aEvent);
extern CHIP_ERROR StartTimer(Layer & aLayer, void * aContext, uint32_t aMilliseconds);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace Layer
} // namespace Platform

/**
 *  @enum LayerState
 *
 *  The state of a Layer object.
 */
enum LayerState
{
    kLayerState_NotInitialized = 0, /**< Not initialized state. */
    kLayerState_Initialized    = 1  /**< Initialized state. */
};

#if CHIP_SYSTEM_CONFIG_USE_LWIP
typedef CHIP_ERROR (*LwIPEventHandlerFunction)(Object & aTarget, EventType aEventType, uintptr_t aArgument);

class LwIPEventHandlerDelegate
{
    friend class Layer;

public:
    bool IsInitialized(void) const;
    void Init(LwIPEventHandlerFunction aFunction);
    void Prepend(const LwIPEventHandlerDelegate *& aDelegateList);

private:
    LwIPEventHandlerFunction mFunction;
    const LwIPEventHandlerDelegate * mNextDelegate;
};
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  @class Layer
 *
 *  @brief
 *      This provides access to timers according to the configured event handling model.
 *
 *      For \c CHIP_SYSTEM_CONFIG_USE_SOCKETS, event readiness notification is handled via WatchableEventManager.
 *
 *      For \c CHIP_SYSTEM_CONFIG_USE_LWIP, event readiness notification is handle via events / messages and platform- and
 *      system-specific hooks for the event/message system.
 */
class DLL_EXPORT Layer
{
public:
    Layer();

    CHIP_ERROR Init(void * aContext);

    // Some other layers hold pointers to System::Layer, so care must be taken
    // to ensure that they are not used after calling Shutdown().
    CHIP_ERROR Shutdown();

    void * GetPlatformData() const;
    void SetPlatformData(void * aPlatformData);

    LayerState State() const;

    CHIP_ERROR NewTimer(Timer *& aTimerPtr);

    void StartTimer(uint32_t aMilliseconds, chip::Callback::Callback<> * aCallback);
    void DispatchTimerCallbacks(uint64_t kCurrentEpoch);

    typedef void (*TimerCompleteFunct)(Layer * aLayer, void * aAppState, CHIP_ERROR aError);
    CHIP_ERROR StartTimer(uint32_t aMilliseconds, TimerCompleteFunct aComplete, void * aAppState);
    void CancelTimer(TimerCompleteFunct aOnComplete, void * aAppState);

    CHIP_ERROR ScheduleWork(TimerCompleteFunct aComplete, void * aAppState);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    WatchableEventManager & WatchableEvents() { return mWatchableEvents; }
    bool GetTimeout(struct timeval & aSleepTime); // TODO(#5556): Integrate timer platform details with WatchableEventManager.
    void HandleTimeout();                         // TODO(#5556): Integrate timer platform details with WatchableEventManager.
#endif                                            // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if CHIP_SYSTEM_CONFIG_USE_IO_THREAD
    void WakeIOThread();
#endif // CHIP_SYSTEM_CONFIG_USE_IO_THREAD

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    typedef CHIP_ERROR (*EventHandler)(Object & aTarget, EventType aEventType, uintptr_t aArgument);
    CHIP_ERROR AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate);

    // Event Handling
    CHIP_ERROR PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);
    CHIP_ERROR DispatchEvents(void);
    CHIP_ERROR DispatchEvent(Event aEvent);
    CHIP_ERROR HandleEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);

    // Timer Management
    CHIP_ERROR HandlePlatformTimer(void);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void SetDispatchQueue(dispatch_queue_t dispatchQueue) { mDispatchQueue = dispatchQueue; };
    dispatch_queue_t GetDispatchQueue() { return mDispatchQueue; };
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    static uint64_t GetClock_Monotonic();
    static uint64_t GetClock_MonotonicMS();
    static uint64_t GetClock_MonotonicHiRes();
    static CHIP_ERROR GetClock_RealTime(uint64_t & curTime);
    static CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTimeMS);
    static CHIP_ERROR SetClock_RealTime(uint64_t newCurTime);

private:
    LayerState mLayerState;
    void * mContext;
    void * mPlatformData;
    chip::Callback::CallbackDeque mTimerCallbacks;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static LwIPEventHandlerDelegate sSystemEventHandlerDelegate;

    const LwIPEventHandlerDelegate * mEventDelegateList;
    Timer * mTimerList;
    bool mTimerComplete;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    WatchableEventManager mWatchableEvents;
    WakeEvent mWakeEvent;
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static CHIP_ERROR HandleSystemLayerEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);

    CHIP_ERROR StartPlatformTimer(uint32_t aDelayMilliseconds);

    friend CHIP_ERROR Platform::Layer::PostEvent(Layer & aLayer, void * aContext, Object & aTarget, EventType aType,
                                                 uintptr_t aArgument);
    friend CHIP_ERROR Platform::Layer::DispatchEvents(Layer & aLayer, void * aContext);
    friend CHIP_ERROR Platform::Layer::DispatchEvent(Layer & aLayer, void * aContext, Event aEvent);
    friend CHIP_ERROR Platform::Layer::StartTimer(Layer & aLayer, void * aContext, uint32_t aMilliseconds);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t mDispatchQueue;
#endif

    // Copy and assignment NOT DEFINED
    Layer(const Layer &) = delete;
    Layer & operator=(const Layer &) = delete;

    friend class Timer;
};

/**
 * This returns the current state of the layer object.
 */
inline LayerState Layer::State() const
{
    return this->mLayerState;
}

} // namespace System
} // namespace chip
