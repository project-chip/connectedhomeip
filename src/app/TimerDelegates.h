/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
