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

#include <app/AppConfig.h>
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

/// @brief Method that triggers a report emission on each ReadHandler that is not blocked on its min interval.
///        Each read handler that is not blocked is immediately marked dirty so that it will report as soon as possible.
void ReportSchedulerImpl::OnEnterActiveMode()
{
#if ICD_REPORT_ON_ENTER_ACTIVE_MODE
    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();
    mNodesPool.ForEachActiveObject([this, now](ReadHandlerNode * node) {
        if (now >= node->GetMinTimestamp())
        {
            this->HandlerForceDirtyState(node->GetReadHandler());
        }

        return Loop::Continue;
    });
#endif
}

void ReportSchedulerImpl::OnSubscriptionEstablished(ReadHandler * aReadHandler)
{
    ReadHandlerNode * newNode = FindReadHandlerNode(aReadHandler);
    // Handler must not be registered yet; it's just being constructed.
    VerifyOrDie(nullptr == newNode);

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

    // The NodePool is the same size as the ReadHandler pool from the IM Engine, so we don't need a check for size here since if a
    // ReadHandler was created, space should be available.
    newNode = mNodesPool.CreateObject(aReadHandler, this, now);

    ChipLogProgress(DataManagement,
                    "Registered a ReadHandler that will schedule a report between system Timestamp: 0x" ChipLogFormatX64
                    " and system Timestamp 0x" ChipLogFormatX64 ".",
                    ChipLogValueX64(newNode->GetMinTimestamp().count()), ChipLogValueX64(newNode->GetMaxTimestamp().count()));
}

void ReportSchedulerImpl::OnBecameReportable(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

    Milliseconds32 newTimeout;
    CalculateNextReportTimeout(newTimeout, node, now);
    ScheduleReport(newTimeout, node, now);
}

void ReportSchedulerImpl::OnSubscriptionReportSent(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);

    Timestamp now = mTimerDelegate->GetCurrentMonotonicTimestamp();

    // This method is called after the report is sent, so the ReadHandler is no longer reportable, and thus CanBeSynced and
    // EngineRunScheduled of the node associated with the ReadHandler are set to false here.
    node->SetCanBeSynced(false);
    node->SetIntervalTimeStamps(aReadHandler, now);
    Milliseconds32 newTimeout;
    // Reset the EngineRunScheduled flag so that the next report is scheduled correctly
    node->SetEngineRunScheduled(false);
    CalculateNextReportTimeout(newTimeout, node, now);
    ScheduleReport(newTimeout, node, now);
}

void ReportSchedulerImpl::OnReadHandlerDestroyed(ReadHandler * aReadHandler)
{
    CancelReport(aReadHandler);

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mNodesPool.ReleaseObject(removeNode);
}

CHIP_ERROR ReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandlerNode * node, const Timestamp & now)
{
    // Cancel Report if it is currently scheduled
    mTimerDelegate->CancelTimer(node);
    if (timeout == Milliseconds32(0))
    {
        node->TimerFired();
        return CHIP_NO_ERROR;
    }
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
    mNodesPool.ForEachActiveObject([this](ReadHandlerNode * node) {
        this->OnReadHandlerDestroyed(node->GetReadHandler());
        return Loop::Continue;
    });
}

bool ReportSchedulerImpl::IsReportScheduled(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturnValue(nullptr != node, false);
    return mTimerDelegate->IsTimerActive(node);
}

CHIP_ERROR ReportSchedulerImpl::CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode, const Timestamp & now)
{
    VerifyOrReturnError(nullptr != FindReadHandlerNode(aNode->GetReadHandler()), CHIP_ERROR_INVALID_ARGUMENT);

    // If the handler is reportable now, just schedule a report immediately
    if (aNode->IsReportableNow(now))
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
