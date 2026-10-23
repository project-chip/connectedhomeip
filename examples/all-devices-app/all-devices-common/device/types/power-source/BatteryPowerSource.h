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

#pragma once

#include <app/clusters/power-source-server/PowerSourceCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class BatteryPowerSource : public SingleEndpoint
{
    using Feature                                 = Clusters::PowerSource::Feature;
    constexpr static auto ActiveBatChargeFaultsId = Clusters::PowerSource::Attributes::ActiveBatChargeFaults::Id;
    constexpr static auto ActiveBatFaultsId       = Clusters::PowerSource::Attributes::ActiveBatFaults::Id;
    constexpr static auto BatCapacityId           = Clusters::PowerSource::Attributes::BatCapacity::Id;
    constexpr static auto BatChargingCurrentId    = Clusters::PowerSource::Attributes::BatChargingCurrent::Id;
    constexpr static auto BatPercentRemainingId   = Clusters::PowerSource::Attributes::BatPercentRemaining::Id;
    constexpr static auto BatTimeRemainingId      = Clusters::PowerSource::Attributes::BatTimeRemaining::Id;
    constexpr static auto BatTimeToFullChargeId   = Clusters::PowerSource::Attributes::BatTimeToFullCharge::Id;
    constexpr static auto BatVoltageId            = Clusters::PowerSource::Attributes::BatVoltage::Id;

public:
    enum class ChargingCapability : uint8_t
    {
        kNotRechargeable,
        kRechargeable,
    };

    // Battery-backed Power Source Cluster that can optionally represent a rechargeable battery.
    using SimpleBatteryPowerSourceCluster = Clusters::PowerSourceCluster<
        BitFlags<Feature>(Feature::kBattery, Feature::kRechargeable).Raw(),
        OptionalAttributeSet<ActiveBatChargeFaultsId, ActiveBatFaultsId, BatCapacityId, BatChargingCurrentId, BatPercentRemainingId,
                             BatTimeRemainingId, BatTimeToFullChargeId, BatVoltageId>::All()>;
    // Takes delegates for the clusters.
    BatteryPowerSource(CharSpan description, Clusters::PowerSource::BatReplaceabilityEnum replaceability,
                       TimerDelegate & timerDelegate, uint8_t order = 0,
                       ChargingCapability chargingCapability = ChargingCapability::kNotRechargeable);
    ~BatteryPowerSource() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Accessors for subclasses/implementations to interact with clusters
    SimpleBatteryPowerSourceCluster & BatteryPowerSourceCluster();

protected:
    TimerDelegate & mTimerDelegate;
    CharSpan mDescription;
    Clusters::PowerSource::BatReplaceabilityEnum mReplaceability;
    const uint8_t mOrder;
    const ChargingCapability mChargingCapability;
    EndpointId mEndpointList[1] = { kInvalidEndpointId };

    LazyRegisteredServerCluster<SimpleBatteryPowerSourceCluster> mBatteryPowerSourceCluster;
};

} // namespace chip::app
