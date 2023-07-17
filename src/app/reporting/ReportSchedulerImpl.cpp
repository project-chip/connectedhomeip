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

using namespace System::Clock;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;

/// @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers, as
/// the engine already verifies that read handlers are reportable before sending a report
void ReportSchedulerImpl::ReportTimerCallback()
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
    ReadHandlerNode * newNode = FindReadHandlerNode(aReadHandler);
    // Handler must not be registered yet; it's just being constructed.
    VerifyOrDie(nullptr == newNode);
    // The NodePool is the same size as the ReadHandler pool from the IM Engine, so we don't need a check for size here since if a
    // ReadHandler was created, space should be available.
    newNode = mNodesPool.CreateObject(aReadHandler, mTimerDelegate, [this]() { this->ReportTimerCallback(); });
    mReadHandlerList.PushBack(newNode);

    ChipLogProgress(DataManagement,
                    "Registered a ReadHandler that will schedule a report between system Timestamp: %" PRIu64
                    " and system Timestamp %" PRIu64 ".",
                    newNode->GetMinTimestamp().count(), newNode->GetMaxTimestamp().count());

    Milliseconds32 newTimeout;
    // No need to check for error here, since the node is already in the list otherwise we would have Died
    CalculateNextReportTimeout(newTimeout, newNode);
    ScheduleReport(newTimeout, newNode);
}

/// @brief When a ReadHandler becomes reportable, schedule, verifies if the min interval of a handleris elapsed. If not,
/// reschedule the report to happen when the min interval is elapsed. If it is, schedule an engine run.
void ReportSchedulerImpl::OnBecameReportable(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);

    Milliseconds32 newTimeout;
    CalculateNextReportTimeout(newTimeout, node);
    ScheduleReport(newTimeout, node);
}

void ReportSchedulerImpl::OnSubscriptionAction(ReadHandler * apReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(apReadHandler);
    VerifyOrReturn(nullptr != node);
    node->SetIntervalTimeStamps(apReadHandler);
    Milliseconds32 newTimeout;
    CalculateNextReportTimeout(newTimeout, node);
    ScheduleReport(newTimeout, node);
    node->SetEngineRunScheduled(false);
}

/// @brief When a ReadHandler is removed, unregister it, which will cancel any scheduled report
void ReportSchedulerImpl::OnReadHandlerDestroyed(ReadHandler * aReadHandler)
{
    CancelReport(aReadHandler);

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mReadHandlerList.Remove(removeNode);
    mNodesPool.ReleaseObject(removeNode);
}

CHIP_ERROR ReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node)
{
    // Cancel Report if it is currently scheduled
    mTimerDelegate->CancelTimer(node);
    ReturnErrorOnFailure(mTimerDelegate->StartTimer(node, timeout));

    return CHIP_NO_ERROR;
}

void ReportSchedulerImpl::CancelReport(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);
    mTimerDelegate->CancelTimer(node);
}

void ReportSchedulerImpl::UnregisterAllHandlers()
{
    while (!mReadHandlerList.Empty())
    {
        ReadHandler * firstReadHandler = mReadHandlerList.begin()->GetReadHandler();
        OnReadHandlerDestroyed(firstReadHandler);
    }
}

bool ReportSchedulerImpl::IsReportScheduled(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturnValue(nullptr != node, false);
    return mTimerDelegate->IsTimerActive(node);
}

CHIP_ERROR ReportSchedulerImpl::CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode)
{
    VerifyOrReturnError(mReadHandlerList.Contains(aNode), CHIP_ERROR_INVALID_ARGUMENT);
    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

    // If the handler is reportable now, just schedule a report immediately
    if (aNode->IsReportableNow())
    {
        // If the handler is reportable now, just schedule a report immediately
        timeout = Milliseconds32(0);
    }
    else if (IsReadHandlerReportable(aNode->GetReadHandler()) && (aNode->GetMinTimestamp() > now))
    {
        // If the handler is reportable now, but the min interval is not elapsed, schedule a report for the moment the min interval
        // has elapsed
        timeout = aNode->GetMinTimestamp() - now;
    }
    else
    {
        // If the handler is not reportable now, schedule a report for the max interval
        timeout = aNode->GetMaxTimestamp() - now;
    }
    return CHIP_NO_ERROR;
}

} // namespace reporting
} // namespace app
} // namespace chip
