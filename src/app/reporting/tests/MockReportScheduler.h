/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/reporting/ReportSchedulerImpl.h>
#include <app/reporting/SynchronizedReportSchedulerImpl.h>

namespace chip {
namespace app {
namespace reporting {

ReportSchedulerImpl * GetDefaultReportScheduler();

SynchronizedReportSchedulerImpl * GetSynchronizedReportScheduler();

} // namespace reporting
} // namespace app
} // namespace chip
