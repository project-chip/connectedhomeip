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

CHIP_ERROR SynchronizedReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node)
{
    // Cancel Report if it is currently scheduled
    mTimerDelegate->CancelTimer(this);
    ReturnErrorOnFailure(mTimerDelegate->StartTimer(this, timeout));
    mTestNextReportTimestamp = mTimerDelegate->GetCurrentMonotonicTimestamp() + timeout;

    return CHIP_NO_ERROR;
}

void SynchronizedReportSchedulerImpl::CancelReport(ReadHandler * aReadHandler)
{
    // We don't need to take action on the handler, since the timer is common here
    mTimerDelegate->CancelTimer(this);
}

void SynchronizedReportSchedulerImpl::UnregisterReadHandler(ReadHandler * aReadHandler)
{
    // Verify list is populated and handler is not null
    VerifyOrReturn((!mReadHandlerList.Empty() || (nullptr == aReadHandler)));

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mReadHandlerList.Remove(removeNode);
    mNodesPool.ReleaseObject(removeNode);

    if (mReadHandlerList.Empty())
    {
        // Only cancel the timer if there are no more handlers registered
        CancelReport(aReadHandler);
    }
}

/// @brief Checks if the timer is active for the given ReadHandler. Since all read handlers are scheduled on the same timer, we
/// check if the node is in the list and if the timer is active for the ReportScheduler
bool SynchronizedReportSchedulerImpl::IsReportScheduled(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturnValue(nullptr != node, false);
    return mTimerDelegate->IsTimerActive(this);
}

/// @brief Find the smallest maximum interval possible and set it as the common maximum
/// @return NO_ERROR if the smallest maximum interval was found, error otherwise, INVALID LIST LENGTH if the list is empty
CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMaxInterval()
{
    VerifyOrReturnError(!mReadHandlerList.Empty(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp now      = mTimerDelegate->GetCurrentMonotonicTimestamp();
    System::Clock::Timestamp earliest = now + Seconds16(kSubscriptionMaxIntervalPublisherLimit);

    for (auto & iter : mReadHandlerList)
    {
        if (iter.GetMaxTimestamp() < earliest && iter.GetMaxTimestamp() > now)
        {
            earliest = iter.GetMaxTimestamp();
        }
    }

    mNextMaxTimestamp = earliest;

    return CHIP_NO_ERROR;
}

/// @brief Find the highest minimum timestamp possible that still respects the lowest max timestamp and sets it as the common
/// minimum. If the max timestamp has not been updated and is in the past, or if no min timestamp is lower than the current max
/// timestamp, this will set now as the common minimum timestamp, thus allowing the report to be sent immediately.
/// @return NO_ERROR if the highest minimum timestamp was found, error otherwise, INVALID LIST LENGTH if the list is empty
CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMinInterval()
{
    VerifyOrReturnError(!mReadHandlerList.Empty(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp latest = mTimerDelegate->GetCurrentMonotonicTimestamp();

    for (auto & iter : mReadHandlerList)
    {
        if (iter.GetMinTimestamp() > latest)
        {
            // We do not want the new min to be set above the max for any handler
            if (iter.GetMinTimestamp() <= mNextMaxTimestamp)
            {
                latest = iter.GetMinTimestamp();
            }
        }
    }

    mNextMinTimestamp = latest;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SynchronizedReportSchedulerImpl::CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode)
{
    VerifyOrReturnError(mReadHandlerList.Contains(aNode), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(FindNextMaxInterval());
    ReturnErrorOnFailure(FindNextMinInterval());

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
    // Find out if any handler is reportable now
    if (IsReadHandlerReportable(aNode->GetReadHandler()))
    {
        if (now > mNextMinTimestamp)
        {
            timeout = Milliseconds32(0);
        }
        else
        {
            timeout = mNextMinTimestamp - now;
        }
    }
    else
    {
        timeout = mNextMaxTimestamp - now;
    }

    // Updates the synching time of each handler
    for (auto & iter : mReadHandlerList)
    {
        // Prevent modifying the sync if the handler is currently reportable, sync's purpose is to allow handler to become
        // reportable earlier than their max interval
        if (!iter.IsReportableNow())
        {
            iter.SetSyncTimestamp(Milliseconds64(now + timeout));
        }
    }

    return CHIP_NO_ERROR;
}

/// @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers, as
/// the engine already verifies that read handlers are reportable before sending a report
void SynchronizedReportSchedulerImpl::ReportTimerCallback()
{
    ReportSchedulerImpl::ReportTimerCallback();

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
    ChipLogProgress(DataManagement, "Engine run at time: %" PRIu64 " for Handlers:", now.count());
    for (auto & iter : mReadHandlerList)
    {
        if (iter.IsReportableNow())
        {
            ChipLogProgress(DataManagement, "Handler: %p with min: %" PRIu64 " and max: %" PRIu64 " and sync: %" PRIu64, (&iter),
                            iter.GetMinTimestamp().count(), iter.GetMaxTimestamp().count(), iter.GetSyncTimestamp().count());
        }
    }
}

} // namespace reporting
} // namespace app
} // namespace chip
