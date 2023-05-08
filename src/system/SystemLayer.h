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
#include <lib/support/LambdaBridge.h>
#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemEvent.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SocketEvents.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#include <utility>

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
     *   Schedules a function with a signature identical to `OnCompleteFunct` to be run as soon as possible in the Matter context.
     *   This must only be called when already in the Matter context (from the Matter event loop, or while holding the Matter
     *   stack lock).
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
     *   Schedules a lambda even to be run as soon as possible in the CHIP context. This function is not thread-safe,
     *   it must be called with in the CHIP context
     *
     *  @param[in] event   A object encapsulate the context of a lambda
     *
     *  @retval    CHIP_NO_ERROR                  On success.
     *  @retval    other Platform-specific errors generated indicating the reason for failure.
     */
    CHIP_ERROR ScheduleLambdaBridge(LambdaBridge && event);

    /**
     * @brief
     *   Schedules a lambda object to be run as soon as possible in the CHIP context. This function is not thread-safe,
     *   it must be called with in the CHIP context
     */
    template <typename Lambda>
    CHIP_ERROR ScheduleLambda(const Lambda & lambda)
    {
        LambdaBridge bridge;
        bridge.Initialize(lambda);
        return ScheduleLambdaBridge(std::move(bridge));
    }

private:
    // Copy and assignment NOT DEFINED
    Layer(const Layer &) = delete;
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

class LayerSocketsLoop : public LayerSockets
{
public:
    virtual void Signal()          = 0;
    virtual void EventLoopBegins() = 0;
    virtual void PrepareEvents()   = 0;
    virtual void WaitForEvents()   = 0;
    virtual void HandleEvents()    = 0;
    virtual void EventLoopEnds()   = 0;

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    virtual void SetDispatchQueue(dispatch_queue_t dispatchQueue) = 0;
    virtual dispatch_queue_t GetDispatchQueue()                   = 0;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
};

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace System
} // namespace chip
