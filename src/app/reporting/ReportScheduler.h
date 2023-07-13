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
#include <lib/core/CHIPError.h>
#include <lib/support/IntrusiveList.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace reporting {

// Forward declaration of TestReportScheduler to allow it to be friend with ReportScheduler
class TestReportScheduler;

using Timestamp = System::Clock::Timestamp;

class ReportScheduler : public ReadHandler::Observer
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
        virtual CHIP_ERROR StartTimer(void * context, System::Clock::Timeout aTimeout) = 0;
        /// @brief Cancel a timer for a given context
        /// @param context used to identify the timer to cancel
        virtual void CancelTimer(void * context)         = 0;
        virtual bool IsTimerActive(void * context)       = 0;
        virtual Timestamp GetCurrentMonotonicTimestamp() = 0;
    };

    class ReadHandlerNode : public IntrusiveListNodeBase<>
    {
    public:
        using TimerCompleteCallback = void (*)();

        ReadHandlerNode(ReadHandler * aReadHandler, TimerDelegate * aTimerDelegate, TimerCompleteCallback aCallback) :
            mTimerDelegate(aTimerDelegate), mCallback(aCallback)
        {
            VerifyOrDie(aReadHandler != nullptr);
            VerifyOrDie(aTimerDelegate != nullptr);
            VerifyOrDie(aCallback != nullptr);

            mReadHandler = aReadHandler;
            SetIntervalTimeStamps(aReadHandler);
        }
        ReadHandler * GetReadHandler() const { return mReadHandler; }
        /// @brief Check if the Node is reportable now, meaning its readhandler was made reportable by attribute dirtying and
        /// handler state, and minimal time interval since last report has elapsed, or the maximal time interval since last
        /// report has elapsed
        bool IsReportableNow() const
        {
            // TODO: Add flags to allow for test to simulate waiting for the min interval or max intrval to elapse when integrating
            // the scheduler in the ReadHandler
            Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
            return (mReadHandler->IsGeneratingReports() &&
                    ((now >= mMinTimestamp && mReadHandler->IsDirty()) || now >= mMaxTimestamp));
        }

        void SetIntervalTimeStamps(ReadHandler * aReadHandler)
        {
            uint16_t minInterval, maxInterval;
            aReadHandler->GetReportingIntervals(minInterval, maxInterval);
            Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
            mMinTimestamp = now + System::Clock::Seconds16(minInterval);
            mMaxTimestamp = now + System::Clock::Seconds16(maxInterval);
        }

        void RunCallback() { mCallback(); }

        Timestamp GetMinTimestamp() const { return mMinTimestamp; }
        Timestamp GetMaxTimestamp() const { return mMaxTimestamp; }

    private:
        TimerDelegate * mTimerDelegate;
        TimerCompleteCallback mCallback;
        ReadHandler * mReadHandler;
        Timestamp mMinTimestamp;
        Timestamp mMaxTimestamp;
    };

    ReportScheduler(TimerDelegate * aTimerDelegate) : mTimerDelegate(aTimerDelegate) {}
    /**
     *  Interface to act on changes in the ReadHandler reportability
     */
    virtual ~ReportScheduler() = default;

    /// @brief Check if a ReadHandler is scheduled for reporting
    virtual bool IsReportScheduled(ReadHandler * aReadHandler) = 0;
    /// @brief Check whether a ReadHandler is reportable right now, taking into account its minimum and maximum intervals.
    /// @param aReadHandler read handler to check
    bool IsReportableNow(ReadHandler * aReadHandler) { return FindReadHandlerNode(aReadHandler)->IsReportableNow(); };
    /// @brief Check if a ReadHandler is reportable without considering the timing
    bool IsReadHandlerReportable(ReadHandler * aReadHandler) const
    {
        return aReadHandler->IsGeneratingReports() && aReadHandler->IsDirty();
    }

    /// @brief Get the number of ReadHandlers registered in the scheduler's node pool
    size_t GetNumReadHandlers() const { return mNodesPool.Allocated(); }

protected:
    friend class chip::app::reporting::TestReportScheduler;

    /// @brief Find the ReadHandlerNode for a given ReadHandler pointer
    /// @param [in] aReadHandler ReadHandler pointer to look for in the ReadHandler nodes list
    /// @return Node Address if node was found, nullptr otherwise
    ReadHandlerNode * FindReadHandlerNode(const ReadHandler * aReadHandler)
    {
        for (auto & iter : mReadHandlerList)
        {
            if (iter.GetReadHandler() == aReadHandler)
            {
                return &iter;
            }
        }
        return nullptr;
    }

    IntrusiveList<ReadHandlerNode> mReadHandlerList;
    ObjectPool<ReadHandlerNode, CHIP_IM_MAX_NUM_READS + CHIP_IM_MAX_NUM_SUBSCRIPTIONS> mNodesPool;
    TimerDelegate * mTimerDelegate;
};
}; // namespace reporting
}; // namespace app
}; // namespace chip
