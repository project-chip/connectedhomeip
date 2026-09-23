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

#include "Modules.h"

#include <algorithm>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>
#include <clusters/PowerSource/Events.h>
#include <clusters/PowerSource/Metadata.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/TimerDelegate.h>

#include <limits>

namespace chip::app::Clusters {

namespace PowerSource::detail {

template <size_t MaxVal>
constexpr static BitSetType SpanToBitSet(Span<const uint8_t> span)
{
    static_assert(MaxVal < kBitSetSize, "MaxVal must be less than kBitSetSize to fit in the BitSetType");
    BitSetType val{};
    for (uint8_t el : span)
    {
        if (el < MaxVal)
        {
            val.set(el);
        }
    }
    return val;
}

template <class = void> // to silence "unused" warnings
static void BitSetToSpan(BitSetType bitset, Span<uint8_t> & buffer)
{
    size_t bufInd = 0;
    for (uint8_t i = 0; i < bitset.size() && bufInd < buffer.size(); i++)
    {
        if (bitset.test(i))
        {
            buffer[bufInd++] = i;
        }
    }
    buffer.reduce_size(bufInd);
}

template <class To, class From, class = std::enable_if_t<sizeof(From) == sizeof(To), void>>
constexpr static Span<To> ConvertSpanType(Span<From> span)
{
    return Span(reinterpret_cast<To *>(span.data()), span.size());
}

template <typename T>
CHIP_ERROR EncodeListOfValues(AttributeValueEncoder & encoder, const T & valueList)
{
    return encoder.EncodeList([&valueList](const auto & enc) -> CHIP_ERROR {
        for (const auto & value : valueList)
        {
            ReturnErrorOnFailure(enc.Encode(value));
        }

        return CHIP_NO_ERROR;
    });
}

constexpr static AttributeSet GetValidOptionalAttributeSet(AttributeSet optionalAttributeSet,
                                                           BitFlags<PowerSource::Feature> features)
{
    using namespace PowerSource::Attributes;

    constexpr uint32_t wiredMandatoryAttributeBits = OptionalAttributeSet<WiredCurrentType::Id>::All();
    constexpr uint32_t wiredAttributeBits =
        OptionalAttributeSet<WiredAssessedInputVoltage::Id, WiredAssessedInputFrequency::Id, WiredCurrentType::Id,
                             WiredAssessedCurrent::Id, WiredNominalVoltage::Id, WiredMaximumCurrent::Id, WiredPresent::Id,
                             ActiveWiredFaults::Id>::All();

    constexpr uint32_t batteryMandatoryAttributeBits =
        OptionalAttributeSet<BatChargeLevel::Id, BatReplacementNeeded::Id, BatReplaceability::Id>::All();
    constexpr uint32_t batteryAttributeBits =
        OptionalAttributeSet<BatVoltage::Id, BatPercentRemaining::Id, BatTimeRemaining::Id, BatChargeLevel::Id,
                             BatReplacementNeeded::Id, BatReplaceability::Id, BatPresent::Id, ActiveBatFaults::Id>::All();

    constexpr uint32_t replaceableBatteryMandatoryAttributeBits =
        OptionalAttributeSet<BatReplacementDescription::Id, BatQuantity::Id>::All();
    constexpr uint32_t replaceableBatteryAttributeBits =
        OptionalAttributeSet<BatReplacementDescription::Id, BatCommonDesignation::Id, BatANSIDesignation::Id, BatIECDesignation::Id,
                             BatApprovedChemistry::Id, BatQuantity::Id>::All();

    constexpr uint32_t rechargeableBatteryMandatoryAttributeBits =
        OptionalAttributeSet<BatChargeState::Id, BatFunctionalWhileCharging::Id>::All();
    constexpr uint32_t rechargeableBatteryAttributeBits =
        OptionalAttributeSet<BatChargeState::Id, BatTimeToFullCharge::Id, BatFunctionalWhileCharging::Id, BatChargingCurrent::Id,
                             ActiveBatChargeFaults::Id>::All();

    constexpr uint32_t capacityAttributeBit = OptionalAttributeSet<BatCapacity::Id>::All();

    uint32_t mandatoryBits = 0;
    uint32_t disabledBits  = 0;

    if (features.Has(PowerSource::Feature::kWired))
    {
        mandatoryBits |= wiredMandatoryAttributeBits;
    }
    else
    {
        disabledBits |= wiredAttributeBits;
    }

    if (features.Has(PowerSource::Feature::kBattery))
    {
        mandatoryBits |= batteryMandatoryAttributeBits;
    }
    else
    {
        disabledBits |= batteryAttributeBits;
    }

    if (features.Has(PowerSource::Feature::kReplaceable))
    {
        mandatoryBits |= replaceableBatteryMandatoryAttributeBits;
    }
    else
    {
        disabledBits |= replaceableBatteryAttributeBits;
    }

    if (features.Has(PowerSource::Feature::kRechargeable))
    {
        mandatoryBits |= rechargeableBatteryMandatoryAttributeBits;
    }
    else
    {
        disabledBits |= rechargeableBatteryAttributeBits;
    }

    if (!(features.Has(PowerSource::Feature::kReplaceable) || features.Has(PowerSource::Feature::kRechargeable)))
    {
        disabledBits |= capacityAttributeBit;
    }

    uint32_t finalBits = (optionalAttributeSet.Raw() | mandatoryBits) & ~disabledBits;
    return AttributeSet(finalBits);
}

} // namespace PowerSource::detail
using PowerSourceOptionalAttributeSet =
    app::OptionalAttributeSet<PowerSource::Attributes::WiredAssessedInputVoltage::Id,
                              PowerSource::Attributes::WiredAssessedInputFrequency::Id,
                              PowerSource::Attributes::WiredAssessedCurrent::Id, PowerSource::Attributes::WiredNominalVoltage::Id,
                              PowerSource::Attributes::WiredMaximumCurrent::Id, PowerSource::Attributes::WiredPresent::Id,
                              PowerSource::Attributes::ActiveWiredFaults::Id, PowerSource::Attributes::BatVoltage::Id,
                              PowerSource::Attributes::BatPercentRemaining::Id, PowerSource::Attributes::BatTimeRemaining::Id,
                              PowerSource::Attributes::BatPresent::Id, PowerSource::Attributes::ActiveBatFaults::Id,
                              PowerSource::Attributes::BatCommonDesignation::Id, PowerSource::Attributes::BatANSIDesignation::Id,
                              PowerSource::Attributes::BatIECDesignation::Id, PowerSource::Attributes::BatApprovedChemistry::Id,
                              PowerSource::Attributes::BatCapacity::Id, PowerSource::Attributes::BatTimeToFullCharge::Id,
                              PowerSource::Attributes::BatChargingCurrent::Id, PowerSource::Attributes::ActiveBatChargeFaults::Id>;
template <std::underlying_type_t<PowerSource::Feature> supportedFeatureBits, uint32_t supportedOptionalAttributeBits>
struct PowerSourceClusterConfig : public PowerSource::detail::AllModules<supportedFeatureBits, supportedOptionalAttributeBits>
{
    constexpr static BitFlags<PowerSource::Feature> supportedFeatures{ supportedFeatureBits };
    static_assert(supportedFeatures.Has(PowerSource::Feature::kWired) ^ supportedFeatures.Has(PowerSource::Feature::kBattery),
                  "Exactly one of Wired or Battery features must be set");

