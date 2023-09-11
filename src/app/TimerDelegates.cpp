/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/InteractionModelEngine.h>
#include <app/TimerDelegates.h>
#include <system/SystemClock.h>

using TimerContext = chip::app::reporting::TimerContext;
using Timeout      = chip::System::Clock::Timeout;

namespace chip {
namespace app {

static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
{
    TimerContext * context = static_cast<TimerContext *>(aAppState);
    context->TimerFired();
}
CHIP_ERROR DefaultTimerDelegate::StartTimer(TimerContext * context, Timeout aTimeout)
{
    return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        aTimeout, TimerCallbackInterface, context);
}
void DefaultTimerDelegate::CancelTimer(TimerContext * context)
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        TimerCallbackInterface, context);
}
bool DefaultTimerDelegate::IsTimerActive(TimerContext * context)
{
    return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->IsTimerActive(
        TimerCallbackInterface, context);
}

System::Clock::Timestamp DefaultTimerDelegate::GetCurrentMonotonicTimestamp()
{
    return System::SystemClock().GetMonotonicTimestamp();
}

} // namespace app
} // namespace chip
