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

#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <system/SystemClock.h>

using Timeout = chip::System::Clock::Timeout;

namespace chip {
namespace app {

static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
{
    TimerContext * context = static_cast<TimerContext *>(aAppState);
    context->TimerFired();
}
CriticalFailure DefaultTimerDelegate::StartTimer(TimerContext * context, Timeout aTimeout)
{
    return DeviceLayer::SystemLayer().StartTimer(aTimeout, TimerCallbackInterface, context);
}
void DefaultTimerDelegate::CancelTimer(TimerContext * context)
{
    DeviceLayer::SystemLayer().CancelTimer(TimerCallbackInterface, context);
}
bool DefaultTimerDelegate::IsTimerActive(TimerContext * context)
{
    return DeviceLayer::SystemLayer().IsTimerActive(TimerCallbackInterface, context);
}

System::Clock::Timestamp DefaultTimerDelegate::GetCurrentMonotonicTimestamp()
{
    return System::SystemClock().GetMonotonicTimestamp();
}

} // namespace app
} // namespace chip