    constexpr static AttributeSet supportedOptionalAttributeSet =
        PowerSource::detail::GetValidOptionalAttributeSet(AttributeSet(supportedOptionalAttributeBits), supportedFeatures);

    PowerSourceClusterConfig(CharSpan desc, PowerSource::WiredCurrentTypeEnum currType)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kWired),
                      "This constructor should only be used for a Wired power source configuration");
        this->description      = desc;
        this->wiredCurrentType = currType;
    }

    PowerSourceClusterConfig(CharSpan desc, PowerSource::BatReplaceabilityEnum replability, TimerDelegate & timerDelegate)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kBattery),
                      "This constructor should only be used for a Battery power source configuration");
        this->description       = desc;
        this->batReplaceability = replability;
        this->mTimerDelegate    = &timerDelegate;
    }

    TimerDelegate & GetTimerDelegate()
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kBattery),
                      "TimerDelegate is only relevant for a Battery power source configuration");
        return *this->mTimerDelegate;
    }

    PowerSourceClusterConfig & MakeReplaceable(CharSpan replDesc, uint8_t quan)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kReplaceable),
                      "This method can only be used for a power source configuration that supports being replaceable");
        this->batReplacementDescription = replDesc;
        this->batQuantity               = quan;

        this->batReplaceable = true;
        return *this;
    }

    PowerSourceClusterConfig & MakeRechargeable()
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kRechargeable),
                      "This method can only be used for a power source configuration that supports being rechargeable");
        this->batRechargeable = true;
        return *this;
    }

    PowerSourceOptionalAttributeSet usedOptionalAttributes{ UINT32_MAX }; // all supported attributes are marked as used by default.
};

