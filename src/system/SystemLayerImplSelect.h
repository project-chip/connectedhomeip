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
 *      This file declares an implementation of System::Layer using select().
 */

#pragma once

#include "system/SystemConfig.h"

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/select.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <inet/ZephyrSocket.h> // nogncheck
#endif

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
#include <ev.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

#include <lib/support/IntrusiveList.h>
#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>
#include <system/WakeEvent.h>

namespace chip {
namespace System {

class LayerImplSelect : public LayerSelectLoop
{
public:
    LayerImplSelect() = default;
    ~LayerImplSelect() override { VerifyOrDie(mLayerState.Destroy()); }

    // Layer overrides.
    CriticalFailure Init() override;
    void Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CriticalFailure StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override;
    Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CriticalFailure ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // LayerSocket overrides.
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut) override;
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data) override;
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut) override;
    SocketWatchToken InvalidSocketWatchToken() override { return reinterpret_cast<SocketWatchToken>(nullptr); }
#endif

    // LayerSelectLoop overrides.
    void Signal() override;
    void EventLoopBegins() override {}
    void PrepareEvents() override;
    void WaitForEvents() override;
    void HandleEvents() override;
    void EventLoopEnds() override {}

    void AddLoopHandler(EventLoopHandler & handler) override;
    void RemoveLoopHandler(EventLoopHandler & handler) override;

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    virtual void SetLibEvLoop(struct ev_loop * aLibEvLoopP) override { mLibEvLoopP = aLibEvLoopP; };
    virtual struct ev_loop * GetLibEvLoop() override { return mLibEvLoopP; };
    static void HandleLibEvTimer(EV_P_ struct ev_timer * t, int revents);
    static void HandleLibEvIoWatcher(EV_P_ struct ev_io * i, int revents);
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

    // Expose the result of WaitForEvents() for non-blocking socket implementations.
    bool IsSelectResultValid() const { return mSelectResult >= 0; }

    /**
     * @brief Abstract interface for an event source compatible with a select()-based event loop.
     *
     * This interface defines the contract for objects that wish to register file descriptors
     * and timeouts for monitoring. It is designed to be managed by a selection layer (e.g., LayerImplSelect).
     */
    struct EventSource : public IntrusiveListNodeBase<IntrusiveMode::Strict>
    {
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~EventSource() = default;

        /**
         * @brief Prepares the file descriptor sets and timeout before the select() call.
         *
         * This method is called by the event loop to gather all file descriptors and the
         * minimum required timeout from all registered sources.
         *
         * @param[in,out] maxfd      The highest-numbered file descriptor. Must be updated if
         *                          this source adds a descriptor larger than the current value.
         * @param[in,out] readfds   Set of file descriptors to be watched for read readiness.
         * @param[in,out] writefds  Set of file descriptors to be watched for write readiness.
         * @param[in,out] exceptfds Set of file descriptors to be watched for error conditions.
         * @param[in,out] timeout   The maximum time to wait for an event.
         *
         * @warning **Timeout Modification Logic:**
         * To ensure all sources are handled correctly, you must follow these rules:
         * - **DO NOT** modify @p timeout if this source's desired timeout is **larger** than the current value.
         * - **ONLY** modify @p timeout if this source's desired timeout is **smaller** than the current value.
         *
         * This ensures the event loop wakes up in time for the earliest pending event among all sources.
         */
        virtual void PrepareEvents(int & maxfd, fd_set & readfds, fd_set & writefds, fd_set & exceptfds,
                                   struct timeval & timeout) = 0;

        /**
         * @brief Processes the results after the select() call returns.
         *
         * The event loop calls this method to allow the source to check if its descriptors
         * are set in the resulting masks and perform the associated I/O or logic.
         *
         * @param[in] readfds   The set of descriptors ready for reading.
         * @param[in] writefds  The set of descriptors ready for writing.
         * @param[in] exceptfds The set of descriptors with pending error conditions.
         */
        virtual void ProcessEvents(const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds) = 0;
    };

    /**
     * @brief Register an EventSource with this LayerImplSelect instance.
     *
     * Adds the given EventSource to the internal list of sources that will be
     * consulted when preparing and handling events in the select()-based loop.
     *
     * Thread-safety: This method is not thread-safe and MUST be called with
     * the ChipStack lock held.
     *
     * Ownership: LayerImplSelect does not take ownership of @p source.
     * The caller is responsible for ensuring that the EventSource object
     * remains valid for as long as it is registered.
     *
     * Lifecycle: The EventSource MUST either outlive its registration (i.e.,
     * remain alive until after it is removed) or be explicitly removed using
     * EventSourceRemove() before it is destroyed.
     */
    void EventSourceAdd(EventSource * source);

    /**
     * @brief Unregister a previously added EventSource.
     *
     * Removes the given EventSource from the internal list of sources that are
     * monitored by the select()-based loop.
     *
     * Thread-safety: This method is not thread-safe and MUST be called with
     * the ChipStack lock held.
     *
     * Ownership: This method does not delete or otherwise destroy @p source;
     * ownership remains with the caller.
     *
     * Lifecycle: EventSourceRemove() MUST be called before destroying an
     * EventSource that has been registered with EventSourceAdd(). It is safe
     * to call EventSourceRemove() multiple times with the same pointer; calls
     * after the source has been removed have no effect.
     */
    void EventSourceRemove(EventSource * source);

    /**
     * @brief Clear all registered EventSource instances.
     *
     * Removes all EventSource objects from the internal list maintained by
     * LayerImplSelect.
     *
     * Thread-safety: This method is not thread-safe and MUST be called with
     * the ChipStack lock held.
     *
     * Ownership: This method does not delete or destroy any EventSource
     * instances; it only clears their registration with the loop.
     *
     * Lifecycle: After this call returns, all previously registered
     * EventSource instances are no longer tracked by the event loop. The
     * objects themselves remain the responsibility of the caller.
     */
    void EventSourceClear();

protected:
    IntrusiveList<EventSource> mSources;
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

    static constexpr int kSocketWatchMax = (INET_CONFIG_ENABLE_TCP_ENDPOINT ? INET_CONFIG_NUM_TCP_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_UDP_ENDPOINT ? INET_CONFIG_NUM_UDP_ENDPOINTS : 0);

    struct SocketWatch
    {
        void Clear();
        int mFD;
        SocketEvents mPendingIO;
        SocketWatchCallback mCallback;
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
        struct ev_io mIoWatcher;
        LayerImplSelect * mLayerImplSelectP;
        void DisableAndClear();
#endif

        intptr_t mCallbackData;
    };
    SocketWatch mSocketWatchPool[kSocketWatchMax];
#endif

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    // List of expired timers being processed right now.  Stored in a member so
    // we can cancel them.
    TimerList mExpiredTimers;
    timeval mNextTimeout;

    IntrusiveList<EventLoopHandler> mLoopHandlers;

    // Members for select loop
    struct SelectSets
    {
        fd_set mReadSet;
        fd_set mWriteSet;
        fd_set mErrorSet;
    };
    SelectSets mSelected;
    int mMaxFd;

    // Return value from select(), carried between WaitForEvents() and HandleEvents().
    int mSelectResult;

    ObjectLifeCycle mLayerState;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
    struct ev_loop * mLibEvLoopP;
#else
    WakeEvent mWakeEvent;
#endif
};

using LayerImpl = LayerImplSelect;

} // namespace System
} // namespace chip
