/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/TimerDelegates.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/reporting/SynchronizedReportSchedulerImpl.h>

namespace chip {
namespace app {
namespace reporting {

/// @brief Static instance of the default report scheduler and synchronized report scheduler meant for injection into IM engine in
/// tests

static chip::app::DefaultTimerDelegate sTimerDelegate;
static ReportSchedulerImpl sTestDefaultReportScheduler(&sTimerDelegate);
static SynchronizedReportSchedulerImpl sTestReportScheduler(&sTimerDelegate);

ReportSchedulerImpl * GetDefaultReportScheduler()
{
    return &sTestDefaultReportScheduler;
}

SynchronizedReportSchedulerImpl * GetSynchronizedReportScheduler()
{
    return &sTestReportScheduler;
}

} // namespace reporting
} // namespace app
} // namespace chip
