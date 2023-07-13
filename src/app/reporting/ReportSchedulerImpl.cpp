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
#include <app/reporting/ReportSchedulerImpl.h>

namespace chip {
namespace app {
namespace reporting {

using Seconds16       = System::Clock::Seconds16;
using Milliseconds32  = System::Clock::Milliseconds32;
using Timeout         = System::Clock::Timeout;
using Timestamp       = System::Clock::Timestamp;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;

/// @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers, as
/// the engine already verifies that read handlers are reportable before sending a report
static void ReportTimerCallback()
{
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
}

ReportSchedulerImpl::ReportSchedulerImpl(TimerDelegate * aTimerDelegate) : ReportScheduler(aTimerDelegate)
{
    VerifyOrDie(nullptr != mTimerDelegate);
}

/// @brief When a ReadHandler is added, register it, which will schedule an engine run
void ReportSchedulerImpl::OnReadHandlerCreated(ReadHandler * aReadHandler)
{
    RegisterReadHandler(aReadHandler);
}

/// @brief When a ReadHandler becomes reportable, schedule, verifies if the min interval of a handleris elapsed. If not,
/// reschedule the report to happen when the min interval is elapsed. If it is, schedule an engine run.
void ReportSchedulerImpl::OnBecameReportable(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);

    Milliseconds32 newTimeout;
    if (node->IsReportableNow())
    {
        // If the handler is reportable now, just schedule a report immediately
        newTimeout = Milliseconds32(0);
    }
    else
    {
        // If the handler is not reportable now, schedule a report for the min interval
        newTimeout = node->GetMinTimestamp() - mTimerDelegate->GetCurrentMonotonicTimestamp();
    }

    ScheduleReport(newTimeout, node);
}

void ReportSchedulerImpl::OnSubscriptionAction(ReadHandler * apReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(apReadHandler);
    VerifyOrReturn(nullptr != node);
    // Schedule callback for max interval by computing the difference between the max timestamp and the current timestamp
    node->SetIntervalTimeStamps(apReadHandler);
    Milliseconds32 newTimeout = node->GetMaxTimestamp() - mTimerDelegate->GetCurrentMonotonicTimestamp();
    ScheduleReport(newTimeout, node);
}

/// @brief When a ReadHandler is removed, unregister it, which will cancel any scheduled report
void ReportSchedulerImpl::OnReadHandlerDestroyed(ReadHandler * aReadHandler)
{
    UnregisterReadHandler(aReadHandler);
}

CHIP_ERROR ReportSchedulerImpl::RegisterReadHandler(ReadHandler * aReadHandler)
{
    ReadHandlerNode * newNode = FindReadHandlerNode(aReadHandler);
    // Handler must not be registered yet; it's just being constructed.
    VerifyOrDie(nullptr == newNode);
    // The NodePool is the same size as the ReadHandler pool from the IM Engine, so we don't need a check for size here since if a
    // ReadHandler was created, space should be available.
    newNode = mNodesPool.CreateObject(aReadHandler, mTimerDelegate, ReportTimerCallback);
    mReadHandlerList.PushBack(newNode);

    ChipLogProgress(DataManagement,
                    "Registered a ReadHandler that will schedule a report between system Timestamp: %" PRIu64
                    " and system Timestamp %" PRIu64 ".",
                    newNode->GetMinTimestamp().count(), newNode->GetMaxTimestamp().count());

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
    Milliseconds32 newTimeout;
    // If the handler is reportable, schedule a report for the min interval, otherwise schedule a report for the max interval
    if (newNode->IsReportableNow())
    {
        // If the handler is reportable now, just schedule a report immediately
        newTimeout = Milliseconds32(0);
    }
    else if (IsReadHandlerReportable(aReadHandler) && (newNode->GetMinTimestamp() > now))
    {
        // If the handler is reportable now, but the min interval is not elapsed, schedule a report for the moment the min interval
        // has elapsed
        newTimeout = newNode->GetMinTimestamp() - now;
    }
    else
    {
        // If the handler is not reportable now, schedule a report for the max interval
        newTimeout = newNode->GetMaxTimestamp() - now;
    }

    ReturnErrorOnFailure(ScheduleReport(newTimeout, newNode));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node)
{
    // Cancel Report if it is currently scheduled
    CancelSchedulerTimer(node);
    StartSchedulerTimer(node, timeout);

    return CHIP_NO_ERROR;
}

void ReportSchedulerImpl::CancelReport(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);
    CancelSchedulerTimer(node);
}

void ReportSchedulerImpl::UnregisterReadHandler(ReadHandler * aReadHandler)
{
    CancelReport(aReadHandler);

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mReadHandlerList.Remove(removeNode);
    mNodesPool.ReleaseObject(removeNode);
}

void ReportSchedulerImpl::UnregisterAllHandlers()
{
    while (!mReadHandlerList.Empty())
    {
        ReadHandler * firstReadHandler = mReadHandlerList.begin()->GetReadHandler();
        UnregisterReadHandler(firstReadHandler);
    }
}

bool ReportSchedulerImpl::IsReportScheduled(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturnValue(nullptr != node, false);
    return CheckSchedulerTimerActive(node);
}

CHIP_ERROR ReportSchedulerImpl::StartSchedulerTimer(ReadHandlerNode * node, System::Clock::Timeout aTimeout)
{
    // Schedule Report
    return mTimerDelegate->StartTimer(node, aTimeout);
}

void ReportSchedulerImpl::CancelSchedulerTimer(ReadHandlerNode * node)
{
    mTimerDelegate->CancelTimer(node);
}

bool ReportSchedulerImpl::CheckSchedulerTimerActive(ReadHandlerNode * node)
{
    return mTimerDelegate->IsTimerActive(node);
}

} // namespace reporting
} // namespace app
} // namespace chip
