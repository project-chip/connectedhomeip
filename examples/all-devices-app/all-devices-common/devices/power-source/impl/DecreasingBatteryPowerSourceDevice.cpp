/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "DecreasingBatteryPowerSourceDevice.h"

namespace chip::app {

constexpr System::Clock::Seconds16 kDecreaseBatteryLevelInterval = System::Clock::Seconds16(1);

DecreasingBatteryPowerSourceDevice::DecreasingBatteryPowerSourceDevice() :
    BatteryPowerSourceDevice("Decreasing Battery Power Source"_span, Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                             mTimerDelegate)
{}

DecreasingBatteryPowerSourceDevice::~DecreasingBatteryPowerSourceDevice()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR DecreasingBatteryPowerSourceDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                        EndpointId parentId)
{
    ReturnErrorOnFailure(BatteryPowerSourceDevice::Register(endpoint, provider, parentId));
    // Kick off the timer loop to decrease battery level every few seconds
    return mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval);
}

void DecreasingBatteryPowerSourceDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    BatteryPowerSourceDevice::Unregister(provider);
    mTimerDelegate.CancelTimer(this);
}

void DecreasingBatteryPowerSourceDevice::TimerFired()
{
    auto & batteryCluster = BatteryPowerSourceCluster();
    auto batteryLevel     = batteryCluster.GetBatPercentRemaining();

    if (batteryLevel.IsNull())
    {
        batteryLevel.SetNonNull(200); // 100% (doubled percentage)
    }
    else if (batteryLevel.Value() > 0)
    {
        batteryLevel.SetNonNull(batteryLevel.Value() - 5); // decrease by 2.5%
    }
    else
    {
        batteryLevel.SetNull(); // set to null when it reaches 0%
    }

    LogErrorOnFailure(batteryCluster.SetBatPercentRemaining(batteryLevel));

    // Restart the timer to continue decreasing the battery level
    SuccessOrDie(mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval));
}

} // namespace chip::app