template <std::underlying_type_t<PowerSource::Feature> supportedFeatureBits, uint32_t supportedOptionalAttributeBits>
class PowerSourceCluster : protected PowerSourceClusterConfig<supportedFeatureBits, supportedOptionalAttributeBits>,
                           public DefaultServerCluster,
                           public PowerSource::detail::BatteryTimerContext::NotifierDelegate,
                           protected PowerSource::detail::BatteryTimerContextsModule<
                               BitFlags<PowerSource::Feature>(supportedFeatureBits).Has(PowerSource::Feature::kBattery)>
{
public:
    using Config = PowerSourceClusterConfig<supportedFeatureBits, supportedOptionalAttributeBits>;
    using Config::supportedFeatures;
    using Config::supportedOptionalAttributeSet;
    using BitSetType = typename PowerSource::detail::BitSetType;

    using PowerSourceStatusEnum    = PowerSource::PowerSourceStatusEnum;
    using WiredCurrentTypeEnum     = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum           = PowerSource::WiredFaultEnum;
    using BatChargeLevelEnum       = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum    = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum             = PowerSource::BatFaultEnum;
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    using BatChargeStateEnum       = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum       = PowerSource::BatChargeFaultEnum;

    static_assert(supportedFeatures.Has(PowerSource::Feature::kWired) ^ supportedFeatures.Has(PowerSource::Feature::kBattery),
                  "Exactly one of Wired or Battery features must be set");

    template <bool batteryFeatureNotSupported                   = !supportedFeatures.Has(PowerSource::Feature::kBattery),
              std::enable_if_t<batteryFeatureNotSupported, int> = 0>
    PowerSourceCluster(EndpointId endpointId, const Config & config) :
        Config(config), DefaultServerCluster({ endpointId, PowerSource::Id })
    {}

    template <bool batteryFeatureSupported                      = supportedFeatures.Has(PowerSource::Feature::kBattery),
              std::enable_if_t<batteryFeatureSupported, size_t> = 0>
    PowerSourceCluster(EndpointId endpointId, const Config & config) :
        Config(config), DefaultServerCluster({ endpointId, PowerSource::Id }),
        PowerSource::detail::BatteryTimerContextsModule<batteryFeatureSupported>(Config::GetTimerDelegate(), *this)
    {}

    // implement `PowerSource::detail::BatteryTimerContext::NotifierDelegate`
    void Notify(AttributeId id) override { NotifyAttributeChanged(id); }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        using namespace PowerSource::Attributes;
        using namespace PowerSource::detail;
        // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is
        // valid.
        AttributeId id = request.path.mAttributeId;

        switch (id)
        {
        case Status::Id:
            return encoder.Encode(this->status);

        case Order::Id:
            return encoder.Encode(this->order);

        case Description::Id:
            return encoder.Encode(
                this->description.SubSpan(0, std::min(this->description.size(), Description::TypeInfo::MaxLength())));

        case EndpointList::Id:
            return EncodeListOfValues(encoder, GetEndpointList());

        case Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(Features().Raw());

        case Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(PowerSource::kRevision);

        // Feature dependent and optional attributes
        case WiredAssessedInputVoltage::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedInputVoltage::Id))
            {
                return encoder.Encode(this->wiredAssessedInputVoltage);
            }
            break;

        case WiredAssessedInputFrequency::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedInputFrequency::Id))
            {
                return encoder.Encode(this->wiredAssessedInputFrequency);
            }
            break;

        case WiredCurrentType::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredCurrentType::Id))
            {
                return encoder.Encode(this->wiredCurrentType);
            }
            break;

        case WiredAssessedCurrent::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedCurrent::Id))
            {
                return encoder.Encode(this->wiredAssessedCurrent);
            }
            break;

        case WiredNominalVoltage::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredNominalVoltage::Id))
            {
                return encoder.Encode(this->wiredNominalVoltage);
            }
            break;

        case WiredMaximumCurrent::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredMaximumCurrent::Id))
            {
                return encoder.Encode(this->wiredMaximumCurrent);
            }
            break;

        case WiredPresent::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(WiredPresent::Id))
            {
                return encoder.Encode(this->wiredPresent);
            }
            break;

        case ActiveWiredFaults::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(ActiveWiredFaults::Id))
            {
                uint8_t faultsBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeWiredFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
            break;

        case BatVoltage::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatVoltage::Id))
            {
                return encoder.Encode(this->batVoltage);
            }
            break;

        case BatPercentRemaining::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatPercentRemaining::Id))
            {
                return encoder.Encode(this->batPercentRemaining);
            }
            break;

        case BatTimeRemaining::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatTimeRemaining::Id))
            {
                return encoder.Encode(this->batTimeRemaining);
            }
            break;

        case BatChargeLevel::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatChargeLevel::Id))
            {
                return encoder.Encode(this->batChargeLevel);
            }
            break;

        case BatReplacementNeeded::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatReplacementNeeded::Id))
            {
                return encoder.Encode(this->batReplacementNeeded);
            }
            break;

        case BatReplaceability::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatReplaceability::Id))
            {
                return encoder.Encode(this->batReplaceability);
            }
            break;

        case BatPresent::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatPresent::Id))
            {
                return encoder.Encode(this->batPresent);
            }
            break;

        case ActiveBatFaults::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(ActiveBatFaults::Id))
            {
                uint8_t faultsBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeBatFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
            break;

        case BatReplacementDescription::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatReplacementDescription::Id))
            {
                return encoder.Encode(this->batReplacementDescription.SubSpan(
                    0, std::min(this->batReplacementDescription.size(), BatReplacementDescription::TypeInfo::MaxLength())));
            }
            break;

        case BatCommonDesignation::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatCommonDesignation::Id))
            {
                return encoder.Encode(this->batCommonDesignation);
            }
            break;

        case BatANSIDesignation::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatANSIDesignation::Id))
            {
                return encoder.Encode(this->batANSIDesignation.SubSpan(
                    0, std::min(this->batANSIDesignation.size(), BatANSIDesignation::TypeInfo::MaxLength())));
            }
            break;

        case BatIECDesignation::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatIECDesignation::Id))
            {
                return encoder.Encode(this->batIECDesignation.SubSpan(
                    0, std::min(this->batIECDesignation.size(), BatIECDesignation::TypeInfo::MaxLength())));
            }
            break;

        case BatApprovedChemistry::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatApprovedChemistry::Id))
            {
                return encoder.Encode(this->batApprovedChemistry);
            }
            break;

        case BatCapacity::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatCapacity::Id))
            {
                return encoder.Encode(this->batCapacity);
            }
            break;

        case BatQuantity::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatQuantity::Id))
            {
                return encoder.Encode(this->batQuantity);
            }
            break;

        case BatChargeState::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatChargeState::Id))
            {
                return encoder.Encode(this->batChargeState);
            }
            break;

        case BatTimeToFullCharge::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatTimeToFullCharge::Id))
            {
                return encoder.Encode(this->batTimeToFullCharge);
            }
            break;

        case BatFunctionalWhileCharging::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatFunctionalWhileCharging::Id))
            {
                return encoder.Encode(this->batFunctionalWhileCharging);
            }
            break;

        case BatChargingCurrent::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(BatChargingCurrent::Id))
            {
                return encoder.Encode(this->batChargingCurrent);
            }
            break;

        case ActiveBatChargeFaults::Id:
            if constexpr (supportedOptionalAttributeSet.IsSet(ActiveBatChargeFaults::Id))
            {
                uint8_t faultsBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeBatChargeFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
            break;

        default:
            break;
        }

        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        using namespace PowerSource::Attributes;
        AttributeListBuilder attributeListBuilder(builder);

        AttributeSet optionalAttributeSet(
            PowerSource::detail::GetValidOptionalAttributeSet(this->usedOptionalAttributes, Features()).Raw() &
            supportedOptionalAttributeSet.Raw());

        constexpr DataModel::AttributeEntry kOptionalAttributes[] = { WiredAssessedInputVoltage::kMetadataEntry,
                                                                      WiredAssessedInputFrequency::kMetadataEntry,
                                                                      WiredCurrentType::kMetadataEntry,
                                                                      WiredAssessedCurrent::kMetadataEntry,
                                                                      WiredNominalVoltage::kMetadataEntry,
                                                                      WiredMaximumCurrent::kMetadataEntry,
                                                                      WiredPresent::kMetadataEntry,
                                                                      ActiveWiredFaults::kMetadataEntry,
                                                                      BatVoltage::kMetadataEntry,
                                                                      BatPercentRemaining::kMetadataEntry,
                                                                      BatTimeRemaining::kMetadataEntry,
                                                                      BatChargeLevel::kMetadataEntry,
                                                                      BatReplacementNeeded::kMetadataEntry,
                                                                      BatReplaceability::kMetadataEntry,
                                                                      BatPresent::kMetadataEntry,
                                                                      ActiveBatFaults::kMetadataEntry,
                                                                      BatReplacementDescription::kMetadataEntry,
                                                                      BatCommonDesignation::kMetadataEntry,
                                                                      BatANSIDesignation::kMetadataEntry,
                                                                      BatIECDesignation::kMetadataEntry,
                                                                      BatApprovedChemistry::kMetadataEntry,
                                                                      BatCapacity::kMetadataEntry,
                                                                      BatQuantity::kMetadataEntry,
                                                                      BatChargeState::kMetadataEntry,
                                                                      BatTimeToFullCharge::kMetadataEntry,
                                                                      BatFunctionalWhileCharging::kMetadataEntry,
                                                                      BatChargingCurrent::kMetadataEntry,
                                                                      ActiveBatChargeFaults::kMetadataEntry };

        return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(kOptionalAttributes), optionalAttributeSet);
    }

