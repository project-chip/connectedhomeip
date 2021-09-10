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
#include <system/SystemError.h>
#include <system/SystemEvent.h>
#include <system/SystemObject.h>
#include <system/SystemTimer.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SocketEvents.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

namespace chip {
namespace System {

class Layer;
using TimerCompleteCallback = void (*)(Layer * aLayer, void * appState);

/**
 * This provides access to timers according to the configured event handling model.
 *
 * The abstract class hierarchy is:
 * - Layer: Core timer methods.
 *   - LayerLwIP: Adds methods specific to CHIP_SYSTEM_CONFIG_USING_LWIP.
 *   - LayerSockets: Adds I/O event methods specific to CHIP_SYSTEM_CONFIG_USING_SOCKETS.
 *     - LayerSocketsLoop: Adds methods for event-loop-based implementations.
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
    virtual CHIP_ERROR Shutdown() = 0;

    /**
     * True if this Layer is initialized. No method on Layer or its abstract descendants, other than this and `Init()`,
     * may be called from general code unless this is true. (Individual Impls may have looser constraints internally.)
     */
    virtual bool IsInitialized() const = 0;

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
    virtual CHIP_ERROR StartTimer(uint32_t aDelayMilliseconds, TimerCompleteCallback aComplete, void * aAppState) = 0;

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
    virtual void CancelTimer(TimerCompleteCallback aOnComplete, void * aAppState) = 0;

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
    virtual CHIP_ERROR ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) = 0;

private:
    // Copy and assignment NOT DEFINED
    Layer(const Layer &) = delete;
    Layer & operator=(const Layer &) = delete;
};

#if CHIP_SYSTEM_CONFIG_USE_LWIP

class LayerLwIP : public Layer
{
protected:
    struct LwIPEventHandlerDelegate;

public:
    class EventHandlerDelegate
    {
    public:
        typedef CHIP_ERROR (*EventHandlerFunction)(Object & aTarget, EventType aEventType, uintptr_t aArgument);

        bool IsInitialized(void) const;
        void Init(EventHandlerFunction aFunction);
        void Prepend(const EventHandlerDelegate *& aDelegateList);

    private:
        friend class LayerLwIP::LwIPEventHandlerDelegate;
        EventHandlerFunction mFunction;
        const EventHandlerDelegate * mNextDelegate;
    };

    /**
     * This adds an event handler delegate to the system layer to extend its ability to handle LwIP events.
     *
     *  @param[in]  aDelegate   An uninitialied LwIP event handler delegate structure
     *
     *  @retval     CHIP_NO_ERROR                 On success.
     *  @retval     CHIP_ERROR_INVALID_ARGUMENT   If the function pointer contained in aDelegate is NULL
     */
    virtual CHIP_ERROR AddEventHandlerDelegate(LayerLwIP::EventHandlerDelegate & aDelegate) = 0;

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
    virtual CHIP_ERROR PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument) = 0;

protected:
    // Provide access to private members of EventHandlerDelegate.
    struct LwIPEventHandlerDelegate : public EventHandlerDelegate
    {
        const EventHandlerFunction & GetFunction() const { return mFunction; }
        const LwIPEventHandlerDelegate * GetNextDelegate() const
        {
            return static_cast<const LwIPEventHandlerDelegate *>(mNextDelegate);
        }
    };
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
