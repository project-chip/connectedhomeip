/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/reporting/ReportScheduler.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

class DefaultTimerDelegate : public reporting::ReportScheduler::TimerDelegate
{
public:
    using TimerContext = reporting::TimerContext;
    using Timeout      = System::Clock::Timeout;
    CHIP_ERROR StartTimer(TimerContext * context, Timeout aTimeout) override;
    void CancelTimer(TimerContext * context) override;
    bool IsTimerActive(TimerContext * context) override;
    System::Clock::Timestamp GetCurrentMonotonicTimestamp() override;
};

} // namespace app
} // namespace chip
