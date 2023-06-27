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
    virtual ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ReadHandlerObserver
    virtual void OnReadHandlerAdded(ReadHandler * aReadHandler) override;
    virtual void OnBecameReportable(ReadHandler * aReadHandler) override;
    virtual void OnReportSent(ReadHandler * aReadHandler) override;
    virtual void OnReadHandlerRemoved(ReadHandler * aReadHandler) override;

    // ReportScheduler specific
    virtual CHIP_ERROR RegisterReadHandler(ReadHandler * aReadHandler) override;
    virtual CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandler * aReadHandler) override;
    virtual void CancelReport(ReadHandler * aReadHandler) override;
    virtual void UnregisterReadHandler(ReadHandler * aReadHandler) override;
    virtual void UnregisterAllHandlers() override;
    virtual bool IsReportScheduled(ReadHandler * aReadHandler) override;

protected:
    friend class chip::app::TestReportScheduler;

    /// @brief Find the ReadHandlerNode for a given ReadHandler pointer
    /// @param [in] aReadHandler
    /// @return Node Address if node was found, nullptr otherwise
    virtual ReadHandlerNode * FindReadHandlerNode(const ReadHandler * aReadHandler) override;

    // void ReportTimerCallback(System::Layer * aLayer, void * aAppState) override;
};

} // namespace reporting
} // namespace app
} // namespace chip
