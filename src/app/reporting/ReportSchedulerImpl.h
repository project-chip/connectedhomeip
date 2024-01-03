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

/**
 * @class ReportSchedulerImpl
 *
 * @brief This class extends ReportScheduler and provides a scheduling logic for the CHIP Interaction Model Reporting Engine.
 *
 * It is reponsible for implementing the ReadHandler and ICD observers callbacks to the Scheduler can take actions whenever a
 * ReadHandler event occurs or the ICD changes modes.
 *
 * All ReadHandlers Observers callbacks rely on the node pool to create or find the node associated to the ReadHandler that
 * triggered the callback and will use the FindReadHandlerNode() method to do so.
 *
 * ## Scheduling Logic
 *
 * This class implements a scheduling logic that calculates the next report timeout based on the current system timestamp, the state
 * of the ReadHandlers associated with the scheduler nodes and the min and max intervals of the ReadHandlers.
 *
 * @note This class mimics the original scheduling in which the ReadHandlers would  schedule themselves. The key difference is that
 * this implementation only relies on a single timer from the scheduling moment rather than having a timer expiring on the min
 * interval that would trigger the start of a second timer expiring on the max interval.
 */
class ReportSchedulerImpl : public ReportScheduler
{
public:
    using Timeout = System::Clock::Timeout;

    ReportSchedulerImpl(TimerDelegate * aTimerDelegate);
    ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ICDStateObserver

    /**
     * @brief When the ICD changes to Idle, no action is taken in this implementation.
     */
    void OnTransitionToIdle() override{};

    /**
     * @brief When the ICD changes to Active, this implementation will trigger a report emission on each ReadHandler that is not
     * blocked on its min interval.
     *
     * @note Most action triggering a change to the Active mode already trigger a report emission, so this method is optionnal as it
     * might be redundant.
     */
    void OnEnterActiveMode() override;

    /**
     * @brief When the ICD changes operation mode, no action is taken in this implementation.
     */
    void OnICDModeChange() override{};

    // ReadHandlerObserver

    /**
     * @brief When a ReadHandler is added, adds a node and register it in the scheduler node pool. Scheduling the report here is
     * un-necessary since the ReadHandler will call MoveToState(HandlerState::CanStartReporting);, which will call
     * OnBecameReportable() and schedule the report.
     *
     * @note This method sets a now Timestamp that is used to calculate the next report timeout.
     */
    void OnSubscriptionEstablished(ReadHandler * aReadHandler) final;

    /**
     * @brief When a ReadHandler becomes reportable, recalculate and reschedule the report.
     *
     * @note This method sets a now Timestamp that is used to calculate the next report timeout.
     */
    void OnBecameReportable(ReadHandler * aReadHandler) final;

    /**
     * @brief When a ReadHandler report is sent, recalculate and reschedule the report.
     *
     * @note This method is called after the report is sent, so the ReadHandler is no longer reportable, and thus CanBeSynced and
     * EngineRunScheduled of the node associated to the ReadHandler are set to false in this method.
     *
     * @note This method sets a now Timestamp that is used to calculate the next report timeout.
     */
    void OnSubscriptionReportSent(ReadHandler * aReadHandler) final;

    /**
     * @brief When a ReadHandler is destroyed, remove the node from the scheduler node pool and cancel the timer associated to it.
     */
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    virtual bool IsReportScheduled(ReadHandler * aReadHandler);

    void ReportTimerCallback() override;

protected:
    /**
     * @brief Schedule a report for the ReadHandler associated to the node.
     *
     *  If a report is already scheduled for the ReadHandler, cancel it and schedule a new one.
     *  If the timeout is 0, directly calls the TimerFired() method of the node instead of scheduling a report.
     *
     * @param[in] timeout The timeout to schedule the report.
     * @param[in] node The node associated to the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, timer related error code otherwise (This can only fail on starting the timer)
     */
    virtual CHIP_ERROR ScheduleReport(Timeout timeout, ReadHandlerNode * node, const Timestamp & now);
    void CancelReport(ReadHandler * aReadHandler);
    virtual void UnregisterAllHandlers();

private:
    friend class chip::app::reporting::TestReportScheduler;

    /**
     * @brief Find the next timer when a report should be scheduled for a ReadHandler.
     *
     * @param[out] timeout The timeout to calculate.
     * @param[in] aNode The node associated to the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success or CHIP_ERROR_INVALID_ARGUMENT if aNode is not in the pool.
     *
     * The logic is as follows:
     * - If the ReadHandler is reportable now, the timeout is 0.
     * - If the ReadHandler is reportable, but the current timestamp is earlier thant the next min interval's timestamp, the timeout
     * is the delta between the next min interval and now.
     * - If the ReadHandler is not reportable, the timeout is the difference between the next max interval and now.
     */
    virtual CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode, const Timestamp & now);
};

} // namespace reporting
} // namespace app
} // namespace chip
