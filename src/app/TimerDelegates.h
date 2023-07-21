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

#include <app/InteractionModelEngine.h>
#include <app/reporting/ReportScheduler.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

class DefaultTimerDelegate : public app::reporting::ReportScheduler::TimerDelegate
{
public:
    using ReadHandlerNode        = app::reporting::ReportScheduler::ReadHandlerNode;
    using InteractionModelEngine = app::InteractionModelEngine;
    using Timeout                = System::Clock::Timeout;
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        app::reporting::ReportScheduler::TimerCallback * callback =
            static_cast<app::reporting::ReportScheduler::TimerCallback *>(aAppState);
        (*callback)();
    }
    CHIP_ERROR StartTimer(void * context, Timeout aTimeout) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            aTimeout, TimerCallbackInterface, context);
    }
    void CancelTimer(void * context) override
    {
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            TimerCallbackInterface, context);
    }
    bool IsTimerActive(void * context) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->IsTimerActive(
            TimerCallbackInterface, context);
    }

    System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return System::SystemClock().GetMonotonicTimestamp(); }
};

} // namespace app
} // namespace chip