#define ENABLE_IF_ATTRIBUTE_SUPPORTED(attrName)                                                                                    \
    static_assert(supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id),                                      \
                  "This method can only be used if the " #attrName " attribute is supported");                                     \
    if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id))                                     \
    {                                                                                                                              \
        return {};                                                                                                                 \
    }                                                                                                                              \
    else

#define ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(attrName)                                                                          \
    static_assert(supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id),                                      \
                  "This method can only be used if the " #attrName " attribute is supported");                                     \
    if constexpr (supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id))

    // Getters

    PowerSourceStatusEnum GetStatus() const { return this->status; }
    uint8_t GetOrder() const { return this->order; }
    CharSpan GetDescription() const { return this->description; }
    DataModel::Nullable<uint32_t> GetWiredAssessedInputVoltage() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedInputVoltage)
        {
            return this->wiredAssessedInputVoltage;
        }
    }
    DataModel::Nullable<uint16_t> GetWiredAssessedInputFrequency() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedInputFrequency)
        {
            return this->wiredAssessedInputFrequency;
        }
    }
    WiredCurrentTypeEnum GetWiredCurrentType() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredCurrentType)
        {
            return this->wiredCurrentType;
        }
    }
    DataModel::Nullable<uint32_t> GetWiredAssessedCurrent() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedCurrent)
        {
            return this->wiredAssessedCurrent;
        }
    }
    uint32_t GetWiredNominalVoltage() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredNominalVoltage)
        {
            return this->wiredNominalVoltage;
        }
    }
    uint32_t GetWiredMaximumCurrent() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredMaximumCurrent)
        {
            return this->wiredMaximumCurrent;
        }
    }
    bool GetWiredPresent() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredPresent)
        {
            return this->wiredPresent;
        }
    }
    void GetActiveWiredFaults(Span<WiredFaultEnum> & buffer) const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveWiredFaults)
        {
            auto span = PowerSource::detail::ConvertSpanType<uint8_t>(buffer);
            PowerSource::detail::BitSetToSpan(this->activeWiredFaultsBitSet, span);
            buffer = PowerSource::detail::ConvertSpanType<WiredFaultEnum>(span);
        }
    }
    DataModel::Nullable<uint32_t> GetBatVoltage() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatVoltage)
        {
            return this->batVoltage;
        }
    }
    DataModel::Nullable<uint8_t> GetBatPercentRemaining() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPercentRemaining)
        {
            return this->batPercentRemaining;
        }
    }
    DataModel::Nullable<uint32_t> GetBatTimeRemaining() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeRemaining)
        {
            return this->batTimeRemaining;
        }
    }
    BatChargeLevelEnum GetBatChargeLevel() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeLevel)
        {
            return this->batChargeLevel;
        }
    }
    bool GetBatReplacementNeeded() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatReplacementNeeded)
        {
            return this->batReplacementNeeded;
        }
    }
    BatReplaceabilityEnum GetBatReplaceability() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatReplaceability)
        {
            return this->batReplaceability;
        }
    }
    bool GetBatPresent() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPresent)
        {
            return this->batPresent;
        }
    }
    void GetActiveBatFaults(Span<BatFaultEnum> & buffer) const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveBatFaults)
        {
            auto span = PowerSource::detail::ConvertSpanType<uint8_t>(buffer);
            PowerSource::detail::BitSetToSpan(this->activeBatFaultsBitSet, span);
            buffer = PowerSource::detail::ConvertSpanType<BatFaultEnum>(span);
        }
    }
    CharSpan GetBatReplacementDescription() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatReplacementDescription)
        {
            return this->batReplacementDescription;
        }
    }
    BatCommonDesignationEnum GetBatCommonDesignation() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatCommonDesignation)
        {
            return this->batCommonDesignation;
        }
    }
    CharSpan GetBatANSIDesignation() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatANSIDesignation)
        {
            return this->batANSIDesignation;
        }
    }
    CharSpan GetBatIECDesignation() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatIECDesignation)
        {
            return this->batIECDesignation;
        }
    }
    BatApprovedChemistryEnum GetBatApprovedChemistry() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatApprovedChemistry)
        {
            return this->batApprovedChemistry;
        }
    }
    uint32_t GetBatCapacity() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatCapacity)
        {
            return this->batCapacity;
        }
    }
    uint8_t GetBatQuantity() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatQuantity)
        {
            return this->batQuantity;
        }
    }
    BatChargeStateEnum GetBatChargeState() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeState)
        {
            return this->batChargeState;
        }
    }
    DataModel::Nullable<uint32_t> GetBatTimeToFullCharge() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeToFullCharge)
        {
            return this->batTimeToFullCharge;
        }
    }
    bool GetBatFunctionalWhileCharging() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatFunctionalWhileCharging)
        {
            return this->batFunctionalWhileCharging;
        }
    }
    DataModel::Nullable<uint32_t> GetBatChargingCurrent() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargingCurrent)
        {
            return this->batChargingCurrent;
        }
    }
    void GetActiveBatChargeFaults(Span<BatChargeFaultEnum> & buffer) const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveBatChargeFaults)
        {
            auto span = PowerSource::detail::ConvertSpanType<uint8_t>(buffer);
            PowerSource::detail::BitSetToSpan(this->activeBatChargeFaultsBitSet, span);
            buffer = PowerSource::detail::ConvertSpanType<BatChargeFaultEnum>(span);
        }
    }
    Span<const EndpointId> GetEndpointList() const { return this->endpointList; }

    // Setters

    /// Attributes marked with the `Fixed` quality do not have setters.
    /// They can be only set during construction using the `PowerSourceClusterConfig` class.
    /// `Fixed` attributes are `Description`, `WiredCurrentType`, `WiredNominalVoltage`, `WiredMaximumCurrent`,
    /// `BatReplaceability`, `BatReplacementDescription`, `BatCommonDesignation`, `BatANSIDesignation`, `BatIECDesignation`,
    /// `BatApprovedChemistry`, `BatCapacity`, `BatQuantity`.

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val)
    {
        VerifyOrReturnError(val != PowerSourceStatusEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
        SetAttributeValue(this->status, val, PowerSource::Attributes::Status::Id);
        return CHIP_NO_ERROR;
    }
    void SetOrder(uint8_t val)
    {
        // This attribute is marked as `Persistent` but the cluster will not support the persistence because the attribute is
        // `ReadOnly` to clients.

        SetAttributeValue(this->order, val, PowerSource::Attributes::Order::Id);
    }
    void SetWiredAssessedInputVoltage(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(WiredAssessedInputVoltage)
        {
            this->wiredAssessedInputVoltage = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
        }
    }
    void SetWiredAssessedInputFrequency(DataModel::Nullable<uint16_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(WiredAssessedInputFrequency)
        {
            this->wiredAssessedInputFrequency = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
        }
    }
    void SetWiredAssessedCurrent(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(WiredAssessedCurrent)
        {
            this->wiredAssessedCurrent = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
        }
    }
    void SetWiredPresent(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(WiredPresent)
        {
            SetAttributeValue(this->wiredPresent, val, PowerSource::Attributes::WiredPresent::Id);
        }
    }
    void SetActiveWiredFaults(Span<const WiredFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveWiredFaults)
        {
            BitSetType bitset = PowerSource::detail::SpanToBitSet<to_underlying(WiredFaultEnum::kUnknownEnumValue)>(
                PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeWiredFaultsBitSet != bitset)
            {
                GenerateWiredFaultEventAndSetAndNotify(bitset);
            }
        }
    }
    CHIP_ERROR AddActiveWiredFault(WiredFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveWiredFaults)
        {
            if (val == WiredFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (this->activeWiredFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeWiredFaultsBitSet;
            newBitSet.set(to_underlying(val));

            GenerateWiredFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveWiredFault(WiredFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveWiredFaults)
        {
            if (val == WiredFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!this->activeWiredFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeWiredFaultsBitSet;
            newBitSet.set(to_underlying(val), false);

            GenerateWiredFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    void SetBatVoltage(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(BatVoltage)
        {
            this->batVoltage = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
        }
    }
    CHIP_ERROR SetBatPercentRemaining(DataModel::Nullable<uint8_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPercentRemaining)
        {
            if (!val.IsNull())
            {
                // Maximum value of 200 representing 100% battery.
                VerifyOrReturnError(val.Value() <= 200, CHIP_IM_GLOBAL_STATUS(ConstraintError));
            }

            return SetQuietNullableAttribute(this->batPercentRemaining, val, this->batPercentRemainingNotifyTimerContext);
        }
    }
    CHIP_ERROR SetBatTimeRemaining(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeRemaining)
        {
            return SetQuietNullableAttribute(this->batTimeRemaining, val, this->batTimeRemainingNotifyTimerContext);
        }
    }
    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeLevel)
        {
            VerifyOrReturnError(val != BatChargeLevelEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
            SetAttributeValue(this->batChargeLevel, val, PowerSource::Attributes::BatChargeLevel::Id);
            return CHIP_NO_ERROR;
        }
    }
    void SetBatReplacementNeeded(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(BatReplacementNeeded)
        {
            SetAttributeValue(this->batReplacementNeeded, val, PowerSource::Attributes::BatReplacementNeeded::Id);
        }
    }
    void SetBatPresent(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(BatPresent)
        {
            SetAttributeValue(this->batPresent, val, PowerSource::Attributes::BatPresent::Id);
        }
    }
    void SetActiveBatFaults(Span<const BatFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveBatFaults)
        {
            BitSetType newBitSet = PowerSource::detail::SpanToBitSet<to_underlying(BatFaultEnum::kUnknownEnumValue)>(
                PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeBatFaultsBitSet != newBitSet)
            {
                GenerateBatFaultEventAndSetAndNotify(newBitSet);
            }
        }
    }
    CHIP_ERROR AddActiveBatFault(BatFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatFaults)
        {
            if (val == BatFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (this->activeBatFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeBatFaultsBitSet;
            newBitSet.set(to_underlying(val));

            GenerateBatFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveBatFault(BatFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatFaults)
        {
            if (val == BatFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!this->activeBatFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeBatFaultsBitSet;
            newBitSet.set(to_underlying(val), false);

            GenerateBatFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatChargeState(BatChargeStateEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeState)
        {
            VerifyOrReturnError(val != BatChargeStateEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
            SetAttributeValue(this->batChargeState, val, PowerSource::Attributes::BatChargeState::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatTimeToFullCharge(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeToFullCharge)
        {
            return SetQuietNullableAttribute(this->batTimeToFullCharge, val, this->batTimeToFullChargeNotifyTimerContext);
        }
    }
    void SetBatFunctionalWhileCharging(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(BatFunctionalWhileCharging)
        {
            SetAttributeValue(this->batFunctionalWhileCharging, val, PowerSource::Attributes::BatFunctionalWhileCharging::Id);
        }
    }
    void SetBatChargingCurrent(DataModel::Nullable<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(BatChargingCurrent)
        {
            this->batChargingCurrent = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
        }
    }
    void SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(ActiveBatChargeFaults)
        {
            BitSetType newBitSet = PowerSource::detail::SpanToBitSet<to_underlying(BatChargeFaultEnum::kUnknownEnumValue)>(
                PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeBatChargeFaultsBitSet != newBitSet)
            {
                GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            }
        }
    }
    CHIP_ERROR AddActiveBatChargeFault(BatChargeFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatChargeFaults)
        {
            if (val == BatChargeFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (this->activeBatChargeFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeBatChargeFaultsBitSet;
            newBitSet.set(to_underlying(val));

            GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveBatChargeFault(BatChargeFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatChargeFaults)
        {
            if (val == BatChargeFaultEnum::kUnknownEnumValue)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!this->activeBatChargeFaultsBitSet.test(to_underlying(val)))
            {
                return CHIP_NO_ERROR;
            }

            BitSetType newBitSet = this->activeBatChargeFaultsBitSet;
            newBitSet.set(to_underlying(val), false);

            GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }

    /// This function takes the list by view, it doesn't store the data into the cluster.
    /// If `CHIP_NO_ERROR` is returned, it is guaranteed that the internal `endpointList` points to the new storage, EVEN IF the
    /// content is the same. If something other then `CHIP_NO_ERROR` is returned, the internal `endpointList` is not updated and
    /// still points to the old storage. The underlying storage of `val` needs to be valid until this function is called again and a
    /// `CHIP_NO_ERROR` is returned.
    CHIP_ERROR SetEndpointList(Span<const EndpointId> val)
    {
        if (GetEndpointList().data_equal(val))
        {
            this->endpointList = val;
            return CHIP_NO_ERROR;
        }

        for (const auto & endpointId : val)
        {
            VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
        }

        this->endpointList = val;
        NotifyAttributeChanged(PowerSource::Attributes::EndpointList::Id);
        return CHIP_NO_ERROR;
    }

#undef ENABLE_IF_ATTRIBUTE_SUPPORTED
#undef ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN

private:
    BitFlags<PowerSource::Feature> Features() const
    {
        if constexpr (supportedFeatures.Has(PowerSource::Feature::kWired))
        {
            return BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired);
        }
        else
        {
            return BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery)
                .Set(PowerSource::Feature::kReplaceable, this->batReplaceable)
                .Set(PowerSource::Feature::kRechargeable, this->batRechargeable);
        }
    }

    template <class T>
    CHIP_ERROR SetQuietNullableAttribute(DataModel::Nullable<T> & oldVal, DataModel::Nullable<T> newVal,
                                         PowerSource::detail::BatteryTimerContext & timerContext)
    {
        if (oldVal == newVal)
        {
            return CHIP_NO_ERROR;
        }

        if (oldVal.IsNull() || newVal.IsNull())
        {
            // Cancel the current timer to guarantee a notification with `NotifyOrSchedule` if needed
            timerContext.CancelCurrentTimer();
        }

        oldVal = newVal;
        if (!newVal.IsNull())
        {
            // if not a null value, notify with a timer to avoid too frequent notifications with non null values.
            LogErrorOnFailure(timerContext.NotifyOrSchedule(newVal.Value()));
        }
        else
        {
            // if new value is null, notify immediately without timer, so a new non null value can be notified immediately without
            // waiting for the timer.
            NotifyAttributeChanged(timerContext.GetAttributeId());
        }
        return CHIP_NO_ERROR;
    }

    void GenerateWiredFaultEventAndSetAndNotify(BitSetType newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveWiredFaults::Id))
        {
            return;
        }

        if (mContext != nullptr)
        {
            WiredFaultEnum oldBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)]{};
            WiredFaultEnum newBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)]{};

            auto oldSpan      = Span(oldBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
            auto oldSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(oldSpan);
            PowerSource::detail::BitSetToSpan(this->activeWiredFaultsBitSet, oldSpanBytes);
            oldSpan.reduce_size(oldSpanBytes.size());

            auto newSpan      = Span(newBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
            auto newSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(newSpan);
            PowerSource::detail::BitSetToSpan(newBitSet, newSpanBytes);
            newSpan.reduce_size(newSpanBytes.size());

            PowerSource::Events::WiredFaultChange::Type event_data{ oldSpan, newSpan };
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        SetAttributeValue(this->activeWiredFaultsBitSet, newBitSet, PowerSource::Attributes::ActiveWiredFaults::Id);
    }

    void GenerateBatFaultEventAndSetAndNotify(BitSetType newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveBatFaults::Id))
        {
            return;
        }

        if (mContext != nullptr)
        {
            BatFaultEnum oldBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)]{};
            BatFaultEnum newBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)]{};

            auto oldSpan      = Span(oldBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
            auto oldSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(oldSpan);
            PowerSource::detail::BitSetToSpan(this->activeBatFaultsBitSet, oldSpanBytes);
            oldSpan.reduce_size(oldSpanBytes.size());

            auto newSpan      = Span(newBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
            auto newSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(newSpan);
            PowerSource::detail::BitSetToSpan(newBitSet, newSpanBytes);
            newSpan.reduce_size(newSpanBytes.size());

            PowerSource::Events::BatFaultChange::Type event_data{ oldSpan, newSpan };
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        SetAttributeValue(this->activeBatFaultsBitSet, newBitSet, PowerSource::Attributes::ActiveBatFaults::Id);
    }

    void GenerateBatChargeFaultEventAndSetAndNotify(BitSetType newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveBatChargeFaults::Id))
        {
            return;
        }

        if (mContext != nullptr)
        {
            BatChargeFaultEnum oldBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)]{};
            BatChargeFaultEnum newBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)]{};

            auto oldSpan      = Span(oldBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
            auto oldSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(oldSpan);
            PowerSource::detail::BitSetToSpan(this->activeBatChargeFaultsBitSet, oldSpanBytes);
            oldSpan.reduce_size(oldSpanBytes.size());

            auto newSpan      = Span(newBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
            auto newSpanBytes = PowerSource::detail::ConvertSpanType<uint8_t>(newSpan);
            PowerSource::detail::BitSetToSpan(newBitSet, newSpanBytes);
            newSpan.reduce_size(newSpanBytes.size());

            PowerSource::Events::BatChargeFaultChange::Type event_data{ oldSpan, newSpan };
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        SetAttributeValue(this->activeBatChargeFaultsBitSet, newBitSet, PowerSource::Attributes::ActiveBatChargeFaults::Id);
    }

    constexpr static System::Clock::Timeout kNotifyTimerDuration = System::Clock::Seconds16(10);
};

} // namespace chip::app::Clusters
