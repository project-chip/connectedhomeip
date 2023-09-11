/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/reporting/ReportSchedulerImpl.h>

namespace chip {
namespace app {
namespace reporting {

using Timeout         = System::Clock::Timeout;
using Timestamp       = System::Clock::Timestamp;
using Milliseconds64  = System::Clock::Milliseconds64;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;
using TimerDelegate   = ReportScheduler::TimerDelegate;

class SynchronizedReportSchedulerImpl : public ReportSchedulerImpl, public TimerContext
{
public:
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    SynchronizedReportSchedulerImpl(TimerDelegate * aTimerDelegate) : ReportSchedulerImpl(aTimerDelegate) {}
    ~SynchronizedReportSchedulerImpl() override { UnregisterAllHandlers(); }

    void OnTransitionToIdle() override;

    bool IsReportScheduled();

    void TimerFired() override;

protected:
    CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandlerNode * node, const Timestamp & now) override;
    void CancelReport();

private:
    friend class chip::app::reporting::TestReportScheduler;

    CHIP_ERROR FindNextMinInterval(const Timestamp & now);
    CHIP_ERROR FindNextMaxInterval(const Timestamp & now);
    CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aReadHandlerNode, const Timestamp & now) override;

    Timestamp mNextMaxTimestamp = Milliseconds64(0);
    Timestamp mNextMinTimestamp = Milliseconds64(0);

    // Timestamp of the next report to be scheduled, only used for testing
    Timestamp mTestNextReportTimestamp = Milliseconds64(0);
};

} // namespace reporting
} // namespace app
} // namespace chip
