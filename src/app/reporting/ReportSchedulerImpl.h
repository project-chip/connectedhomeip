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

#include <app/reporting/ReportScheduler.h>

namespace chip {
namespace app {
namespace reporting {

class ReportSchedulerImpl : public ReportScheduler
{
public:
    using Timeout = System::Clock::Timeout;

    ReportSchedulerImpl(TimerDelegate * aTimerDelegate);
    ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ICDStateObserver
    void OnEnterActiveMode() override;
    void OnTransitionToIdle() override;
    // No action is needed by the ReportScheduler on ICD operation Mode changes
    void OnICDModeChange() override{};

    // ReadHandlerObserver
    void OnSubscriptionEstablished(ReadHandler * aReadHandler) final;
    void OnBecameReportable(ReadHandler * aReadHandler) final;
    void OnSubscriptionReportSent(ReadHandler * aReadHandler) final;
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    virtual bool IsReportScheduled(ReadHandler * aReadHandler);

    void ReportTimerCallback() override;

protected:
    virtual CHIP_ERROR ScheduleReport(Timeout timeout, ReadHandlerNode * node, const Timestamp & now);
    void CancelReport(ReadHandler * aReadHandler);
    virtual void UnregisterAllHandlers();

private:
    friend class chip::app::reporting::TestReportScheduler;

    virtual CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode, const Timestamp & now);
};

} // namespace reporting
} // namespace app
} // namespace chip
