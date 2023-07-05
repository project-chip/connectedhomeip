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

#include <app/reporting/ReportSchedulerImpl.h>

namespace chip {
namespace app {
namespace reporting {

using Timeout         = System::Clock::Timeout;
using Timestamp       = System::Clock::Timestamp;
using Milliseconds64  = System::Clock::Milliseconds64;
using ReadHandlerNode = ReportScheduler::ReadHandlerNode;
using TimerDelegate   = ReportScheduler::TimerDelegate;

class SynchronizedReportSchedulerImpl : public ReportSchedulerImpl
{
public:
    SynchronizedReportSchedulerImpl(TimerDelegate * aTimerDelegate) : ReportSchedulerImpl(aTimerDelegate) {}
    ~SynchronizedReportSchedulerImpl() {}

    // ReadHandlerObserver
    void OnBecameReportable(ReadHandler * aReadHandler) override;
    void OnSubscriptionAction(ReadHandler * aReadHandler) override;

    bool IsReportScheduled(ReadHandler * aReadHandler) override;

    void ReportTimerCallback() override;

protected:
    CHIP_ERROR RegisterReadHandler(ReadHandler * aReadHandler) override;
    virtual CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandlerNode * node) override;
    void CancelReport(ReadHandler * aReadHandler) override;
    void UnregisterReadHandler(ReadHandler * aReadHandler) override;

private:
    friend class chip::app::reporting::TestReportScheduler;

    CHIP_ERROR StartSchedulerTimer(ReadHandlerNode * node, Timeout aTimeout) override;
    void CancelSchedulerTimer(ReadHandlerNode * node) override;
    bool CheckSchedulerTimerActive(ReadHandlerNode * node) override;

    CHIP_ERROR FindNextMinInterval();
    CHIP_ERROR FindNextMaxInterval();
    CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aReadHandlerNode);

    Timestamp mNextMaxTimestamp    = Milliseconds64(0);
    Timestamp mNextMinTimestamp    = Milliseconds64(0);
    Timestamp mNextReportTimestamp = Milliseconds64(0);
};

} // namespace reporting
} // namespace app
} // namespace chip
