/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    // ReadHandlerObserver
    void OnSubscriptionEstablished(ReadHandler * aReadHandler) final;
    void OnBecameReportable(ReadHandler * aReadHandler) final;
    void OnSubscriptionReportSent(ReadHandler * aReadHandler) final;
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    bool IsReportScheduled(ReadHandler * aReadHandler);

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
