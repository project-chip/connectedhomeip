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
#include <clusters/PowerSource/Events.h>
#include <lib/support/ScopedBuffer.h>

#include <variant>
#include <algorithm>

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

    struct WiredConfiguration
    {
        CharSpan description;
        WiredCurrentTypeEnum currentType;
        uint32_t nominalVoltage;
        uint32_t maximumCurrent;

        // To force the user to specify these mandatory attributes (taking the corresponding feature into account).
        WiredConfiguration(CharSpan description, WiredCurrentTypeEnum currentType):
            description(description), currentType(currentType)
        {}
    };

    struct BatteryConfiguration
    {
        CharSpan description;
        BatReplaceabilityEnum replaceability;
        CharSpan replacementDescription;
        BatCommonDesignationEnum commonDesignation;
        CharSpan ansiDesignation;
        CharSpan iecDesignation;
        BatApprovedChemistryEnum approvedChemistry;
        uint32_t capacity;
        uint8_t quantity;

        // To force the user to specify mandatory attributes (taking the corresponding features into account).

        BatteryConfiguration(CharSpan description, BatReplaceabilityEnum replaceability):
            replaceable(false), rechargeable(false), description(description), replaceability(replaceability)
        {}

        void MakeReplaceable(BatReplacementDescriptionEnum replacementDescription, uint8_t quantity)
        {
            replaceable = true;
            this->replacementDescription = replacementDescription;
            this->quantity = quantity;
        }

        void MakeRechargeable() { rechargeable = true; }

        bool isReplaceable() { return replaceable; }
        bool isRechargeable() { return rechargeable; }

    private:
        bool replaceable;
        bool rechargeable;
    };

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer, WiredConfiguration config);
    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer, BatteryConfiguration config);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    const BitFlags<PowerSource::Feature> & Features() const { return mFeatures; }

