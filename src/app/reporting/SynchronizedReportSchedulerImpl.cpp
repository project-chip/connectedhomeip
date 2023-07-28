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

CHIP_ERROR SynchronizedReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node)
{
    // Cancel Report if it is currently scheduled
    mTimerDelegate->CancelTimer(this);
    if (timeout == Milliseconds32(0))
    {
        ReportTimerCallback();
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(mTimerDelegate->StartTimer(this, timeout));
    mTestNextReportTimestamp = mTimerDelegate->GetCurrentMonotonicTimestamp() + timeout;

    return CHIP_NO_ERROR;
}

void SynchronizedReportSchedulerImpl::CancelReport()
{
    // We don't need to take action on the handler, since the timer is common here
    mTimerDelegate->CancelTimer(this);
}

/// @brief Checks if the timer is active for the ReportScheduler
bool SynchronizedReportSchedulerImpl::IsReportScheduled()
{
    return mTimerDelegate->IsTimerActive(this);
}

/// @brief Find the smallest maximum interval possible and set it as the common maximum
/// @return NO_ERROR if the smallest maximum interval was found, error otherwise, INVALID LIST LENGTH if the list is empty
CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMaxInterval()
{
    VerifyOrReturnError(mNodesPool.Allocated(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp now      = mTimerDelegate->GetCurrentMonotonicTimestamp();
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

/// @brief Find the highest minimum timestamp possible that still respects the lowest max timestamp and sets it as the common
/// minimum. If the max timestamp has not been updated and is in the past, or if no min timestamp is lower than the current max
/// timestamp, this will set now as the common minimum timestamp, thus allowing the report to be sent immediately.
/// @return NO_ERROR if the highest minimum timestamp was found, error otherwise, INVALID LIST LENGTH if the list is empty
CHIP_ERROR SynchronizedReportSchedulerImpl::FindNextMinInterval()
{
    VerifyOrReturnError(mNodesPool.Allocated(), CHIP_ERROR_INVALID_LIST_LENGTH);
    System::Clock::Timestamp latest = mTimerDelegate->GetCurrentMonotonicTimestamp();

    mNodesPool.ForEachActiveObject([&latest, this](ReadHandlerNode * node) {
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

CHIP_ERROR SynchronizedReportSchedulerImpl::CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode)
{
    VerifyOrReturnError(nullptr != FindReadHandlerNode(aNode->GetReadHandler()), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(FindNextMaxInterval());
    ReturnErrorOnFailure(FindNextMinInterval());
    bool reportableNow   = false;
    bool reportableAtMin = false;

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

    mNodesPool.ForEachActiveObject([&reportableNow, &reportableAtMin, this](ReadHandlerNode * node) {
        if (!node->IsEngineRunScheduled())
        {
            if (node->IsReportableNow())
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

    // Find out if any handler is reportable now

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

    // Updates the synching time of each handler
    mNodesPool.ForEachActiveObject([now, timeout](ReadHandlerNode * node) {
        // Prevent modifying the sync if the handler is currently reportable, sync's purpose is to allow handler to become
        // reportable earlier than their max interval
        if (!node->IsReportableNow())
        {
            node->SetSyncTimestamp(Milliseconds64(now + timeout));
        }

        return Loop::Continue;
    });

    return CHIP_NO_ERROR;
}

/// @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers, as
/// the engine already verifies that read handlers are reportable before sending a report
void SynchronizedReportSchedulerImpl::TimerFired()
{
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();

    mNodesPool.ForEachActiveObject([](ReadHandlerNode * node) {
        if (node->IsReportableNow())
        {
            node->SetEngineRunScheduled(true);
            ChipLogProgress(DataManagement, "Handler: %p with min: %" PRIu64 " and max: %" PRIu64 " and sync: %" PRIu64, (node),
                            node->GetMinTimestamp().count(), node->GetMaxTimestamp().count(), node->GetSyncTimestamp().count());
        }

        return Loop::Continue;
    });
}

} // namespace reporting
} // namespace app
} // namespace chip
