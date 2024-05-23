/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/InteractionModelEngine.h>
#include <app/reporting/SynchronizedReportSchedulerImpl.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace reporting {

using namespace System::Clock;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;

void SynchronizedReportSchedulerImpl::OnReadHandlerDestroyed(ReadHandler * aReadHandler)
{
    // Verify list is populated
    VerifyOrReturn(mNodesPool.Allocated());

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mNodesPool.ReleaseObject(removeNode);

    if (!mNodesPool.Allocated())
    {
        // Only cancel the timer if there are no more handlers registered
        CancelReport();
    }
}

void SynchronizedReportSchedulerImpl::OnTransitionToIdle()
{
    Timestamp now               = mTimerDelegate->GetCurrentMonotonicTimestamp();
    uint32_t targetIdleInterval = static_cast<uint32_t>(ICD_SLEEP_TIME_JITTER_MS);
    VerifyOrReturn(now >= mNextReportTimestamp);
    if (((mNextReportTimestamp - now) < Seconds16(targetIdleInterval)) && (now > mNextMinTimestamp))
    {
        // If the next report is due in less than the idle mode duration and we are past the min interval, we can just send it now
        CancelReport();
        TimerFired();
    }
}

CHIP_ERROR SynchronizedReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node, const Timestamp & now)
{
    // Cancel Report if it is currently scheduled
    mTimerDelegate->CancelTimer(this);
    if (timeout == Milliseconds32(0))
    {
        TimerFired();
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(mTimerDelegate->StartTimer(this, timeout));
    mNextReportTimestamp = now + timeout;

    return CHIP_NO_ERROR;
}

void SynchronizedReportSchedulerImpl::CancelReport()
{
    // We don't need to take action on the handler, since the timer is common here
    mTimerDelegate->CancelTimer(this);
}

/// @brief Checks if the timer is active for the ReportScheduler
bool SynchronizedReportSchedulerImpl::IsReportScheduled(ReadHandler * ReadHandler)
{
    return mTimerDelegate->IsTimerActive(this);
}

CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMaxInterval(const Timestamp & now)
{
    VerifyOrReturnError(mNodesPool.Allocated(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp earliest = now + Seconds16::max();

    mNodesPool.ForEachActiveObject([&earliest, now](ReadHandlerNode * node) {
        if (node->GetMaxTimestamp() < earliest && node->GetMaxTimestamp() > now)
        {
            earliest = node->GetMaxTimestamp();
        }

        return Loop::Continue;
    });

    mNextMaxTimestamp = earliest;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMinInterval(const Timestamp & now)
{
    VerifyOrReturnError(mNodesPool.Allocated(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp latest = now;

    mNodesPool.ForEachActiveObject([&latest, this](ReadHandlerNode * node) {
        // We only consider the min interval if the handler is reportable. This is done to have only reportable handlers
        // contribute to setting the next min interval and avoid delaying a report for a handler that would not generate
        // a one on its min interval anyway.
        if (node->GetMinTimestamp() > latest && this->IsReadHandlerReportable(node->GetReadHandler()) &&
            node->GetMinTimestamp() <= this->mNextMaxTimestamp)
        {
            // We do not want the new min to be set above the max for any handler
            latest = node->GetMinTimestamp();
        }

        return Loop::Continue;
    });

    mNextMinTimestamp = latest;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SynchronizedReportSchedulerImpl::CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode,
                                                                       const Timestamp & now)
{
    ReturnErrorOnFailure(FindNextMaxInterval(now));
    ReturnErrorOnFailure(FindNextMinInterval(now));
    bool reportableNow   = false;
    bool reportableAtMin = false;

    // Find out if any handler is reportable now or at the next min interval
    mNodesPool.ForEachActiveObject([&reportableNow, &reportableAtMin, this, now](ReadHandlerNode * node) {
        // If a node is already scheduled, we don't need to check if it is reportable now unless a chunked report is in progress.
        // In this case, the node will be Reportable, as it is impossible to have node->IsChunkedReport() == true without being
        // reportable, therefore we need to keep scheduling engine runs until the report is complete
        if (!node->IsEngineRunScheduled() || node->IsChunkedReport())
        {
            if (node->IsReportableNow(now))
            {
                reportableNow = true;
                return Loop::Break;
            }

            if (this->IsReadHandlerReportable(node->GetReadHandler()) && node->GetMinTimestamp() <= this->mNextMaxTimestamp)
            {
                reportableAtMin = true;
            }
        }

        return Loop::Continue;
    });

    if (reportableNow)
    {
        timeout = Milliseconds32(0);
    }
    else if (reportableAtMin)
    {
        timeout = mNextMinTimestamp - now;
    }
    else
    {
        // Schedule report at next max otherwise
        timeout = mNextMaxTimestamp - now;
    }

    return CHIP_NO_ERROR;
}

void SynchronizedReportSchedulerImpl::TimerFired()
{
    Timestamp now   = mTimerDelegate->GetCurrentMonotonicTimestamp();
    bool firedEarly = true;

    // If there are no handlers registered, no need to do anything.
    VerifyOrReturn(mNodesPool.Allocated());

    mNodesPool.ForEachActiveObject([now, &firedEarly](ReadHandlerNode * node) {
        if (node->GetMinTimestamp() <= now)
        {
            // Since this handler can now report whenever it wants to, mark it as allowed to report if any other handler is
            // reporting using the CanBeSynced flag.
            node->SetCanBeSynced(true);
        }

        if (node->IsReportableNow(now))
        {
            // We set firedEarly false here because we assume we fired the timer early if no handler is reportable at the
            // moment, which becomes false if we find a handler that is reportable
            firedEarly = false;
            node->SetEngineRunScheduled(true);
            ChipLogProgress(DataManagement, "Handler: %p with min: 0x" ChipLogFormatX64 " and max: 0x" ChipLogFormatX64 "", (node),
                            ChipLogValueX64(node->GetMinTimestamp().count()), ChipLogValueX64(node->GetMaxTimestamp().count()));
        }

        return Loop::Continue;
    });

    if (firedEarly)
    {
        // If we fired the timer early, we need to recalculate the next report timeout and reschedule the report so it can run when
        // at least one read handler is reportable. Here we can't set the SetEngineRunScheduled flag to true, because this flag
        // allows handlers to generate reports before their min (assuming their min has elapsed from the timer's perspective but not
        // from the monotonic timer), and we don't know which handler was the one that should be reportable.
        Timeout timeout = Milliseconds32(0);
        ReturnOnFailure(CalculateNextReportTimeout(timeout, nullptr, now));
        ScheduleReport(timeout, nullptr, now);
    }
    else
    {
        // If we have a reportable handler, we can schedule an engine run
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
}

} // namespace reporting
} // namespace app
} // namespace chip
