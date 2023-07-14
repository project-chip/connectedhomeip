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
    ReportSchedulerImpl(TimerDelegate * aTimerDelegate);
    ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ReadHandlerObserver
    void OnReadHandlerCreated(ReadHandler * aReadHandler) override;
    void OnBecameReportable(ReadHandler * aReadHandler) override;
    void OnSubscriptionAction(ReadHandler * aReadHandler) override;
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

protected:
    virtual CHIP_ERROR RegisterReadHandler(ReadHandler * aReadHandler);
    virtual CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandlerNode * node);
    virtual void CancelReport(ReadHandler * aReadHandler);
    virtual void UnregisterReadHandler(ReadHandler * aReadHandler);
    virtual void UnregisterAllHandlers();

private:
    friend class chip::app::reporting::TestReportScheduler;

    bool IsReportScheduled(ReadHandler * aReadHandler) override;

    /// @brief Start a timer for a given ReadHandlerNode, ensures that if a timer is already running for this node, it is cancelled
    /// @param node Node of the ReadHandler list to start a timer for
    /// @param aTimeout Delay before the timer expires
    virtual CHIP_ERROR StartSchedulerTimer(ReadHandlerNode * node, System::Clock::Timeout aTimeout);
    /// @brief Cancel the timer for a given ReadHandlerNode
    virtual void CancelSchedulerTimer(ReadHandlerNode * node);
    /// @brief Check if the timer for a given ReadHandlerNode is active
    virtual bool CheckSchedulerTimerActive(ReadHandlerNode * node);
};

} // namespace reporting
} // namespace app
} // namespace chip
