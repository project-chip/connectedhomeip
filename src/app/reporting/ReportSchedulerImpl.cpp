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

using Seconds16      = System::Clock::Seconds16;
using Milliseconds32 = System::Clock::Milliseconds32;
using Timeout        = System::Clock::Timeout;
using Timestamp      = System::Clock::Timestamp;

/// @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers, as
/// the engine already verifies that read handlers are reportable before sending a report
static void ReportTimerCallback(System::Layer * aLayer, void * aAppState)
{
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
}

/// @brief When a ReadHandler is added, register it, which will schedule an engine run on the max interval
void ReportSchedulerImpl::OnReadHandlerAdded(ReadHandler * aReadHandler)
{
    RegisterReadHandler(aReadHandler);
}

/// @brief When a ReadHandler becomes reportable, schedule, verifies if the handler is the min interval is elapsed. If not,
/// reschedule the report to happen when the min interval is elapsed. If it is, schedule an engine run.
void ReportSchedulerImpl::OnBecameReportable(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);
    if (node->IsReportableNow())
    {
        // If the handler is reportable now, just schedule a report immediately
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
    else
    {
        Milliseconds32 newTimeout =
            Milliseconds32(node->GetMinInterval().count() - System::SystemClock().GetMonotonicTimestamp().count());
        // If the handler is not reportable now, schedule a report for the min interval
        ScheduleReport(newTimeout, aReadHandler);
    }
}

void ReportSchedulerImpl::OnReportSent(ReadHandler * apReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(apReadHandler);
    VerifyOrReturn(nullptr != node);
    // Schedule callback for max interval
    node->SetIntervalsTimeStamp(apReadHandler);
    Milliseconds32 newTimeout =
        Milliseconds32(node->GetMaxInterval().count() - System::SystemClock().GetMonotonicTimestamp().count());
    ScheduleReport(newTimeout, apReadHandler);
}

/// @brief When a ReadHandler is removed, unregister it, which will cancel any scheduled report
void ReportSchedulerImpl::OnReadHandlerRemoved(ReadHandler * aReadHandler)
{
    UnregisterReadHandler(aReadHandler);
}

CHIP_ERROR ReportSchedulerImpl::RegisterReadHandler(ReadHandler * aReadHandler)
{
    ReadHandlerNode * newNode = FindReadHandlerNode(aReadHandler);
    // If the handler is already registered, no need to register it again
    VerifyOrReturnValue(nullptr == newNode, CHIP_NO_ERROR);
    newNode = mNodesPool.CreateObject(aReadHandler);
    mReadHandlerList.PushBack(newNode);

    ChipLogProgress(DataManagement,
                    "Registered ReadHandler that will schedule a report between system Timestamp: %" PRIu64
                    " and system Timestamp %" PRIu64 ".",
                    newNode->GetMinInterval().count(), newNode->GetMaxInterval().count());

    Timestamp now = System::SystemClock().GetMonotonicTimestamp();
    // If the handler is reportable, schedule a report for the min interval, otherwise schdule a report for the max interval
    if ((newNode->IsReportableNow()))
    {
        // If the handler is reportable now, just schedule a report immediately
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
    else if (IsReadHandlerReportable(aReadHandler) && (newNode->GetMinInterval() > now))
    {
        Milliseconds32 newTimeout = Milliseconds32(newNode->GetMinInterval().count() - now.count());
        // If the handler is reportable now, but the min interval is not elapsed, schedule a report for the min interval
        ReturnErrorOnFailure(ScheduleReport(newTimeout, aReadHandler));
    }
    else
    {
        Milliseconds32 newTimeout = Milliseconds32(newNode->GetMaxInterval().count() - now.count());
        // If the handler is not reportable now, schedule a report for the max interval
        ReturnErrorOnFailure(ScheduleReport(newTimeout, aReadHandler));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReportSchedulerImpl::ScheduleReport(Timeout timeout, ReadHandler * aReadHandler)
{
    // Verify the handler is still registered
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturnError(nullptr != node, CHIP_ERROR_NOT_FOUND);

    // Cancel Report if it is currently scheduled
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        ReportTimerCallback, node);

    if (!IsChunkedReport(aReadHandler))
    {
        // Schedule Report
        ReturnErrorOnFailure(
            InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
                timeout, ReportTimerCallback, node));
    }

    return CHIP_NO_ERROR;
}

void ReportSchedulerImpl::CancelReport(ReadHandler * aReadHandler)
{
    ReadHandlerNode * node = FindReadHandlerNode(aReadHandler);
    VerifyOrReturn(nullptr != node);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        ReportTimerCallback, node);
}

void ReportSchedulerImpl::UnregisterReadHandler(ReadHandler * aReadHandler)
{
    // Verify list is populated and handler is not null
    VerifyOrReturn((!mReadHandlerList.Empty() || (nullptr == aReadHandler)));
    CancelReport(aReadHandler);

    ReadHandlerNode * removeNode = FindReadHandlerNode(aReadHandler);
    // Nothing to remove if the handler is not found in the list
    VerifyOrReturn(nullptr != removeNode);

    mReadHandlerList.Remove(removeNode);
    mNodesPool.ReleaseObject(removeNode);
}

void ReportSchedulerImpl::UnregisterAllHandlers()
{
    // Verify list is populated
    VerifyOrReturn(!mReadHandlerList.Empty());
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
    return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->IsTimerActive(
        ReportTimerCallback, node);
}

ReportSchedulerImpl::ReadHandlerNode * ReportSchedulerImpl::FindReadHandlerNode(const ReadHandler * aReadHandler)
{
    // Verify list is populated and handler is not null
    VerifyOrReturnValue(!mReadHandlerList.Empty() && (nullptr != aReadHandler), nullptr);
    for (auto & iter : mReadHandlerList)
    {
        if (iter.GetReadHandler() == aReadHandler)
        {
            return &iter;
        }
    }
    return nullptr;
}

} // namespace reporting
} // namespace app
} // namespace chip
