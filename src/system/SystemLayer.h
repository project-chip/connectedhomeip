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

#include <lib/core/CHIPCallback.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemEvent.h>
#include <system/SystemObject.h>
#include <system/SystemTimer.h>
#include <system/WatchableEventManager.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SocketEvents.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

namespace chip {
namespace System {

using TimerCompleteCallback = void (*)(Layer * aLayer, void * appState);

/**
 * This provides access to timers according to the configured event handling model.
 */
class DLL_EXPORT Layer
{
public:
    Layer() = default;
    ~Layer();

    CHIP_ERROR Init();

    // Some other layers hold pointers to System::Layer, so care must be taken
    // to ensure that they are not used after calling Shutdown().
    CHIP_ERROR Shutdown();

    bool IsInitialized() const { return mLayerState.IsInitialized(); }

    /**
     * @brief
     *   This method starts a one-shot timer.
     *
     *   @note
     *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
     *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
     *       the currently-running timer will first be cancelled.
     *
     *   @param[in]  aDelayMilliseconds Time in milliseconds before this timer fires.
     *   @param[in]  aComplete          A pointer to the function called when timer expires.
     *   @param[in]  aAppState          A pointer to the application state object used when timer expires.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
     *   @return Other Value indicating timer failed to start.
     */
    CHIP_ERROR StartTimer(uint32_t aDelayMilliseconds, TimerCompleteCallback aComplete, void * aAppState);

    /**
     * @brief
     *   This method cancels a one-shot timer, started earlier through @p StartTimer().
     *
     *   @note
     *       The cancellation could fail silently in two different ways. If the timer specified by the combination of the callback
     *       function and application state object couldn't be found, cancellation could fail. If the timer has fired, but not yet
     *       removed from memory, cancellation could also fail.
     *
     *   @param[in]  aOnComplete   A pointer to the callback function used in calling @p StartTimer().
     *   @param[in]  aAppState     A pointer to the application state object used in calling @p StartTimer().
     *
     */
    void CancelTimer(TimerCompleteCallback aOnComplete, void * aAppState);

    /**
     * @brief
     *   Schedules a function with a signature identical to `OnCompleteFunct` to be run as soon as possible in the CHIP context.
     *
     * @note
     *   This function could, in principle, be implemented as `StartTimer`. The specification for `SystemTimer` however
     *   permits certain optimizations that might make that implementation impossible. Specifically, `SystemTimer`
     *   API may only be called from the thread owning the particular `SystemLayer`, whereas the `ScheduleWork` may be
     *   called from any thread. Additionally, whereas the `SystemTimer` API permits the invocation of the already
     *   expired handler in line, `ScheduleWork` guarantees that the handler function will be called only after the
     *   current CHIP event completes.
     *
     * @param[in] aComplete A pointer to a callback function to be called
     *                      when this timer fires.
     *
     * @param[in] aAppState A pointer to an application state object to be
     *                      passed to the callback function as argument.
     *
     * @retval CHIP_ERROR_INCORRECT_STATE If the SystemLayer has
     *                      not been initialized.
     *
     * @retval CHIP_ERROR_NO_MEMORY If the SystemLayer cannot
     *                      allocate a new timer.
     *
     * @retval CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ScheduleWork(TimerCompleteCallback aComplete, void * aAppState);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    /**
     * Initialize watching for events on a file descriptor.
     *
     * Returns an opaque token through @a tokenOut that must be passed to subsequent operations for this file descriptor.
     * StopWatchingSocket() must be called before closing the file descriptor.
     */
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut);

    /**
     * Register a callback function.
     *
     * The callback will be invoked (with the CHIP stack lock held) when requested event(s) are ready.
     */
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data);

    /**
     * Request a callback when the associated file descriptor is readable.
     */
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token);

    /**
     * Request a callback when the associated file descriptor is writable.
     */
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token);

    /**
     * Cancel a request for a callback when the associated file descriptor is readable.
     */
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token);

    /**
     * Cancel a request for a callback when the associated file descriptor is writable.
     */
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token);

    /**
     * Stop watching for events on the associated file descriptor.
     *
     * This MUST be called before the file descriptor is closed.
     * It is not necessary to clear callback requests before calling this function.
     */
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut);

    /**
     * Return a SocketWatchToken that is guaranteed not to be valid. Clients may use this to initialize variables.
     */
    SocketWatchToken InvalidSocketWatchToken();

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void SetDispatchQueue(dispatch_queue_t dispatchQueue);
    dispatch_queue_t GetDispatchQueue();
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    /**
     * This adds an event handler delegate to the system layer to extend its ability to handle LwIP events.
     *
     *  @param[in]  aDelegate   An uninitialied LwIP event handler delegate structure
     *
     *  @retval     CHIP_NO_ERROR                 On success.
     *  @retval     CHIP_ERROR_INVALID_ARGUMENT   If the function pointer contained in aDelegate is NULL
     */
    CHIP_ERROR AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate);

    /**
     * This posts an event / message of the specified type with the provided argument to this instance's platform-specific event
     * queue.
     *
     *  @param[in,out]  aTarget     A pointer to the CHIP System Layer object making the post request.
     *  @param[in]      aEventType  The type of event to post.
     *  @param[in,out]  aArgument   The argument associated with the event to post.
     *
     *  @retval    CHIP_NO_ERROR                  On success.
     *  @retval    CHIP_ERROR_INCORRECT_STATE     If the state of the Layer object is incorrect.
     *  @retval    CHIP_ERROR_NO_MEMORY           If the event queue is already full.
     *  @retval    other Platform-specific errors generated indicating the reason for failure.
     */
    CHIP_ERROR PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    WatchableEventManager & WatchableEventsManager() { return mWatchableEventsManager; }
    Clock & GetClock() { return mClock; }

private:
    ObjectLifeCycle mLayerState;
    WatchableEventManager mWatchableEventsManager;
    Clock mClock;

    // Copy and assignment NOT DEFINED
    Layer(const Layer &) = delete;
    Layer & operator=(const Layer &) = delete;
};

} // namespace System
} // namespace chip
