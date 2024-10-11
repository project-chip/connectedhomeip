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

#include <type_traits>
#include <utility>

// Include configuration headers
#include <system/SystemConfig.h>

#include <lib/core/CHIPCallback.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/LambdaBridge.h>
#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemEvent.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <lib/support/IntrusiveList.h>
#include <system/SocketEvents.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
#include <ev.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

namespace chip {
namespace System {

class Layer;
using TimerCompleteCallback = void (*)(Layer * aLayer, void * appState);

/**
 * This provides access to timers according to the configured event handling model.
 *
 * The abstract class hierarchy is:
 * - Layer: Core timer methods.
 *   - LayerFreeRTOS: Adds methods specific to CHIP_SYSTEM_CONFIG_USING_LWIP and CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT.
 *   - LayerSockets: Adds I/O event methods specific to CHIP_SYSTEM_CONFIG_USING_SOCKETS.
 *     - LayerSocketsLoop: Adds methods for event-loop-based implementations.
 *
 * Threading notes:
 *
 * The SDK is not generally thread safe. System::Layer methods should only be called from
 * a single context, or otherwise externally synchronized. For platforms that use a CHIP
 * event loop thread, timer callbacks are invoked on that thread; for platforms that use
 * a CHIP lock, the lock is held.
 */
class DLL_EXPORT Layer
{
public:
    Layer()          = default;
    virtual ~Layer() = default;

    /**
     * Initialize the Layer.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * Shut down the Layer.
     *
     * Some other layers hold pointers to System::Layer, so care must be taken
     * to ensure that they are not used after calling Shutdown().
     */
    virtual void Shutdown() = 0;

    /**
     * True if this Layer is initialized. No method on Layer or its abstract descendants, other than this and `Init()`,
     * may be called from general code unless this is true. (Individual Impls may have looser constraints internally.)
     */
    virtual bool IsInitialized() const = 0;

    /**
     * @brief
     *   This method starts a one-shot timer.  This method must be called while in the Matter context (from
     *   the Matter event loop, or while holding the Matter stack lock).
     *
     *   @note
     *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
     *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
     *       the currently-running timer will first be cancelled.
     *
     *   @param[in]  aDelay             Time before this timer fires.
     *   @param[in]  aComplete          A pointer to the function called when timer expires.
     *   @param[in]  aAppState          A pointer to the application state object used when timer expires.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
     *   @return Other Value indicating timer failed to start.
     */
    virtual CHIP_ERROR StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) = 0;

    /**
     * @brief
     *   This method extends the timer expiry to the provided aDelay. This method must be called while in the Matter context
     *   (from the Matter event loop, or while holding the Matter stack lock).
     *   aDelay is not added to the Remaining time of the timer. The finish line is pushed back to aDelay.
     *
     *   @note The goal of this method is that the timer remaining time cannot be shrunk and only extended to a new time
     *         If the provided new Delay is smaller than the timer's remaining time, the timer is left untouched.
     *         In the other case the method acts like StartTimer
     *
     *   @param[in]  aDelay             Time before this timer fires.
     *   @param[in]  aComplete          A pointer to the function called when timer expires.
     *   @param[in]  aAppState          A pointer to the application state object used when timer expires.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return CHIP_ERROR_INVALID_ARGUMENT If the provided aDelay value is 0
     *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
     *   @return Other Value indicating timer failed to start.
     */
    virtual CHIP_ERROR ExtendTimerTo(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) = 0;

    /**
     * @brief
     *   This method searches for the timer matching the provided parameters.
     *   and returns whether it is still "running" and waiting to trigger or not.
     *
     *   @note This is used to verify by how long the ExtendTimer method extends the timer, as it may ignore an extension request
     *        if it is shorter than the current timer's remaining time.
     *
     *   @param[in]  onComplete         A pointer to the function called when timer expires.
     *   @param[in]  appState           A pointer to the application state object used when timer expires.
     *
     *   @return True if there is a current timer set to call, at some point in the future, the provided onComplete callback
     *           with the corresponding appState context. False otherwise.
     */
    virtual bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) = 0;

    /**
     * @brief
     *   This method searches for the timer matching the provided parameters
     *   and returns the remaining time left before it expires.
     *   @param[in]  onComplete         A pointer to the function called when timer expires.
     *   @param[in]  appState           A pointer to the application state object used when timer expires.
     *
     *  @return The remaining time left before the timer expires.
     */
    virtual Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) = 0;

    /**
     * @brief This method cancels a one-shot timer, started earlier through @p StartTimer().  This method must
     *        be called while in the Matter context (from the Matter event loop, or while holding the Matter
     *        stack lock).
     *
     *   @note
     *       The cancellation could fail silently if the timer specified by the combination of the callback
     *       function and application state object couldn't be found.
     *
     *   @param[in]  aOnComplete   A pointer to the callback function used in calling @p StartTimer().
     *   @param[in]  aAppState     A pointer to the application state object used in calling @p StartTimer().
     *
     */
    virtual void CancelTimer(TimerCompleteCallback aOnComplete, void * aAppState) = 0;

    /**
     * @brief
     *   Schedules a `TimerCompleteCallback` to be run as soon as possible in the Matter context.
     *
     *  WARNING: This must only be called when already in the Matter context (from the Matter event loop, or
     *           while holding the Matter stack lock). The `PlatformMgr::ScheduleWork()` equivalent method
     *           is safe to call outside Matter context.
     *
     * @param[in] aComplete     A pointer to a callback function to be called when this timer fires.
     * @param[in] aAppState     A pointer to an application state object to be passed to the callback function as argument.
     *
     * @retval CHIP_ERROR_INCORRECT_STATE   If the System::Layer has not been initialized.
     * @retval CHIP_ERROR_NO_MEMORY         If the SystemLayer cannot allocate a new timer.
     * @retval CHIP_NO_ERROR                On success.
     */
    virtual CHIP_ERROR ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) = 0;

    /**
     * @brief
     *   Schedules a lambda object to be run as soon as possible in the Matter context.
     *
     * This is safe to call from any context and will guarantee execution in Matter context.
     * Note that the Lambda's capture have to fit within `CHIP_CONFIG_LAMBDA_EVENT_SIZE` bytes.
     *
     * @param[in] lambda The Lambda to execute in Matter context.
     *
     * @retval CHIP_NO_ERROR On success.
     * @retval other Platform-specific errors generated indicating the reason for failure.
     */
    template <typename Lambda>
    CHIP_ERROR ScheduleLambda(const Lambda & lambda)
    {
        static_assert(std::is_invocable_v<Lambda>, "lambda argument must be an invocable with no arguments");
        LambdaBridge bridge;
        bridge.Initialize(lambda);
        return ScheduleLambdaBridge(std::move(bridge));
    }

