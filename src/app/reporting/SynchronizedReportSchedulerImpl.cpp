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

using Seconds16       = System::Clock::Seconds16;
using Milliseconds32  = System::Clock::Milliseconds32;
using Timeout         = System::Clock::Timeout;
using Timestamp       = System::Clock::Timestamp;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;

/// @brief When a ReadHandler becomes reportable, recaculate the earliest possible interval before scheduling an engine run and
/// reschedule the report
void SynchronizedReportSchedulerImpl::OnBecameReportable(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);
    Milliseconds32 newTimeout;
    ReturnOnFailure(CalculateNextReportTimeout(newTimeout, node));
    ScheduleReport(newTimeout, node);
}

void SynchronizedReportSchedulerImpl::OnSubscriptionAction(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);

    // Update the node's timestamps
    node->SetIntervalTimeStamps(aReadHandler);

    Milliseconds32 newTimeout;
    ReturnOnFailure(CalculateNextReportTimeout(newTimeout, node));
    ScheduleReport(newTimeout, node);
}

CHIP_ERROR SynchronizedReportSchedulerImpl::RegisterReadHandler(ReadHandler * aReadHandler)
{
    ReadHandlerNode * newNode = FindReadHandlerNode(aReadHandler);
    // If the handler is already registered, no need to register it again
    VerifyOrReturnValue(nullptr == newNode, CHIP_NO_ERROR);
    newNode = mNodesPool.CreateObject(aReadHandler, mTimerDelegate, [this]() { this->ReportTimerCallback(); });
    mReadHandlerList.PushBack(newNode);

    ChipLogProgress(DataManagement,
                    "Registered ReadHandler that will schedule a report between system Timestamp: %" PRIu64
                    " and system Timestamp %" PRIu64 ".",
                    newNode->GetMinTimestamp().count(), newNode->GetMaxTimestamp().count());

    Milliseconds32 newTimeout;
    CHIP_ERROR err = CalculateNextReportTimeout(newTimeout, newNode);
    if (CHIP_NO_ERROR != err)
    {
        mReadHandlerList.Remove(newNode);
        mNodesPool.ReleaseObject(newNode);
        return err;
    }

    ReturnErrorOnFailure(ScheduleReport(newTimeout, newNode));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SynchronizedReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node)
{
    // Cancel Report if it is currently scheduled
    CancelSchedulerTimer(nullptr);

    ReturnErrorOnFailure(StartSchedulerTimer(nullptr, timeout));
    mNextReportTimestamp = mTimerDelegate->GetCurrentMonotonicTimestamp() + timeout;

    return CHIP_NO_ERROR;
}

void SynchronizedReportSchedulerImpl::CancelReport(ReadHandler * aReadHandler)
{
    // We don't need to take action on the handler, since the timer is common here
    CancelSchedulerTimer(nullptr);
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
    return CheckSchedulerTimerActive(nullptr);
}

CHIP_ERROR SynchronizedReportSchedulerImpl::StartSchedulerTimer(ReadHandlerNode * node, System::Clock::Timeout aTimeout)
{
    return mTimerDelegate->StartTimer(this, aTimeout);
}
void SynchronizedReportSchedulerImpl::CancelSchedulerTimer(ReadHandlerNode * node)
{
    mTimerDelegate->CancelTimer(this);
}
bool SynchronizedReportSchedulerImpl::CheckSchedulerTimerActive(ReadHandlerNode * node)
{
    return mTimerDelegate->IsTimerActive(this);
}

/// @brief Find the smallest maximum interval possible and set it as the common maximum
/// @return NO_ERROR if the smallest maximum interval was found, error otherwise, INVALID LIST LENGTH if the list is empty
CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMaxInterval()
{
    System::Clock::Timestamp now      = mTimerDelegate->GetCurrentMonotonicTimestamp();
    System::Clock::Timestamp earliest = now + Milliseconds64(kSubscriptionMaxIntervalPublisherLimit);
    VerifyOrReturnError(!mReadHandlerList.Empty(), CHIP_ERROR_INVALID_LIST_LENGTH);
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
    System::Clock::Timestamp now    = mTimerDelegate->GetCurrentMonotonicTimestamp();
    System::Clock::Timestamp latest = now;

    VerifyOrReturnError(!mReadHandlerList.Empty(), CHIP_ERROR_INVALID_LIST_LENGTH);
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

CHIP_ERROR SynchronizedReportSchedulerImpl::CalculateNextReportTimeout(System::Clock::Timeout & timeout, ReadHandlerNode * aNode)
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
            timeout = Milliseconds32(mNextMinTimestamp.count() - now.count());
        }
    }
    else
    {
        timeout = Milliseconds32(mNextMaxTimestamp.count() - now.count());
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
#ifndef CONFIG_BUILD_FOR_HOST_UNIT_TEST
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
#else
    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
    ChipLogProgress(DataManagement, "Engine run at time: %" PRIu64 " for Handlers:", now.count());

    for (auto & iter : mReadHandlerList)
    {
        if (iter.IsReportableNow())
        {
            ChipLogProgress(DataManagement, "Handler: %p" PRIu64 " with min: %" PRIu64 " and max: %" PRIu64 " and sync: %" PRIu64,
                            (&iter), iter.GetMinTimestamp().count(), iter.GetMaxTimestamp().count(),
                            iter.GetSyncTimestamp().count());
        }
    }
#endif
}

} // namespace reporting
} // namespace app
} // namespace chip
