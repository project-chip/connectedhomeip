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
 * @brief This class extends ReportSchedulerImpl and overrides its scheduling logic.
 *
 * It overrides the OnTransitionToIdle methods from ReadHandler::Observer.
 *
 * It inherits from TimerContext so that it can be used as a TimerDelegate instead of relying on the nodes to schedule themselves.
 *
 * ## Scheduling Logic
 *
 * This class implements a scheduling logic that aims to make all ReadHandlers report at the same time when possible.
 * The goal is to minimize the number of times a device wakes up to report, and thus this aims to schedule all reports at the latest
 * possible time while ensuring that all reports get sent before their max interval.
 *
 * The logic also aims to minimize the impact on the responsiveness of the device.
 *
 * The scheduling logic is as follows:
 * - The CalculateNextReportTimeout is called by any ReadHandler methods that affect when/whether a report should be sent. These
 *   are:
 *      * OnSubscriptionEstablished,
 *      * OnBecameReportable,
 *      * OnSubscriptionReportSent
 *
 * - The Synchronized Scheduler keeps track of the next min and max interval timestamps and updates them in
 *   CalculateNextReportTimeout
 *
 * - The next max interval is calculated as the earliest max interval of all the registered ReadHandlersNodes.
 *
 * - The next min interval is calculated as the latest min interval of the registered ReadHandlersNodes that:
 *     * Have a min timestamp greater than the current time
 *     * Are Reportable (this prevents a ReadHandler that is not reportable from blocking the reporting of other ReadHandlers)
 *       TODO: Assess if we want to keep this behavior or simply let the min interval be the earliest min interval to prevent cases
 *             where a ReadHandler with a dirty path but a very high min interval blocks all reports
 * - If no ReadHandlerNode matches the min interval criteria, the next min interval is set to the current timestamp.
 *
 * - The next report timeout is calculated in CalculatedNextReportTimeout based on the next min and max interval timestamps, as well
 * as the status of each ReadHandlerNode in the pool.
 *
 * @note Unlike the non-synchronized implementation, the Synchronized Scheduler will reschedule itself in the event that a timer
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
     * It loops through all handlers and sets their CanBeSynced flag to true if the current timestamp is greater than
     * their respective minimal timestamps.
     *
     * While looping, it checks if any handler is reportable now. If not, we recalculate the next report timeout and reschedule the
     * report.
     *
     * If a Readhandler is reportable now, an engine run is scheduled.
     *
     * If the timer expires after all nodes are unregistered, no action is taken.
     */
    void TimerFired() override;

protected:
    /**
     * @brief Schedule a report for the Scheduler.
     *
     * If a report is already scheduled, cancel it and schedule a new one.
     *
     * @param[in] timeout The delay before the report will happen.
     * @param[in] node The node associated with the ReadHandler.
     * @param[in] now The current system timestamp.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, timer-related error code otherwise (This can only fail on starting the timer)
     */
    CHIP_ERROR ScheduleReport(System::Clock::Timeout timeout, ReadHandlerNode * node, const Timestamp & now) override;
    void CancelReport();

private:
    friend class chip::app::reporting::TestReportScheduler;

    /**
     * @brief Find the highest minimum timestamp possible that still respects the lowest max timestamp and sets it as the common
     * minimum. If the max timestamp has not been updated and is in the past, or if no min timestamp is lower than the current max
     * timestamp, this will set the "now" parameter as the common minimum timestamp, thus allowing the report to be sent
     * immediately.
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
      @note When looping through the ReadHandlerNodes, the IsEngineRunScheduled flag is used to prevent calling ScheduleRun on a
            ReadHandler that already has an engine run scheduled, which would cause an endless report loop in some cases. The only
            reason why we would want to call ScheduleRun on a node that already has an engine run scheduled is if the ongoing report
            is chunked, which means that the report is not fully sent yet and that the EngineRun should be scheduled again until
     there are no chunks left.

          The Endless Reporting Loop Scenario would be:
          1. At least two ReadHandlers are registered to the Scheduler
          2. ScheduleRun() is called with 2 reportable ReadHandlers (meaning they both return true to IsReportableNow())
          3. The Scheduler sends the first report and calls OnSubscriptionReportSent on the ReadHandler
          4. OnSubscriptionReportSent calls CalculateNextReportTimeout, which loops through all ReadHandlers and finds that a least
           one ReadHandler is reportable now, and thus sets the timeout to 0.
          5. OnSubscriptionReportSent then calls ScheduleReport with a timeout of 0, which calls TimerFired on the Scheduler
          6. If the MinInterval of the ReadHandler is 0, the Scheduler will set the CanBeSynced flag to true, and the
             IsReportableNow Will return true since (now >= MinTimestamp || CanBeSynced()) will be true.
          7. ScheduleRun() will be called on the ReadHandler with 2 reportable ReadHandlers, and the loop will start again.
     *
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