private:
    CHIP_ERROR ScheduleLambdaBridge(LambdaBridge && bridge);

    // Not copyable
    Layer(const Layer &)             = delete;
    Layer & operator=(const Layer &) = delete;
};

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

class LayerFreeRTOS : public Layer
{
};

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

class LayerSockets : public Layer
{
public:
    /**
     * Initialize watching for events on a file descriptor.
     *
     * Returns an opaque token through @a tokenOut that must be passed to subsequent operations for this file descriptor.
     * Multiple calls to start watching the same file descriptor will return the same token.
     * StopWatchingSocket() must be called before closing the file descriptor.
     */
    virtual CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut) = 0;

    /**
     * Register a callback function.
     *
     * The callback will be invoked (with the CHIP stack lock held) when requested event(s) are ready.
     */
    virtual CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data) = 0;

    /**
     * Request a callback when the associated file descriptor is readable.
     */
    virtual CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token) = 0;

    /**
     * Request a callback when the associated file descriptor is writable.
     */
    virtual CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token) = 0;

    /**
     * Cancel a request for a callback when the associated file descriptor is readable.
     */
    virtual CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token) = 0;

    /**
     * Cancel a request for a callback when the associated file descriptor is writable.
     */
    virtual CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token) = 0;

    /**
     * Stop watching for events on the associated file descriptor.
     *
     * This MUST be called before the file descriptor is closed.
     * It is not necessary to clear callback requests before calling this function.
     */
    virtual CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut) = 0;

    /**
     * Return a SocketWatchToken that is guaranteed not to be valid. Clients may use this to initialize variables.
     */
    virtual SocketWatchToken InvalidSocketWatchToken() = 0;
};

class LayerSocketsLoop;

/**
 * EventLoopHandlers can be registered with a LayerSocketsLoop instance to enable
 * participation of those handlers in the processing cycle of the event loop. This makes
 * it possible to implement adapters that allow components utilizing a third-party event
 * loop API to participate in the Matter event loop, instead of having to run an entirely
 * separate event loop on another thread.
 *
 * Specifically, the `PrepareEvents` and `HandleEvents` methods of registered event loop
 * handlers will be called from the LayerSocketsLoop methods of the same names.
 *
 * @see LayerSocketsLoop::PrepareEvents
 * @see LayerSocketsLoop::HandleEvents
 */
class EventLoopHandler : public chip::IntrusiveListNodeBase<>
{
public:
    virtual ~EventLoopHandler() {}

    /**
     * Prepares events and returns the next requested wake time.
     */
    virtual Clock::Timestamp PrepareEvents(Clock::Timestamp now) { return Clock::Timestamp::max(); }

    /**
     * Handles / dispatches pending events.
     * Every call to this method will have been preceded by a call to `PrepareEvents`.
     */
    virtual void HandleEvents() = 0;

private:
    // mState is provided exclusively for use by the LayerSocketsLoop implementation
    // sub-class and can be accessed by it via the LayerSocketsLoop::LoopHandlerState() helper.
    friend class LayerSocketsLoop;
    intptr_t mState = 0;
};

class LayerSocketsLoop : public LayerSockets
{
public:
    virtual void Signal()          = 0;
    virtual void EventLoopBegins() = 0;
    virtual void PrepareEvents()   = 0;
    virtual void WaitForEvents()   = 0;
    virtual void HandleEvents()    = 0;
    virtual void EventLoopEnds()   = 0;

#if !CHIP_SYSTEM_CONFIG_USE_DISPATCH
    virtual void AddLoopHandler(EventLoopHandler & handler)    = 0;
    virtual void RemoveLoopHandler(EventLoopHandler & handler) = 0;
#endif // !CHIP_SYSTEM_CONFIG_USE_DISPATCH

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    virtual void SetDispatchQueue(dispatch_queue_t dispatchQueue) = 0;
    virtual dispatch_queue_t GetDispatchQueue()                   = 0;
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    virtual void SetLibEvLoop(struct ev_loop * aLibEvLoopP) = 0;
    virtual struct ev_loop * GetLibEvLoop()                 = 0;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

protected:
    // Expose EventLoopHandler.mState as a non-const reference to sub-classes
    decltype(EventLoopHandler::mState) & LoopHandlerState(EventLoopHandler & handler) { return handler.mState; }
};

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace System
} // namespace chip
