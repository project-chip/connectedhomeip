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
#include <app/icd/ICDStateObserver.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace reporting {

// Forward declaration of TestReportScheduler to allow it to be friend with ReportScheduler
class TestReportScheduler;

using Timestamp = System::Clock::Timestamp;

class TimerContext
{
public:
    virtual ~TimerContext() {}
    virtual void TimerFired() = 0;
};

class ReportScheduler : public ReadHandler::Observer, public ICDStateObserver
{
public:
    /// @brief This class acts as an interface between the report scheduler and the system timer to reduce dependencies on the
    /// system layer.
    class TimerDelegate
    {
    public:
        virtual ~TimerDelegate() {}
        /// @brief Start a timer for a given context. The report scheduler must always cancel an existing timer for a context (using
        /// CancelTimer) before starting a new one for that context.
        /// @param context context to pass to the timer callback.
        /// @param aTimeout time in miliseconds before the timer expires
        virtual CHIP_ERROR StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) = 0;
        /// @brief Cancel a timer for a given context
        /// @param context used to identify the timer to cancel
        virtual void CancelTimer(TimerContext * context)   = 0;
        virtual bool IsTimerActive(TimerContext * context) = 0;
        virtual Timestamp GetCurrentMonotonicTimestamp()   = 0;
    };

    class ReadHandlerNode : public TimerContext
    {
    public:
        ReadHandlerNode(ReadHandler * aReadHandler, TimerDelegate * aTimerDelegate, ReportScheduler * aScheduler) :
            mTimerDelegate(aTimerDelegate), mScheduler(aScheduler)
        {
            VerifyOrDie(aReadHandler != nullptr);
            VerifyOrDie(aTimerDelegate != nullptr);
            VerifyOrDie(aScheduler != nullptr);

            mReadHandler = aReadHandler;
            SetIntervalTimeStamps(aReadHandler);
        }
        ReadHandler * GetReadHandler() const { return mReadHandler; }

        /// @brief Check if the Node is reportable now, meaning its readhandler was made reportable by attribute dirtying and
        /// handler state, and minimal time interval since last report has elapsed, or the maximal time interval since last
        /// report has elapsed
        bool IsReportableNow() const
        {
            Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

            return (mReadHandler->IsGeneratingReports() &&
                    (now >= mMinTimestamp && (mReadHandler->IsDirty() || now >= mMaxTimestamp || now >= mSyncTimestamp)));
        }

        bool IsEngineRunScheduled() const { return mEngineRunScheduled; }
        void SetEngineRunScheduled(bool aEngineRunScheduled) { mEngineRunScheduled = aEngineRunScheduled; }

        void SetIntervalTimeStamps(ReadHandler * aReadHandler)
        {
            uint16_t minInterval, maxInterval;
            aReadHandler->GetReportingIntervals(minInterval, maxInterval);
            Timestamp now  = mTimerDelegate->GetCurrentMonotonicTimestamp();
            mMinTimestamp  = now + System::Clock::Seconds16(minInterval);
            mMaxTimestamp  = now + System::Clock::Seconds16(maxInterval);
            mSyncTimestamp = mMaxTimestamp;
        }

        void TimerFired() override
        {
            mScheduler->ReportTimerCallback();
            SetEngineRunScheduled(true);
        }

        void SetSyncTimestamp(System::Clock::Timestamp aSyncTimestamp)
        {
            // Prevents the sync timestamp being set to a value lower than the min timestamp to prevent it to appear as reportable
            // on the next timeout calculation and cause the scheduler to run the engine too early
            VerifyOrReturn(aSyncTimestamp >= mMinTimestamp);
            mSyncTimestamp = aSyncTimestamp;
        }

        System::Clock::Timestamp GetMinTimestamp() const { return mMinTimestamp; }
        System::Clock::Timestamp GetMaxTimestamp() const { return mMaxTimestamp; }
        System::Clock::Timestamp GetSyncTimestamp() const { return mSyncTimestamp; }

    private:
        TimerDelegate * mTimerDelegate;
        ReadHandler * mReadHandler;
        ReportScheduler * mScheduler;
        Timestamp mMinTimestamp;
        Timestamp mMaxTimestamp;
        Timestamp mSyncTimestamp; // Timestamp at which the read handler will be allowed to emit a report so it can be synced with
                                  // other handlers that have an earlier max timestamp
        bool mEngineRunScheduled = false; // Flag to indicate if the engine run is already scheduled so the scheduler can ignore
                                          // it when calculating the next run time
    };

    ReportScheduler(TimerDelegate * aTimerDelegate) : mTimerDelegate(aTimerDelegate) {}
    /**
     *  Interface to act on changes in the ReadHandler reportability
     */
    virtual ~ReportScheduler() = default;

    virtual void ReportTimerCallback() = 0;

    /// @brief Check whether a ReadHandler is reportable right now, taking into account its minimum and maximum intervals.
    /// @param aReadHandler read handler to check
    bool IsReportableNow(ReadHandler * aReadHandler) { return FindReadHandlerNode(aReadHandler)->IsReportableNow(); }
    /// @brief Check if a ReadHandler is reportable without considering the timing
    bool IsReadHandlerReportable(ReadHandler * aReadHandler) const { return aReadHandler->IsReportable(); }
    /// @brief Sets the ForceDirty flag of a ReadHandler
    void HandlerForceDirtyState(ReadHandler * aReadHandler) { aReadHandler->ForceDirtyState(); }

    /// @brief Get the number of ReadHandlers registered in the scheduler's node pool
    size_t GetNumReadHandlers() const { return mNodesPool.Allocated(); }

#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
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
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

protected:
    friend class chip::app::reporting::TestReportScheduler;

    /// @brief Find the ReadHandlerNode for a given ReadHandler pointer
    /// @param [in] aReadHandler ReadHandler pointer to look for in the ReadHandler nodes list
    /// @return Node Address if node was found, nullptr otherwise
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