#define VALIDATE_FEATURE(feature_name, attr_name) \
    VerifyOrDieWithMsg(Features().Has(Feature::##feature_name), NotSpecified, "Attempting to read attribute `" #attr_name "` when feature `" #feature_name "` is not set.");

#define VALIDATE_OPTIONAL_ATTRIBUTE(attr_name) \
    VerifyOrDieWithMsg(mOptionalAttributeSet.IsSet(PowerSource::Attributes::##attr_name::Id), NotSpecified, "Attempting to read attribute `" #attr_name "` when it is not specified as supported optional attribute");

    // Getters

    PowerSourceStatusEnum GetStatus() const;
    uint8 GetOrder() const;
    CharSpan GetDescription() const;
    Optional<uint32_t> GetWiredAssessedInputVoltage() const;
    Optional<uint16_t> GetWiredAssessedInputFrequency() const;
    WiredCurrentTypeEnum GetWiredCurrentType() const;
    Optional<uint32_t> GetWiredAssessedCurrent() const;
    uint32_t GetWiredNominalVoltage() const;
    uint32_t GetWiredMaximumCurrent() const;
    bool GetWiredPresent() const;
    Span<WiredFaultEnum> GetActiveWiredFaults() const;
    Optional<uint32_t> GetBatVoltage() const;
    Optional<uint8_t> GetBatPercentRemaining() const;
    Optional<uint32_t> GetBatTimeRemaining() const;
    BatChargeLevelEnum GetBatChargeLevel() const;
    bool GetBatReplacementNeeded() const;
    BatReplaceabilityEnum GetBatReplaceability() const;
    bool GetBatPresent() const;
    Span<BatFaultEnum> GetActiveBatFaults() const;
    CharSpan GetBatReplacementDescription() const;
    BatCommonDesignationEnum GetBatCommonDesignation() const;
    CharSpan GetBatANSIDesignation() const;
    CharSpan GetBatIECDesignation() const;
    BatApprovedChemistryEnum GetBatApprovedChemistry() const;
    uint32_t GetBatCapacity() const;
    uint8_t GetBatQuantity() const;
    BatChargeStateEnum GetBatChargeState() const;
    Optional<uint32> GetBatTimeToFullCharge() const;
    bool GetBatFunctionalWhileCharging() const;
    Optional<uint32_t> GetBatChargingCurrent() const;
    Span<BatChargeFaultEnum> GetActiveBatChargeFaults() const;
    Span<EndpointId> GetEndpointList() const;

    // Setters

    /// Some attributes' setters are private because they are marked with the `Fixed` quality,
    /// It is possible to set them with the constructor. If it is needed to set them after construction,
    /// a setter function named like `SetConfiguration` can be added, or separate setters can be made public.

    /// `Fixed` attributes are `Description`, `WiredCurrentType`, `WiredNominalVoltage`, `WiredMaximumCurrent`, `BatReplaceability`,
    /// `BatReplacementDescription`, `BatCommonDesignation`, `BatANSIDesignation`, `BatIECDesignation`, `BatApprovedChemistry`,
    /// `BatCapacity` and `BatQuantity`

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val);
    CHIP_ERROR SetOrder(uint8_t val);
    CHIP_ERROR SetWiredAssessedInputVoltage(Optional<uint32_t> val);
    CHIP_ERROR SetWiredAssessedInputFrequency(Optional<uint16_t> val);
    CHIP_ERROR SetWiredAssessedCurrent(Optional<uint32_t> val);
    CHIP_ERROR SetWiredPresent(bool val);
    CHIP_ERROR SetActiveWiredFaults(Span<WiredFaultEnum> val);
    CHIP_ERROR SetBatVoltage(Optional<uint32_t> val);
    CHIP_ERROR SetBatPercentRemaining(Optional<uint8_t> val);
    CHIP_ERROR SetBatTimeRemaining(Optional<uint32_t> val);
    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum val);
    CHIP_ERROR SetBatReplacementNeeded(bool val);
    CHIP_ERROR SetBatPresent(bool val);
    CHIP_ERROR SetActiveBatFaults(Span<BatFaultEnum> val);
    CHIP_ERROR SetBatChargeState(BatChargeStateEnum val);
    CHIP_ERROR SetBatTimeToFullCharge(Optional<uint32_t> val);
    CHIP_ERROR SetBatFunctionalWhileCharging(bool val);
    CHIP_ERROR SetBatChargingCurrent(Optional<uint32_t> val);
    CHIP_ERROR SetActiveBatChargeFaults(Span<BatChargeFaultEnum> val);
    CHIP_ERROR SetEndpointList(Span<EndpointId> val);

private:

    // Setters for `Fixed` attributes

    CHIP_ERROR SetDescription(CharSpan val);
    CHIP_ERROR SetWiredCurrentType(WiredCurrentTypeEnum val);
    CHIP_ERROR SetWiredNominalVoltage(uint32_t val);
    CHIP_ERROR SetWiredMaximumCurrent(uint32_t val);
    CHIP_ERROR SetBatReplaceability(BatReplaceabilityEnum val);
    CHIP_ERROR SetBatReplacementDescription(CharSpan val);
    CHIP_ERROR SetBatCommonDesignation(BatCommonDesignationEnum val);
    CHIP_ERROR SetBatANSIDesignation(CharSpan val);
    CHIP_ERROR SetBatIECDesignation(CharSpan val);
    CHIP_ERROR SetBatApprovedChemistry(BatApprovedChemistryEnum val);
    CHIP_ERROR SetBatCapacity(uint32_t val);
    CHIP_ERROR SetBatQuantity(uint8_t val);

public:
    struct WiredAttributes
    {
        Optional<uint32_t> assessedInputVoltage{};
        Optional<uint16_t> assessedInputFrequency{};
        WiredCurrentTypeEnum currentType{};
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
        CharSpan GetReplacementDescription() { return CharSpan(mReplacementDescriptionBuffer, strlen(mReplacementDescriptionBuffer)); }
        BatCommonDesignationEnum commonDesignation = BatCommonDesignationEnum::kUnspecified;
        CharSpan GetANSIDesignation() { return CharSpan(mANSIDesignationBuffer, strlen(mANSIDesignationBuffer)); }
        CharSpan GetIECDesignation() { return CharSpan(mIECDesignationBuffer, strlen(mIECDesignationBuffer)); }
        BatApprovedChemistryEnum approvedChemistry = BatApprovedChemistryEnum::kUnspecified;
        uint8_t quantity{};

        char mReplacementDescriptionBuffer[PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength() + 1] = { 0 };
        char mANSIDesignationBuffer[PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength() + 1] = { 0 };
        char mIECDesignationBuffer[PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength() + 1] = { 0 };
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
        CharSpan GetDescription() { return CharSpan(mDescriptionBuffer, strlen(mDescriptionBuffer)); }
        Span<EndpointId> GetPoweredEndpoints() { return Span(mPoweredEndpointsBuffer.Get(), mPoweredEndpointsCount); };

        char mDescriptionBuffer[PowerSource::Attributes::Description::TypeInfo::MaxLength() + 1] = { 0 };

        Platform::ScopedMemoryBufferWithSize<EndpointId> mPoweredEndpointsBuffer;
        size_t mPoweredEndpointsCount = 0;

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

    template <class T>
    void SetAndNotify(T & current_val, const T & new_val, AttributeId id)
    {
        VerifyOrReturn(current_val != new_val); // no-op if equal

        current_val = new_val;
        NotifyAttributeChanged(id);
    }

    // maxSize without null byte
    CHIP_ERROR SetStringAndNotify(CharSpan val, CharSpan current, char* buffer, size_t maxSize, AttributeId id)
    {
        if (current.data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        auto err = CopyCharSpanToMutableCharSpan(val, Span(buffer, maxSize))
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        buffer[val.size()] = 0; // null byte
        NotifyAttributeChanged(id);
        return CHIP_NO_ERROR;
    }

    EndpointId GetEndpointId()
    {
        return GetPaths()[0].mEndpointId;
    }

    const BitFlags<PowerSource::Feature> mFeatures;
    OptionalAttributeSet mOptionalAttributeSet;
    struct Attributes mAttributes;

    // context
    System::Layer & mSystemLayer;

    // notify timer stuff
    std::atomic_bool mBatPercentRemainingNotifyTimerExpired{true};
    std::atomic_bool mBatTimeRemainingNotifyTimerExpired{true};
    std::atomic_bool mBatTimeToFullChargeNotifyTimerExpired{true};

    constexpr System::Clock::Timeout notifyTimerDuration = System::Clock::Seconds16(10);

    static void SetTimerExpired(System::Layer *, void * pAtomicBool)
    {
        std::atomic_bool* p = reinterpret_cast<std::atomic_bool>(pAtomicBool);
        *p = true;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
} // namespace chip