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

using Timeout = System::Clock::Timeout;

class ReportSchedulerImpl : public ReportScheduler
{
public:
    ReportSchedulerImpl(TimerDelegate * aTimerDelegate);
    ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ReadHandlerObserver
    void OnReadHandlerCreated(ReadHandler * aReadHandler) override;
    void OnBecameReportable(ReadHandler * aReadHandler) override;
    void OnSubscriptionAction(ReadHandler * aReadHandler) override;
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    bool IsReportScheduled(ReadHandler * aReadHandler) override;

    void ReportTimerCallback();

protected:
    virtual CHIP_ERROR RegisterReadHandler(ReadHandler * aReadHandler);
    virtual CHIP_ERROR ScheduleReport(Timeout timeout, ReadHandlerNode * node);
    virtual void CancelReport(ReadHandler * aReadHandler);
    virtual void UnregisterReadHandler(ReadHandler * aReadHandler);
    virtual void UnregisterAllHandlers();

private:
    friend class chip::app::reporting::TestReportScheduler;

    virtual CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode);
};

} // namespace reporting
} // namespace app
} // namespace chip
