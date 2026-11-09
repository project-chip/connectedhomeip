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
#include "DecreasingBatteryPowerSource.h"

#include <algorithm>

namespace chip::app {

namespace {

constexpr System::Clock::Seconds16 kDecreaseBatteryLevelInterval = System::Clock::Seconds16(1);

// BatPercentRemaining is a doubled percentage: 200 == 100%.
constexpr uint8_t kFullBatteryLevelHalfPercent = 200;
constexpr uint8_t kBatteryLevelStepHalfPercent = 5;

constexpr uint32_t kFullVoltageMv  = 3000;
constexpr uint32_t kEmptyVoltageMv = 2000;

} // namespace

DecreasingBatteryPowerSource::DecreasingBatteryPowerSource(TimerDelegate & timerDelegate) :
    BatteryPowerSource("Decreasing Battery Power Source"_span, Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                       timerDelegate)
{}

DecreasingBatteryPowerSource::~DecreasingBatteryPowerSource()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR DecreasingBatteryPowerSource::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    ReturnErrorOnFailure(BatteryPowerSource::Register(endpoint, provider, composition));

    CHIP_ERROR err = mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval);
    if (err != CHIP_NO_ERROR)
    {
        BatteryPowerSource::Unregister(provider);
        return err;
    }

    return CHIP_NO_ERROR;
}

void DecreasingBatteryPowerSource::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    BatteryPowerSource::Unregister(provider);
}

void DecreasingBatteryPowerSource::TimerFired()
{
    auto & batteryCluster = BatteryPowerSourceCluster();
    auto batteryLevel     = batteryCluster.GetBatPercentRemaining();

    if (batteryLevel.IsNull())
    {
        batteryLevel.SetNonNull(kFullBatteryLevelHalfPercent);
    }
    else if (batteryLevel.Value() > 0)
    {
        const uint8_t level = batteryLevel.Value();
        batteryLevel.SetNonNull(static_cast<uint8_t>(level - std::min(level, kBatteryLevelStepHalfPercent)));
    }
    else
    {
        batteryLevel.SetNull();
    }

    LogErrorOnFailure(batteryCluster.SetBatPercentRemaining(batteryLevel));

    DataModel::Nullable<uint32_t> batteryVoltage;
    if (!batteryLevel.IsNull())
    {
        const uint32_t level = batteryLevel.Value();
        batteryVoltage.SetNonNull(kEmptyVoltageMv + ((kFullVoltageMv - kEmptyVoltageMv) * level) / kFullBatteryLevelHalfPercent);
    }
    batteryCluster.SetBatVoltage(batteryVoltage);

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kDecreaseBatteryLevelInterval));
}

} // namespace chip::app
