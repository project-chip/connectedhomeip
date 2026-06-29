/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LoggingOnlySmokeCoAlarmDevice.h"

#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

const std::array<ExpressedStateEnum, chip::app::Clusters::SmokeCoAlarmCluster::kPriorityOrderLength>
    chip::app::LoggingOnlySmokeCoAlarmDevice::sPriorityOrder = {
        ExpressedStateEnum::kInoperative, ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kInterconnectSmoke,
        ExpressedStateEnum::kCOAlarm,     ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kHardwareFault,
        ExpressedStateEnum::kTesting,     ExpressedStateEnum::kEndOfService,   ExpressedStateEnum::kBatteryAlert
    };

namespace chip {
namespace app {

// The device is its own delegate: SmokeCoAlarmDevice only stores the reference, so passing *this
// (the not-yet-fully-constructed SmokeCoAlarmDelegate base) is safe as long as it is not used until
// the cluster is registered.
LoggingOnlySmokeCoAlarmDevice::LoggingOnlySmokeCoAlarmDevice(TimerDelegate & timerDelegate) :
    SmokeCoAlarmDevice(timerDelegate, *this)
{}

LoggingOnlySmokeCoAlarmDevice::~LoggingOnlySmokeCoAlarmDevice()
{
    // Cancel any pending self-test timer before the delegate goes away.
    mTimerDelegate.CancelTimer(this);
}

void LoggingOnlySmokeCoAlarmDevice::OnSelfTestRequested()
{
    ChipLogDetail(NotSpecified, "SmokeCoAlarm: self-test started");
    mTimerDelegate.StartTimer(this, System::Clock::Seconds32(kSelfTestTimeoutSec));
}

void LoggingOnlySmokeCoAlarmDevice::TimerFired()
{
    Clusters::SmokeCoAlarmCluster & cluster = GetSmokeCoAlarmCluster();
    cluster.SetTestInProgress(false);
    cluster.SetExpressedStateByPriority(sPriorityOrder);
    ChipLogDetail(NotSpecified, "SmokeCoAlarm: self-test complete");
}

void LoggingOnlySmokeCoAlarmDevice::OnSmokeSensitivityLevelChanged(SensitivityEnum newLevel)
{
    ChipLogDetail(NotSpecified, "SmokeCoAlarm: smoke sensitivity level changed to %u", to_underlying(newLevel));
}

void LoggingOnlySmokeCoAlarmDevice::OnExpressedStateChanged(ExpressedStateEnum newState)
{
    ChipLogDetail(NotSpecified, "SmokeCoAlarm: expressed state changed to %u", to_underlying(newState));
}

} // namespace app
} // namespace chip
