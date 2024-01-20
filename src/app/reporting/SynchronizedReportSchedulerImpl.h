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

/**
 * @class Synchronized ReportSchedulerImpl
 *
 * @brief This class extends ReportSchedulerImpl and overrides it's scheduling logic.
 *
 * It only overrides Observers method where the scheduling logic make it necessary, the others are kept as is.
 *
 * It inherits from TimerContext so that it can be used as a TimerDelegate instead on relying on the nodes to schedule themselves.
 *
 * ## Scheduling Logic
 *
 * This class implements a scheduling logic that aims to make all ReadHandlers report at the same time when possible.
 * The goal is to minimize the different times a device wakes up to report, and thus this aims to schedule all reports at the latest
 * possible time while ensuring that all reports get sent before their max interval.
 *
 * The logic also aims to minimize the impact on the responsivity of the device.
 *
 * The scheduling logic is as follows:
 * - The CalculateNextReportTimeout is called by the same ReadHandler Observer callbacks than the non-synchronized implementation:
 *      * OnSubscriptionEstablished,
 *      * OnBecameReportable,
 *      * OnSubscriptionReportSent
 *
 * - The Synchronized Scheduler keeps track of the next min and max interval timestamps. It updates in CalculateNextReportTimeout
 *
 * - The next max interval is calculated as the earliest max interval of all the registered ReadHandlersNodes.
 *
 * - The next min interval is calculated as the latest min interval of the registered ReadHandlersNodes that:
 *     * Have a min timestamp greater than the current time
 *     * Are Reportable (this prevents a ReadHandler that is not reportable to hold the report of all the others)
 *       TODO: Assess if we want to keep this behavior or simply let the min interval be the earliest min interval to prevent cases
 *             where a ReadHandler with a dirty path but a very high min interval blocks all reports
 * - If no ReadHandlerNode matches min interval  the criteria, the next min interval is set to current timestamp.
 *
 * - The next report timeout is calculated in CalculatedNextReportTimeout based on the next min and max interval timestamps, as well
 * as the status of each ReadHandlerNode in the pool.
 *
 * @note Unlike the non-synchronized implementation, the Synchronized Scheduler will reschedule itself in the event where a timer
 * fires before a reportable timestamp is reached.
 *
 * @note In this implementation, nodes still keep track of their own min and max interval timestamps.
 */
class SynchronizedReportSchedulerImpl : public ReportSchedulerImpl, public TimerContext
{
public:
    void OnReadHandlerDestroyed(ReadHandler * aReadHandler) override;

    SynchronizedReportSchedulerImpl(TimerDelegate * aTimerDelegate) : ReportSchedulerImpl(aTimerDelegate) {}
    ~SynchronizedReportSchedulerImpl() override { UnregisterAllHandlers(); }

    void OnTransitionToIdle() override;

    bool IsReportScheduled(ReadHandler * ReadHandler) override;

    /** @brief Callback called when the report timer expires to schedule an engine run regardless of the state of the ReadHandlers,
     *
     * It loops through all handlers and sets their CanBeSynced flag to true if the current timstamp is greater than
     * their respective minimal timestamps.
     *
     * While looping, it checks if any handler is reportable now. If not, we recalculate the next report timeout and reschedule the
     * report.
     *
     * If a Readhangler is reportable now, an engine run is scheduled.
     *
     * If the timer expires after all nodes were unregistered, no action is taken.
     */
    void TimerFired() override;

protected:
    /**
     * @brief Schedule a report for the Scheduler.
     *
     * If a report is already scheduled, cancel it and schedule a new one.
     *
     * @param[in] timeout The timeout to schedule the report.
     * @param[in] node The node associated to the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, timer related error code otherwise (This can only fail on starting the timer)
     */
    CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandlerNode * node, const Timestamp & now) override;
    void CancelReport();

private:
    friend class chip::app::reporting::TestReportScheduler;

    /**
     * @brief Find the highest minimum timestamp possible that still respects the lowest max timestamp and sets it as the common
     * minimum. If the max timestamp has not been updated and is in the past, or if no min timestamp is lower than the current max
     * timestamp, this will set now as the common minimum timestamp, thus allowing the report to be sent immediately.
     *
     * @param[in] now The current system timestamp, set by the event that triggered the call of this method.
     *
     * @return CHIP_ERROR on success or CHIP_ERROR_INVALID_LIST_LENGTH if the list is empty
     */
    CHIP_ERROR FindNextMinInterval(const Timestamp & now);

    /**
     * @brief Find the smallest maximum interval possible and set it as the common maximum
     *
     * @param[in] now The current system timestamp, set by the event that triggered the call of this method.
     *
     * @return CHIP_ERROR on success or CHIP_ERROR_INVALID_LIST_LENGTH if the list is empty
     */
    CHIP_ERROR FindNextMaxInterval(const Timestamp & now);

    /**
     *  @brief Calculate the next report timeout for all ReadHandlerNodes
     *
     * @param[out] timeout The timeout to calculate.
     * @param[in] aReadHandlerNode unused, kept to preserve the signature of the base class
     * @param[in] now The current system timestamp when the event leading to the call of this method happened.
     *
     *  The next report timeout is calculated by looping through all the ReadHandlerNodes and finding if any are reportable now
     *      or at min.
     *   * If a ReadHandlerNode is reportable now, the timeout is set to 0.
     *   * If a ReadHandlerNode is reportable at min, the timeout is set to the difference between the Scheduler's  min timestamp
     *      and the current time.
     *   * If no ReadHandlerNode is reportable, the timeout is set to the difference between the Scheduler's max timestamp and the
     *      current time.
     *
     * @note Since this method is called after the OnSubscriptionReportSent callback, to avoid an endless reporting loop, Nodes with
     * the IsEngineRunScheduled flag set are ignored when finding if the Scheduler should report at min, max or now.
     *
     * @note If a ReadHandler's report is Chunked, the IsEngineRunScheduled is ignored since we do want to keep rescheduling the
     *       report to the now timestamp until it is fully sent. IsChunkedReport is used to prevent starting a chunked report and
     * then waiting on the max interval after the first chunk is sent.
     */
    CHIP_ERROR CalculateNextReportTimeout(Timeout & timeout, ReadHandlerNode * aReadHandlerNode, const Timestamp & now) override;

    Timestamp mNextMaxTimestamp = Milliseconds64(0);
    Timestamp mNextMinTimestamp = Milliseconds64(0);

    // Timestamp of the next report to be scheduled, used by OnTransitionToIdle to determine whether we should emit a report before
    // the device goes to idle mode
    Timestamp mNextReportTimestamp = Milliseconds64(0);
};

} // namespace reporting
} // namespace app
} // namespace chip
