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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>
#include <lib/support/ScopedBuffer.h>

#include <variant>

namespace chip {
namespace app {
namespace Clusters {

class PowerSourceCluster : public DefaultServerCluster
{
public:
    using PowerSourceStatusEnum = PowerSource::PowerSourceStatusEnum;
    using WiredCurrentTypeEnum = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum = PowerSource::WiredFaultEnum;
    using BatChargeLevelEnum = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum = PowerSource::BatFaultEnum;
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    using BatChargeStateEnum = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum = PowerSource::BatChargeFaultEnum;

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

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
        mFeatures(WiredFeatures())
    {}

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
        mFeatures(BatteryFeatures(batteryDelegate.replacableBatteryDelegate != nullptr, batteryDelegate.rechargeableBatteryDelegate != nullptr))
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    const BitFlags<PowerSource::Feature> & Features() const { return mFeatures; }

    // Getters

    

private:
    struct WiredAttributes
    {
        Optional<uint32_t> assessedInputVoltage{};
        Optional<uint16_t> assessedInputFrequency{};
        WiredCurrentTypeEnum currentType = WiredCurrentTypeEnum::kUnknownEnumValue; // This will be checked before doing anything,
                                                                                    // Is it even allowed to use this value somewhere?
        Optional<uint32_t> assessedCurrent{};
        uint32_t nominalVoltage{};
        uint32_t maximumCurrent{};
        bool isPresent{};
        Span<WiredFaultEnum> GetActiveFaults() { return Span<WiredFaultEnum>(mActiveFaultsBuffer, mActiveFaultsSize); }

        constexpr size_t kMaxActiveFaults = 8;
        size_t mActiveFaultsSize = 0;
        WiredFaultEnum mActiveFaultsBuffer[kMaxActiveFaults];
    };

    struct ReplaceableBatteryAttributes
    {
        CharSpan GetReplacementDescription() { return CharSpan(mReplacementDescriptionBuffer, mReplacementDescriptionSize); }
        BatCommonDesignationEnum commonDesignation = BatCommonDesignationEnum::kUnspecified;
        CharSpan GetAnsiDesignation() { return CharSpan(mAnsiDesignationBuffer, mAnsiDesignationSize); }
        CharSpan GetIECDesignation() { return CharSpan(mIECDesignationBuffer, mIECDesignationSize); }
        BatApprovedChemistryEnum approvedChemistry = BatApprovedChemistryEnum::kUnspecified;
        uint8_t quantity{};

        char mReplacementDescriptionBuffer[PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength() + 1];
        size_t mReplacementDescriptionSize = 0;
        char mAnsiDesignationBuffer[PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength() + 1];
        size_t mAnsiDesignationSize = 0;
        char mIECDesignationBuffer[PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength() + 1];
        size_t mIECDesignationSize = 0;
    };

    struct RechargeableBatteryAttributes
    {
        BatChargeStateEnum chargeState = BatChargeStateEnum::kUnknown;
        Optional<uint32_t> timeToFullCharge{};
        bool functionalWhileCharging{};
        Optional<uint32_t> chargingCurrent{};
        Span<BatChargeFaultEnum> GetActiveChargeFaults() { return Span<BatChargeFaultEnum>(mActiveChargeFaultsBuffer, mActiveChargeFaultsSize); }

        constexpr size_t kMaxActiveChargeFaults = 16;
        size_t mActiveChargeFaultsSize = 0;
        BatChargeFaultEnum mActiveChargeFaultsBuffer[kMaxActiveChargeFaults];
    };

    struct BatteryAttributes
    {
        Optional<uint32_t> voltage{};
        Optional<uint8_t> percentRemaining{};
        Optional<uint32_t> timeRemaining{};
        BatChargeLevelEnum chargeLevel = BatChargeLevelEnum::kOk;
        bool replacementNeeded{};
        BatReplaceabilityEnum replaceability = BatReplaceabilityEnum::kUnspecified;
        bool isPresent{};
        Span<BatFaultEnum> GetActiveFaults() { return Span<BatFaultEnum>(mActiveFaultsBuffer, mActiveFaultsSize); }
        uint32_t capacity{};

        constexpr size_t kMaxActiveFaults = 8;
        size_t mActiveFaultsSize = 0;
        BatFaultEnum mActiveFaultsBuffer[kMaxActiveFaults];

        ReplaceableBatteryAttributes replaceable{};
        RechargeableBatteryAttributes rechargeable{};
    };

    struct Attributes
    {
        PowerSourceStatusEnum status = PowerSourceStatusEnum::kUnspecified;
        uint8_t order{};
        CharSpan GetDescription() { return CharSpan(mDescriptionBuffer, mDescriptionSize); }
        Span<EndpointId> GetPoweredEndpoints() { return mPoweredEndpointsBuffer.Span(); };

        char mDescriptionBuffer[PowerSource::Attributes::Description::TypeInfo::MaxLength() + 1];
        size_t mDescriptionSize = 0;

        Platform::ScopedMemoryBufferWithSize<EndpointId> mPoweredEndpointsBuffer;

        union
        {
            WiredAttributes wired;
            BatteryAttributes battery;
        };
    };

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

    void forceOptionalAttributesValidity();

    const BitFlags<PowerSource::Feature> mFeatures;
    OptionalAttributeSet mOptionalAttributeSet;
    struct Attributes mAttributes;
}

} // namespace Clusters
} // namespace app
} // namespace chip
