/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/ReadHandler.h>
#include <app/icd/server/ICDStateObserver.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace reporting {

// Forward declaration of TestReportScheduler to allow it to be friend with ReportScheduler
class TestReportScheduler;

class TimerContext
{
public:
    virtual ~TimerContext() {}
    virtual void TimerFired() = 0;
};

/**
 * @class ReportScheduler
 *
 * @brief This class is responsible for scheduling Engine runs based on the reporting intervals of the ReadHandlers.
 *
 *
 * This class holds a pool of ReadHandlerNodes that are used to keep track of the minimum and maximum timestamps for a report to be
 * emitted based on the reporting intervals of the ReadHandlers associated with the node.
 *
 * The ReportScheduler also holds a TimerDelegate pointer that is used to start and cancel timers for the ReadHandlers depending
 * on the reporting logic of the Scheduler.
 *
 * It inherits the ReadHandler::Observer class to be notified of reportability changes in the ReadHandlers.
 * It inherits the ICDStateObserver class to allow the implementation to generate reports based on the changes in ICD devices state,
 * such as going from idle to active mode and vice-versa.
 *
 * @note The logic for how and when to schedule reports is implemented in the subclasses of ReportScheduler, such as
 * ReportSchedulerImpl and SyncronizedReportSchedulerImpl.
 */
class ReportScheduler : public ReadHandler::Observer, public ICDStateObserver
{
public:
    using Timestamp = System::Clock::Timestamp;

