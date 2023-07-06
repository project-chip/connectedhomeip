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

// Forward declaration of TestReportScheduler to allow it to be friend with ReportScheduler
class TestReportScheduler;

namespace reporting {

class ReportScheduler : public ReadHandler::Observer
{
public:
    class ReadHandlerNode : public IntrusiveListNodeBase<>
    {
    public:
        using TimerCompleteCallback = void (*)();

        ReadHandlerNode(ReadHandler * aReadHandler, TimerCompleteCallback aCallback) : mCallback(aCallback)
        {
            mReadHandler = aReadHandler;
            SetIntervalsTimeStamp(aReadHandler);
        }
        ReadHandler * GetReadHandler() const { return mReadHandler; }
        /// @brief Check if the Node is reportable now, meaning its readhandler was made reportable by attribute dirtying and
        /// handler state, and minimal time interval last reporthas elapsed since, or the maximal time interval since last report
        /// has elapsed
        bool IsReportableNow() const
        {
            // TODO: Add flags to allow for test to simulate waiting for the min interval or max intrval to elapse when integrating
            // the scheduler in the ReadHandler
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
            return (mReadHandler->IsGeneratingReports() &&
                    ((now >= mMinTimestamp && mReadHandler->IsDirty()) || now >= mMaxTimestamp));
        }

        void SetIntervalsTimeStamp(ReadHandler * aReadHandler)
        {
            uint16_t minInterval, maxInterval;
            aReadHandler->GetReportingIntervals(minInterval, maxInterval);
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
            mMinTimestamp                = now + System::Clock::Seconds16(minInterval);
            mMaxTimestamp                = now + System::Clock::Seconds16(maxInterval);
        }

        void RunCallback() { mCallback(); }

        System::Clock::Timestamp GetMinTimestamp() const { return mMinTimestamp; }
        System::Clock::Timestamp GetMaxTimestamp() const { return mMaxTimestamp; }

    private:
        TimerCompleteCallback mCallback;
        ReadHandler * mReadHandler;
        System::Clock::Timestamp mMinTimestamp;
        System::Clock::Timestamp mMaxTimestamp;
    };

    class TimerDelegate
    {
    public:
        virtual ~TimerDelegate() {}
        virtual CHIP_ERROR StartTimer(void * context, System::Clock::Timeout aTimeout) = 0;
        virtual void CancelTimer(void * context)                                       = 0;
        virtual bool IsTimerActive(void * context)                                     = 0;
    };

    ReportScheduler(TimerDelegate * aTimerDelegate) : mTimerDelegate(aTimerDelegate) {}
    /**
     *  Interface to act on changes in the ReadHandler reportability
     */
    virtual ~ReportScheduler() = default;

    /// @brief Register a ReadHandler to the scheduler, will schedule report
    /// @param aReadHandler read handler to register
    virtual CHIP_ERROR RegisterReadHandler(ReadHandler * aReadHandler) = 0;
    /// @brief Schedule a report for a given ReadHandler
    /// @param timeout time in seconds before the next engine run is scheduled
    /// @param aReadHandler read handler to schedule a report for
    /// @return CHIP_ERROR not found if the ReadHandler is not registered in the scheduler, specific timer error if the timer
    /// couldn't be started
    virtual CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandler * aReadHandler) = 0;
    /// @brief Cancel a scheduled report for a given ReadHandler
    /// @param aReadHandler readhandler to look for in the ReadHandler nodes list. If found, the timer started for this report will
    /// be cancelled.
    virtual void CancelReport(ReadHandler * aReadHandler) = 0;
    /// @brief Unregister a ReadHandler from the scheduler
    /// @param aReadHandler read handler to unregister
    /// @return CHIP_NO_ERROR if the ReadHandler was successfully unregistered or not found, specific error otherwise
    virtual void UnregisterReadHandler(ReadHandler * aReadHandler) = 0;
    /// @brief Unregister all ReadHandlers from the scheduler
    /// @return CHIP_NO_ERROR if all ReadHandlers were successfully unregistered, specific error otherwise
    virtual void UnregisterAllHandlers() = 0;
    /// @brief Check if a ReadHandler is scheduled for reporting
    virtual bool IsReportScheduled(ReadHandler * aReadHandler) = 0;
    /// @brief Check if a ReadHandler is reportable given its minimal and maximal intervals by using the node timestamps
    /// @param aReadHandler read handler to check
    bool IsReportableNow(ReadHandler * aReadHandler) { return FindReadHandlerNode(aReadHandler)->IsReportableNow(); };
    /// @brief Check if a ReadHandler is reportable without considering the timing
    bool IsReadHandlerReportable(ReadHandler * aReadHandler) const
    {
        return aReadHandler->IsGeneratingReports() && aReadHandler->IsDirty();
    }
    /// @brief Check the ReadHandler's ChunkedReport flag to prevent rescheduling if the Schedule is called when the engine is
    /// processing a chunked report
    bool IsChunkedReport(ReadHandler * aReadHandler) const { return aReadHandler->IsChunkedReport(); }

    /// @brief Get the number of ReadHandlers registered in the scheduler's node pool
    size_t GetNumReadHandlers() const { return mNodesPool.Allocated(); }

protected:
    friend class chip::app::TestReportScheduler;

    /// @brief Find the ReadHandlerNode for a given ReadHandler pointer
    /// @param [in] aReadHandler ReadHandler pointer to look for in the ReadHandler nodes list
    /// @return Node Address if node was found, nullptr otherwise
    virtual ReadHandlerNode * FindReadHandlerNode(const ReadHandler * aReadHandler) = 0;

    virtual CHIP_ERROR StartTimerForHandler(ReadHandlerNode * node, System::Clock::Timeout aTimeout) = 0;
    virtual void CancelTimerForHandler(ReadHandlerNode * node)                                       = 0;
    virtual bool CheckTimerActiveForHandler(ReadHandlerNode * node)                                  = 0;

    IntrusiveList<ReadHandlerNode> mReadHandlerList;
    ObjectPool<ReadHandlerNode, CHIP_IM_MAX_NUM_READS + CHIP_IM_MAX_NUM_SUBSCRIPTIONS> mNodesPool;
    TimerDelegate * mTimerDelegate;
};
}; // namespace reporting
}; // namespace app
}; // namespace chip
