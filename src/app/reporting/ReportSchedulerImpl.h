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
 * It is responsible for implementing the ReadHandler and ICD observers callbacks so the Scheduler can take action whenever a
 * ReadHandler event occurs or the ICD mode change occurs.
 *
 * All ReadHandlers Observers callbacks rely on the node pool to create or find the node associated with the ReadHandler that
 * triggered the callback and will use the FindReadHandlerNode() method to do so.
 *
 * ## Scheduling Logic
 *
 * This class implements a scheduling logic that calculates the next report timeout based on the current system timestamp, the state
 * of the ReadHandlers associated with the scheduler nodes and the min and max intervals of the ReadHandlers.
 *
 * The logic is as follows:
 *
 * - When a ReadHandler is created for a subscription, the scheduler adds a node and registers it in the scheduler node pool.
 *
 * - Each node can schedule a report independently from the other nodes, and thus each node has its timer.
 *
 * - The timeout of each node timer is calculated when its associated ReadHandler becomes reportable, when a report is sent for
 *   the ReadHandler.
 *
 * - The scheduler calculates the next report timeout of each node timer based on the current system timestamp and the state of the
 *  ReadHandlers. If the ReadHandler is not reportable, the timeout is the difference between the next max interval and now. If the
 *  ReadHandler is reportable, the timeout is the difference between the next min interval and now. If that min interval is in the
 *  past, the scheduler directly calls the TimerFired() method instead of starting a timer.
 *
 *

 */
class ReportSchedulerImpl : public ReportScheduler
{
public:
    using Timeout = System::Clock::Timeout;

    ReportSchedulerImpl(TimerDelegate * aTimerDelegate);
    ~ReportSchedulerImpl() override { UnregisterAllHandlers(); }

    // ICDStateObserver

    /**
     * @brief This implementation is not attempting any synchronization on external events as each Node is scheduled independently
     *        solely based on its ReadHandler's state. Therefore, no synchronization action on the ICDState is needed in this
     *        implementation.
     */
    void OnTransitionToIdle() override{};

    /**
     * @brief When the ICD transitions to Active mode, this implementation will trigger a report emission on each ReadHandler that
     * is not blocked by its min interval.
     *
     * @note Most of the actions that trigger a change to the Active mode already trigger a report emission (e.g. Event or Attribute
     * change), so this method is optional as it might be redundant.
     */
    void OnEnterActiveMode() override;

    /**
     * @brief Similar to the OnTransitionToIdle() method, this implementation does not attempt any synchronization on ICD events,
     *        therefore no action is needed on the ICDModeChange() method.
     */
    void OnICDModeChange() override{};

    /**
     * @brief This implementation does not attempt any synchronization on this ICD event, therefore no action is needed on
     * ICDEnterIdleMode()
     */
    void OnEnterIdleMode() override{};

    // ReadHandlerObserver

    /**
     * @brief When a ReadHandler is created for a subscription, the scheduler adds a node and registers it in the scheduler node
     * pool. Scheduling the report here is unnecessary since the ReadHandler will call
     * MoveToState(HandlerState::CanStartReporting);, which will call OnBecameReportable() and schedule a report.
     *
     * @note This method sets a timestamp to the call time that is used as an input parameter by the ScheduleReport method.
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
     * @note This method sets a now Timestamp that is used to calculate the next report timeout.
     */
    void OnSubscriptionReportSent(ReadHandler * aReadHandler) final;

    /**
     * @brief When a ReadHandler is destroyed, remove the node from the scheduler node pool and cancel the timer associated to it.
     */
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    /**
     * @brief Checks if a report is scheduled for the ReadHandler by checking if the timer is active.
     *
     *      @note If the CalculateNextReportTimeout outputs 0, the TimerFired() will be called directly instead of starting a timer,
     *      so this method will return false.
     */
    virtual bool IsReportScheduled(ReadHandler * aReadHandler);

    void ReportTimerCallback() override;

protected:
    /**
     * @brief Schedule a report for the ReadHandler associated with a ReadHandlerNode.
     *
     * @note If a report is already scheduled for the ReadHandler, this method will cancel it and schedule a new one.
     *
     * @param[in] timeout The timeout to schedule the report.
     * @param[in] node The node associated with the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, timer-related error code otherwise (This can only fail on starting the timer)
     */
    virtual CHIP_ERROR ScheduleReport(Timeout timeout, ReadHandlerNode * node, const Timestamp & now);
    void CancelReport(ReadHandler * aReadHandler);
    virtual void UnregisterAllHandlers();

private:
    friend class chip::app::reporting::TestReportScheduler;

    /**
     * @brief Find the next timestamp when a report should be scheduled for a ReadHandler.
     *
     * @param[out] timeout The timeout calculated from the "now" timestamp provided as an input parameter.
     * @param[in] aNode The node associated to the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success or CHIP_ERROR_INVALID_ARGUMENT if aNode is not in the pool.
     *
     */
    virtual CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aNode, const Timestamp & now);
};

} // namespace reporting
} // namespace app
} // namespace chip