    /// @brief This class acts as an interface between the report scheduler and the system timer to reduce dependencies on the
    /// system layer.
    class TimerDelegate
    {
    public:
        virtual ~TimerDelegate() {}
        /// @brief Start a timer for a given context. The report scheduler must always cancel an existing timer for a context (using
        /// CancelTimer) before starting a new one for that context.
        /// @param context context to pass to the timer callback.
        /// @param aTimeout time in milliseconds before the timer expires
        virtual CHIP_ERROR StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) = 0;
        /// @brief Cancel a timer for a given context
        /// @param context used to identify the timer to cancel
        virtual void CancelTimer(TimerContext * context)   = 0;
        virtual bool IsTimerActive(TimerContext * context) = 0;
        virtual Timestamp GetCurrentMonotonicTimestamp()   = 0;
    };

    /**
     * @class ReadHandlerNode
     *
     * @brief This class is responsible for determining when a ReadHandler is reportable depending on the monotonic timestamp of
     *  the system and the intervals of the ReadHandler. It inherits the TimerContext class to allow it to be used as a context for
     *  a TimerDelegate so that the TimerDelegate can call the TimerFired method when the timer expires.
     *
     *  Three conditions that can prevent the ReadHandler from being reportable:
     *  1: The ReadHandler is not in the CanStartReporting state:
     *      This condition can be resolved by setting the CanStartReporting flag on the ReadHandler
     *
     *  2: The minimal interval since the last report has not elapsed
     *      This condition can be resolved after enough time has passed since the last report or by setting the EngineRunScheduled
     *      flag
     *
     *  3: The maximal interval since the last report has not elapsed and the ReadHandler is not dirty:
     *      This condition can be resolved after enough time has passed since the last report to reach the max interval, by the
     *      ReadHandler becoming dirty or by setting the CanBeSynced flag and having another ReadHandler needing to report.
     *
     *  Once the 3 conditions are met, the ReadHandler is considered reportable.
     *
     *  Flags:
     *
     *  CanBeSynced: Mechanism to allow the ReadHandler to emit a report if another readHandler is ReportableNow.
     *  This flag is currently only used by the SynchronizedReportScheduler to allow firing reports of ReadHandlers at the same
     *  time.
     *
     *  EngineRunScheduled: Mechanism to ensure that the reporting engine will see the ReadHandler as reportable if a timer fires.
     *  This flag is used to confirm that the next report timer has fired for a ReadHandler, thus allowing reporting when timers
     *  fire earlier than the minimal timestamp due to mechanisms such as NTP clock adjustments.
     *
     */
    class ReadHandlerNode : public TimerContext
    {
    public:
        enum class ReadHandlerNodeFlags : uint8_t
        {
            // Flag to indicate if the engine run is already scheduled so the scheduler can ignore
            // it when calculating the next run time
            EngineRunScheduled = (1 << 0),
            // Flag to allow the read handler to be synced with other handlers that have an earlier max timestamp
            CanBeSynced = (1 << 1),
        };

        ReadHandlerNode(ReadHandler * aReadHandler, ReportScheduler * aScheduler, const Timestamp & now) : mScheduler(aScheduler)
        {
            VerifyOrDie(aReadHandler != nullptr);
            VerifyOrDie(aScheduler != nullptr);

            mReadHandler = aReadHandler;
            SetIntervalTimeStamps(aReadHandler, now);
        }
        ReadHandler * GetReadHandler() const { return mReadHandler; }

        /// @brief Check if the Node is reportable now, meaning its readhandler was made reportable by attribute dirtying and
        /// handler state, and minimal time interval since the last report has elapsed, or the maximal time interval since the last
        /// report has elapsed.
        /// @note If a handler has been flagged as scheduled for an engine run, it will be reported regardless of the timestamps.
        /// This is done to guarantee that the reporting engine will see the handler as reportable if a timer fires, even if it
        /// fires early.
        /// @param now current time to use for the check, the user must ensure to provide a valid time for this to be reliable
        bool IsReportableNow(const Timestamp & now) const
        {
            return (mReadHandler->CanStartReporting() &&
                    ((now >= mMinTimestamp && (mReadHandler->IsDirty() || now >= mMaxTimestamp || CanBeSynced())) ||
                     IsEngineRunScheduled()));
        }

        bool CanStartReporting() const { return mReadHandler->CanStartReporting(); }
        bool IsChunkedReport() const { return mReadHandler->IsChunkedReport(); }
        bool IsEngineRunScheduled() const { return mFlags.Has(ReadHandlerNodeFlags::EngineRunScheduled); }
        void SetEngineRunScheduled(bool aEngineRunScheduled)
        {
            mFlags.Set(ReadHandlerNodeFlags::EngineRunScheduled, aEngineRunScheduled);
        }
        bool CanBeSynced() const { return mFlags.Has(ReadHandlerNodeFlags::CanBeSynced); }
        void SetCanBeSynced(bool aCanBeSynced) { mFlags.Set(ReadHandlerNodeFlags::CanBeSynced, aCanBeSynced); }

        /// @brief Set the interval timestamps for the node based on the read handler reporting intervals
        /// @param aReadHandler read handler to get the intervals from
        /// @param now current time to calculate the mMin and mMax timestamps, the user must ensure to provide a valid time for this
        /// to be reliable
        void SetIntervalTimeStamps(ReadHandler * aReadHandler, const Timestamp & now)
        {
            uint16_t minInterval, maxInterval;
            aReadHandler->GetReportingIntervals(minInterval, maxInterval);
            mMinTimestamp = now + System::Clock::Seconds16(minInterval);
            mMaxTimestamp = now + System::Clock::Seconds16(maxInterval);
        }

        void TimerFired() override
        {
            SetEngineRunScheduled(true);
            mScheduler->ReportTimerCallback();
        }

        System::Clock::Timestamp GetMinTimestamp() const { return mMinTimestamp; }
        System::Clock::Timestamp GetMaxTimestamp() const { return mMaxTimestamp; }

    private:
        ReadHandler * mReadHandler;
        ReportScheduler * mScheduler;
        Timestamp mMinTimestamp;
        Timestamp mMaxTimestamp;

        BitFlags<ReadHandlerNodeFlags> mFlags;
    };

    ReportScheduler(TimerDelegate * aTimerDelegate) : mTimerDelegate(aTimerDelegate) {}

    virtual ~ReportScheduler() = default;

    virtual void ReportTimerCallback() = 0;

    /// @brief Check whether a ReadHandler is reportable right now, taking into account its minimum and maximum intervals.
    /// @param aReadHandler read handler to check
    bool IsReportableNow(ReadHandler * aReadHandler)
    {
        // Update the now timestamp to ensure external calls to IsReportableNow are always comparing to the current time
        Timestamp now          = mTimerDelegate->GetCurrentMonotonicTimestamp();
        ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
        return (nullptr != node) ? node->IsReportableNow(now) : false;
    }

    /// @brief Check if a ReadHandler is reportable without considering the timing
    bool IsReadHandlerReportable(ReadHandler * aReadHandler) const
    {
        return (nullptr != aReadHandler) ? aReadHandler->ShouldStartReporting() : false;
    }
    /// @brief Sets the ForceDirty flag of a ReadHandler
    void HandlerForceDirtyState(ReadHandler * aReadHandler) { aReadHandler->ForceDirtyState(); }

    /// @brief Get the number of ReadHandlers registered in the scheduler's node pool
    size_t GetNumReadHandlers() const { return mNodesPool.Allocated(); }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    Timestamp GetMinTimestampForHandler(const ReadHandler * aReadHandler)
    {
        ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
        return node->GetMinTimestamp();
    }
    Timestamp GetMaxTimestampForHandler(const ReadHandler * aReadHandler)
    {
        ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
        return node->GetMaxTimestamp();
    }
    ReadHandlerNode * GetReadHandlerNode(const ReadHandler * aReadHandler) { return FindReadHandlerNode(aReadHandler); }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

protected:
    friend class chip::app::reporting::TestReportScheduler;

    /// @brief Find the ReadHandlerNode for a given ReadHandler pointer
    /// @param [in] aReadHandler ReadHandler pointer to look for in the ReadHandler nodes list
    /// @return Node Address if the node was found, nullptr otherwise
    ReadHandlerNode * FindReadHandlerNode(const ReadHandler * aReadHandler)
    {
        ReadHandlerNode * foundNode = nullptr;
        mNodesPool.ForEachActiveObject([&foundNode, aReadHandler](ReadHandlerNode * node) {
            if (node->GetReadHandler() == aReadHandler)
            {
                foundNode = node;
                return Loop::Break;
            }

            return Loop::Continue;
        });
        return foundNode;
    }

    ObjectPool<ReadHandlerNode, CHIP_IM_MAX_NUM_READS + CHIP_IM_MAX_NUM_SUBSCRIPTIONS> mNodesPool;
    TimerDelegate * mTimerDelegate;
};
}; // namespace reporting
}; // namespace app
}; // namespace chip
