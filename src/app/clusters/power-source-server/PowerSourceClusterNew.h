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

#include "Delegates.h"
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>

#include <variant>

namespace chip {
namespace app {
namespace Clusters {

class PowerSourceCluster : public DefaultServerCluster
{
public:
    struct Wired
    {
        PowerSourceDelegate & baseDelegate;
        WiredPowerSourceDelegate & wiredDelegate;
    };

    struct Battery
    {
        PowerSourceDelegate & baseDelegate;
        BatteryDelegate & batteryDelegate;
        ReplacableBatteryDelegate * replacableBatteryDelegate = nullptr;
        RechargeableBatteryDelegate * rechargeableBatteryDelegate = nullptr;
    };

    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
        PowerSource::Attributes::WiredAssessedInputVoltage::Id,
        PowerSource::Attributes::WiredAssessedInputFrequency::Id,
        PowerSource::Attributes::WiredCurrentType::Id,
        PowerSource::Attributes::WiredAssessedCurrent::Id,
        PowerSource::Attributes::WiredNominalVoltage::Id,
        PowerSource::Attributes::WiredMaximumCurrent::Id,
        PowerSource::Attributes::WiredPresent::Id,
        PowerSource::Attributes::ActiveWiredFaults::Id,
        PowerSource::Attributes::BatVoltage::Id,
        PowerSource::Attributes::BatPercentRemaining::Id,
        PowerSource::Attributes::BatTimeRemaining::Id,
        PowerSource::Attributes::BatChargeLevel::Id,
        PowerSource::Attributes::BatReplacementNeeded::Id,
        PowerSource::Attributes::BatReplaceability::Id,
        PowerSource::Attributes::BatPresent::Id,
        PowerSource::Attributes::ActiveBatFaults::Id,
        PowerSource::Attributes::BatReplacementDescription::Id,
        PowerSource::Attributes::BatCommonDesignation::Id,
        PowerSource::Attributes::BatANSIDesignation::Id,
        PowerSource::Attributes::BatIECDesignation::Id,
        PowerSource::Attributes::BatApprovedChemistry::Id,
        PowerSource::Attributes::BatCapacity::Id,
        PowerSource::Attributes::BatQuantity::Id,
        PowerSource::Attributes::BatChargeState::Id,
        PowerSource::Attributes::BatTimeToFullCharge::Id,
        PowerSource::Attributes::BatFunctionalWhileCharging::Id,
        PowerSource::Attributes::BatChargingCurrent::Id,
        PowerSource::Attributes::ActiveBatChargeFaults::Id>

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, const Wired & wiredDelegate) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet), mDelegate(wiredDelegate), mFeatures(WiredFeatures())
    {}

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, const Battery & batteryDelegate) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet), mDelegate(batteryDelegate),
        mFeatures(BatteryFeatures(batteryDelegate.replacableBatteryDelegate != nullptr, batteryDelegate.rechargeableBatteryDelegate != nullptr))
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    const BitFlags<PowerSource::Feature> & Features() const { return mFeatures; }

    void NotifyAttributeChanged(AttributeId attributeId);

private:
    static inline BitFlags<PowerSource::Feature> WiredFeatures()
    {
        return BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired);
    }

    static inline BitFlags<PowerSource::Feature> BatteryFeatures(bool replaceable, bool rechargeable)
    {
        auto flags = BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery);
        return flags.Set(PowerSource::Feature::kReplaceable, replaceable)
                    .Set(PowerSource::Feature::kRechargeable, rechargeable);
    }

    PowerSourceDelegate & GetBaseDelegate()
    {
        return std::visit([](auto & delegate) { return delegate.baseDelegate; }, mDelegate);
    }
    // Precondition: Features().Has(PowerSource::Feature::kWired)
    WiredPowerSourceDelegate & GetWiredDelegate() { return *std::get_if<Wired>(&mDelegate); }
    // Precondition: Features().Has(PowerSource::Feature::kBattery)
    BatteryDelegate & GetBatteryDelegate() { return *std::get_if<Battery>(&mDelegate); }
    // Precondition: Features().Has(PowerSource::Feature::kReplaceable)
    ReplacableBatteryDelegate & GetReplacableBatteryDelegate()
    {
        return *(std::get_if<Battery>(&mDelegate)->replacableBatteryDelegate);
    }
    // Precondition: Features().Has(PowerSource::Feature::kRechargeable)
    RechargeableBatteryDelegate & GetRechargeableBatteryDelegate()
    {
        return *(std::get_if<Battery>(&mDelegate)->rechargeableBatteryDelegate);
    }

    void forceOptionalAttributesValidity();

    const std::variant<Wired, Battery> mDelegate;
    const BitFlags<PowerSource::Feature> mFeatures;
    OptionalAttributeSet mOptionalAttributeSet;
}

} // namespace Clusters
} // namespace app
} // namespace chip
