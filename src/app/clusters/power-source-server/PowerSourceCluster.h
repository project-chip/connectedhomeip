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

#include "Modules.h"

#include <algorithm>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>
#include <clusters/PowerSource/Events.h>
#include <clusters/PowerSource/Metadata.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/TimerDelegate.h>
#include <lib/support/TimerDelegateMock.h>

#include <limits>

namespace chip::app::Clusters {

namespace PowerSource::detail {

template<class T, class = std::enable_if_t<std::is_unsigned_v<T> && std::is_integral_v<T>, void>>
constexpr static T SpanToBitSet(Span<const uint8_t> span)
{
    T val{};
    for (uint8_t el : span)
    {
        val |= (1 << el);
    }
    return val;
}

template<class T, class = std::enable_if_t<std::is_unsigned_v<T> && std::is_integral_v<T>, void>>
constexpr static void BitSetToSpan(T bitset, Span<uint8_t> & buffer)
{
    size_t bufInd = 0;
    for (uint8_t i = 0; i < sizeof(T) * 8 && bufInd < buffer.size(); i++)
    {
        if (bitset & (1 << i))
        {
            buffer[bufInd++] = i;
        }
    }
    buffer.reduce_size(bufInd);
}

template<class To, class From, class = std::enable_if_t<sizeof(From) == sizeof(To), void>>
constexpr static Span<To> ConvertSpanType(Span<From> span)
{
    return Span(reinterpret_cast<To *>(span.data()), span.size());
}

template <typename T>
CHIP_ERROR EncodeOptional(AttributeValueEncoder & encoder, const std::optional<T> & value, CHIP_ERROR err = CHIP_NO_ERROR)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeNull();
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (!value.has_value())
    {
        return encoder.EncodeNull();
    }
    return encoder.Encode(*value);
}

template <typename T>
CHIP_ERROR EncodeOptional(AttributeValueEncoder & encoder, const Optional<T> & value, CHIP_ERROR err = CHIP_NO_ERROR)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeNull();
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (!value.HasValue())
    {
        return encoder.EncodeNull();
    }
    return encoder.Encode(value.Value());
}

template <typename T>
CHIP_ERROR EncodeListOfValues(AttributeValueEncoder & encoder, const T & valueList, CHIP_ERROR err = CHIP_NO_ERROR)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeEmptyList();
    }
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    return encoder.EncodeList([&valueList](const auto & enc) -> CHIP_ERROR {
        for (const auto & value : valueList)
        {
            if constexpr (std::is_enum_v<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>)
            {
                ReturnErrorOnFailure(enc.Encode(to_underlying(value)));
            }
            else
            {
                ReturnErrorOnFailure(enc.Encode(value));
            }
        }

        return CHIP_NO_ERROR;
    });
}

// all bits are 0 except mandatory attributes whose related features are in the `features`
constexpr static AttributeSet MandatoryAttributeSetFromFeatures(BitFlags<PowerSource::Feature> features)
{
    using namespace PowerSource::Attributes;
    constexpr uint32_t wiredMandatoryAttributeBits = OptionalAttributeSet<WiredCurrentType::Id>::All();

    constexpr uint32_t batteryMandatoryAttributeBits = OptionalAttributeSet<BatChargeLevel::Id, BatReplacementNeeded::Id, BatReplaceability::Id>::All();

    constexpr uint32_t replaceableBatteryMandatoryAttributeBits = OptionalAttributeSet<BatReplacementDescription::Id, BatQuantity::Id>::All();

    constexpr uint32_t rechargeableBatteryMandatoryAttributeBits = OptionalAttributeSet<BatChargeState::Id, BatFunctionalWhileCharging::Id>::All();

    uint32_t attributeBits = (wiredMandatoryAttributeBits * features.Has(PowerSource::Feature::kWired)) |
                                (batteryMandatoryAttributeBits * features.Has(PowerSource::Feature::kBattery)) |
                                (replaceableBatteryMandatoryAttributeBits * features.Has(PowerSource::Feature::kReplaceable)) |
                                (rechargeableBatteryMandatoryAttributeBits * features.Has(PowerSource::Feature::kRechargeable));
    return AttributeSet(attributeBits);
}

// all bits are 1 expect attributes related to features that are not in the `features`
// used as a mask, to disable all non-supported attributes
constexpr static AttributeSet DisabledAttributeSetFromFeatures(BitFlags<PowerSource::Feature> features)
{
    using namespace PowerSource::Attributes;
    constexpr uint32_t wiredAttributeBits = OptionalAttributeSet<
        WiredAssessedInputVoltage::Id, WiredAssessedInputFrequency::Id,
        WiredCurrentType::Id, WiredAssessedCurrent::Id,
        WiredNominalVoltage::Id, WiredMaximumCurrent::Id,
        WiredPresent::Id, ActiveWiredFaults::Id>::All();

    constexpr uint32_t batteryAttributeBits = OptionalAttributeSet<
        BatVoltage::Id, BatPercentRemaining::Id,
        BatTimeRemaining::Id, BatChargeLevel::Id,
        BatReplacementNeeded::Id, BatReplaceability::Id,
        BatPresent::Id, ActiveBatFaults::Id>::All();

    constexpr uint32_t replaceableBatteryAttributeBits = OptionalAttributeSet<
        BatReplacementDescription::Id, BatCommonDesignation::Id,
        BatANSIDesignation::Id, BatIECDesignation::Id,
        BatApprovedChemistry::Id, BatQuantity::Id>::All();

    constexpr uint32_t rechargeableBatteryAttributeBits = OptionalAttributeSet<
        BatChargeState::Id,
        BatTimeToFullCharge::Id, BatFunctionalWhileCharging::Id,
        BatChargingCurrent::Id, ActiveBatChargeFaults::Id>::All();

    constexpr uint32_t capacityAttributeBit = OptionalAttributeSet<BatCapacity::Id>::All();

    uint32_t attributeBits = ~(wiredAttributeBits * !features.Has(PowerSource::Feature::kWired) |
                                batteryAttributeBits * !features.Has(PowerSource::Feature::kBattery) |
                                replaceableBatteryAttributeBits * !features.Has(PowerSource::Feature::kReplaceable) |
                                rechargeableBatteryAttributeBits * !features.Has(PowerSource::Feature::kRechargeable) |
                                capacityAttributeBit * !(features.Has(PowerSource::Feature::kReplaceable) || features.Has(PowerSource::Feature::kRechargeable)));
    return AttributeSet(attributeBits);
}

constexpr static AttributeSet GetValidOptionalAttributeSet(AttributeSet optionalAttributeSet, BitFlags<PowerSource::Feature> features)
{
    uint32_t bits = (optionalAttributeSet.Raw()
        | MandatoryAttributeSetFromFeatures(features).Raw())
        & DisabledAttributeSetFromFeatures(features).Raw();
    return AttributeSet(bits);
}

} // namespace PowerSource::detail
using PowerSourceOptionalAttributeSet = app::OptionalAttributeSet<
    PowerSource::Attributes::WiredAssessedInputVoltage::Id,
    PowerSource::Attributes::WiredAssessedInputFrequency::Id,
    PowerSource::Attributes::WiredAssessedCurrent::Id,
    PowerSource::Attributes::WiredNominalVoltage::Id,
    PowerSource::Attributes::WiredMaximumCurrent::Id,
    PowerSource::Attributes::WiredPresent::Id,
    PowerSource::Attributes::ActiveWiredFaults::Id,
    PowerSource::Attributes::BatVoltage::Id,
    PowerSource::Attributes::BatPercentRemaining::Id,
    PowerSource::Attributes::BatTimeRemaining::Id,
    PowerSource::Attributes::BatPresent::Id,
    PowerSource::Attributes::ActiveBatFaults::Id,
    PowerSource::Attributes::BatCommonDesignation::Id,
    PowerSource::Attributes::BatANSIDesignation::Id,
    PowerSource::Attributes::BatIECDesignation::Id,
    PowerSource::Attributes::BatApprovedChemistry::Id,
    PowerSource::Attributes::BatCapacity::Id,
    PowerSource::Attributes::BatTimeToFullCharge::Id,
    PowerSource::Attributes::BatChargingCurrent::Id,
    PowerSource::Attributes::ActiveBatChargeFaults::Id
>;
template <std::underlying_type_t<PowerSource::Feature> supportedFeatureBits, uint32_t supportedOptionalAttributeBits>
struct PowerSourceClusterConfig : public PowerSource::detail::AllModulesExceptEndpointList<supportedFeatureBits, supportedOptionalAttributeBits>
{
    constexpr static BitFlags<PowerSource::Feature> supportedFeatures{ supportedFeatureBits };
    static_assert(supportedFeatures.Has(PowerSource::Feature::kWired) ^ supportedFeatures.Has(PowerSource::Feature::kBattery),
                  "Exactly one of Wired or Battery features must be set");

    static constexpr AttributeSet supportedOptionalAttributeSet = PowerSource::detail::GetValidOptionalAttributeSet(AttributeSet(supportedOptionalAttributeBits), supportedFeatures);


    PowerSourceClusterConfig(EndpointId endpointId, CharSpan desc, PowerSource::WiredCurrentTypeEnum currType)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kWired), "This constructor should only be used for a Wired power source configuration");
        mEndpointId = endpointId;
        this->description = desc;
        this->wiredCurrentType = currType;
    }

    PowerSourceClusterConfig(EndpointId endpointId, CharSpan desc, PowerSource::BatReplaceabilityEnum replability, TimerDelegate & timerDelegate)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kBattery),
                      "This constructor should only be used for a Battery power source configuration");
        mEndpointId = endpointId;
        this->description = desc;
        this->batReplaceability = replability;
        this->mTimerDelegate = &timerDelegate;
    }

    PowerSourceClusterConfig & MakeReplaceable(CharSpan replDesc, uint8_t quan)
    {
        static_assert(supportedFeatures.Has(PowerSource::Feature::kReplaceable),
                      "This method can only be used for a power source configuration that supports being replaceable");
        this->batReplacementDescription = replDesc;
        this->batQuantity = quan;

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

    /// if set true, `PowerSourceCluster::Startup` function will fetch the `Order` attribute's value from the persistent storage.
    /// if set false, the value will be the default value set in the `PowerSourceClusterConfig` (0 if not set explicitly).
    bool orderAttributeFetchFromPersistentStorageDuringStartup = true;
    PowerSourceOptionalAttributeSet usedOptionalAttributes { UINT32_MAX }; // all supported attributes are marked as used by default.
    EndpointId mEndpointId{};
};

template <std::underlying_type_t<PowerSource::Feature> supportedFeatureBits, uint32_t supportedOptionalAttributeBits>
class PowerSourceCluster : protected PowerSourceClusterConfig<supportedFeatureBits, supportedOptionalAttributeBits>,
                           protected PowerSource::detail::BatteryTimerContextsModule<BitFlags<PowerSource::Feature>(supportedFeatureBits).Has(PowerSource::Feature::kBattery)>,
                           protected PowerSource::detail::EndpointListModule,
                           public DefaultServerCluster
{
public:
    using ConfigType = PowerSourceClusterConfig<supportedFeatureBits, supportedOptionalAttributeBits>;
    using ConfigType::supportedFeatures;
    using ConfigType::supportedOptionalAttributeSet;

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

    PowerSourceCluster(const ConfigType & config) :
        ConfigType(config),
        DefaultServerCluster({ config.mEndpointId, PowerSource::Id })
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

        // the `Order` attribute is marked as `Persistent`.
        VerifyOrReturnValue(this->orderAttributeFetchFromPersistentStorageDuringStartup, CHIP_NO_ERROR);

        AttributePersistence attributePersistence(context.attributeStorage);
        attributePersistence.LoadNativeEndianValue<uint8_t>({ mPath.mEndpointId, mPath.mClusterId, PowerSource::Attributes::Order::Id }, this->order, this->order);

        // if getting the value from persistent storage fails, continue with our lives.
        return CHIP_NO_ERROR;
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        using namespace PowerSource::Attributes;
        using namespace PowerSource::detail;
        // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is valid.
        AttributeId id = request.path.mAttributeId;
        if (id == Status::Id) { return encoder.Encode(this->status); }
        if (id == Order::Id) { return encoder.Encode(this->order); }
        if (id == Description::Id) { return encoder.Encode(this->description.SubSpan(0, std::min(this->description.size(), Description::TypeInfo::MaxLength()))); }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedInputVoltage::Id))
        {
            if (id == WiredAssessedInputVoltage::Id) { return EncodeOptional(encoder, this->wiredAssessedInputVoltage); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedInputFrequency::Id))
        {
            if (id == WiredAssessedInputFrequency::Id) { return EncodeOptional(encoder, this->wiredAssessedInputFrequency); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredCurrentType::Id))
        {
            if (id == WiredCurrentType::Id) { return encoder.Encode(this->wiredCurrentType); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredAssessedCurrent::Id))
        {
            if (id == WiredAssessedCurrent::Id) { return EncodeOptional(encoder, this->wiredAssessedCurrent); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredNominalVoltage::Id))
        {
            if (id == WiredNominalVoltage::Id) { return encoder.Encode(this->wiredNominalVoltage); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredMaximumCurrent::Id))
        {
            if (id == WiredMaximumCurrent::Id) { return encoder.Encode(this->wiredMaximumCurrent); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(WiredPresent::Id))
        {
            if (id == WiredPresent::Id) { return encoder.Encode(this->wiredPresent); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(ActiveWiredFaults::Id))
        {
            if (id == ActiveWiredFaults::Id)
            {
                uint8_t faultsBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeWiredFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatVoltage::Id))
        {
            if (id == BatVoltage::Id) { return EncodeOptional(encoder, this->batVoltage); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatPercentRemaining::Id))
        {
            if (id == BatPercentRemaining::Id) { return EncodeOptional(encoder, this->batPercentRemaining); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatTimeRemaining::Id))
        {
            if (id == BatTimeRemaining::Id) { return EncodeOptional(encoder, this->batTimeRemaining); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatChargeLevel::Id))
        {
            if (id == BatChargeLevel::Id) { return encoder.Encode(this->batChargeLevel); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatReplacementNeeded::Id))
        {
            if (id == BatReplacementNeeded::Id) { return encoder.Encode(this->batReplacementNeeded); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatReplaceability::Id))
        {
            if (id == BatReplaceability::Id) { return encoder.Encode(this->batReplaceability); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatPresent::Id))
        {
            if (id == BatPresent::Id) { return encoder.Encode(this->batPresent); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(ActiveBatFaults::Id))
        {
            if (id == ActiveBatFaults::Id)
            {
                uint8_t faultsBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeBatFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatReplacementDescription::Id))
        {
            if (id == BatReplacementDescription::Id) { return encoder.Encode(this->batReplacementDescription.SubSpan(0, std::min(this->batReplacementDescription.size(), BatReplacementDescription::TypeInfo::MaxLength()))); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatCommonDesignation::Id))
        {
            if (id == BatCommonDesignation::Id) { return encoder.Encode(this->batCommonDesignation); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatANSIDesignation::Id))
        {
            if (id == BatANSIDesignation::Id) { return encoder.Encode(this->batANSIDesignation.SubSpan(0, std::min(this->batANSIDesignation.size(), BatANSIDesignation::TypeInfo::MaxLength()))); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatIECDesignation::Id))
        {
            if (id == BatIECDesignation::Id) { return encoder.Encode(this->batIECDesignation.SubSpan(0, std::min(this->batIECDesignation.size(), BatIECDesignation::TypeInfo::MaxLength()))); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatApprovedChemistry::Id))
        {
            if (id == BatApprovedChemistry::Id) { return encoder.Encode(this->batApprovedChemistry); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatCapacity::Id))
        {
            if (id == BatCapacity::Id) { return encoder.Encode(this->batCapacity); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatQuantity::Id))
        {
            if (id == BatQuantity::Id) { return encoder.Encode(this->batQuantity); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatChargeState::Id))
        {
            if (id == BatChargeState::Id) { return encoder.Encode(this->batChargeState); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatTimeToFullCharge::Id))
        {
            if (id == BatTimeToFullCharge::Id) { return EncodeOptional(encoder, this->batTimeToFullCharge); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatFunctionalWhileCharging::Id))
        {
            if (id == BatFunctionalWhileCharging::Id) { return encoder.Encode(this->batFunctionalWhileCharging); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(BatChargingCurrent::Id))
        {
            if (id == BatChargingCurrent::Id) { return EncodeOptional(encoder, this->batChargingCurrent); }
        }
        if constexpr (supportedOptionalAttributeSet.IsSet(ActiveBatChargeFaults::Id))
        {
            if (id == ActiveBatChargeFaults::Id)
            {
                uint8_t faultsBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)];
                auto faultsSpan = Span(faultsBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
                BitSetToSpan(this->activeBatChargeFaultsBitSet, faultsSpan);
                return EncodeListOfValues(encoder, faultsSpan);
            }
        }
        if (id == EndpointList::Id) { return EncodeListOfValues(encoder, GetEndpointList()); }
        if (id == Globals::Attributes::FeatureMap::Id) { return encoder.Encode(Features()); }
        if (id == Globals::Attributes::ClusterRevision::Id) { return encoder.Encode(PowerSource::kRevision); }

        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        using namespace PowerSource::Attributes;
        AttributeListBuilder attributeListBuilder(builder);

        AttributeSet optionalAttributeSet(PowerSource::detail::GetValidOptionalAttributeSet(this->usedOptionalAttributes, Features()).Raw()
                                                                         & supportedOptionalAttributeSet.Raw());

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

#define ENABLE_IF_ATTRIBUTE_SUPPORTED(attrName) \
    static_assert(supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id), "This method can only be used if the " #attrName " attribute is supported"); \
    if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id)) \
    { \
        return {}; \
    } \
    else

#define ENABLE_IF_ATTRIBUTE_SUPPORTED_NO_RETURN(attrName) \
    static_assert(supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id), "This method can only be used if the " #attrName " attribute is supported"); \
    if constexpr (supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::attrName::Id))

    // Getters

    PowerSourceStatusEnum GetStatus() const
    {
        return this->status;
    }
    uint8_t GetOrder() const
    {
        return this->order;
    }
    CharSpan GetDescription() const
    {
        return this->description;
    }
    std::optional<uint32_t> GetWiredAssessedInputVoltage() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedInputVoltage)
        {
            return this->wiredAssessedInputVoltage;
        }
    }
    std::optional<uint16_t> GetWiredAssessedInputFrequency() const
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
    std::optional<uint32_t> GetWiredAssessedCurrent() const
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
    std::optional<uint32_t> GetBatVoltage() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatVoltage)
        {
            return this->batVoltage;
        }
    }
    std::optional<uint8_t> GetBatPercentRemaining() const
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPercentRemaining)
        {
            return this->batPercentRemaining;
        }
    }
    std::optional<uint32_t> GetBatTimeRemaining() const
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
    std::optional<uint32_t> GetBatTimeToFullCharge() const
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
    std::optional<uint32_t> GetBatChargingCurrent() const
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
    Span<const EndpointId> GetEndpointList() const
    {
        return Span(this->endpointList.Get(), this->endpointListCount);
    }

    // Setters

    /// Attributes marked with the `Fixed` quality do not have setters.
    /// They can be only set during construction using the `WiredPowerSourceCluster::Configuration` class.
    /// `Fixed` attributes are `Description`, `WiredCurrentType`, `WiredNominalVoltage`, `WiredMaximumCurrent`

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val)
    {
        SetAttributeValue(this->status, val, PowerSource::Attributes::Status::Id);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetOrder(uint8_t val)
    {
        using namespace PowerSource::Attributes;
        // This attribute is marked as `Persistent`.
        if (mContext != nullptr && val != this->order)
        {
            AttributePersistence attributePersistence(mContext->attributeStorage);
            attributePersistence.StoreNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, val);
        }

        SetAttributeValue(this->order, val, Order::Id);

        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetWiredAssessedInputVoltage(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedInputVoltage)
        {
            this->wiredAssessedInputVoltage = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetWiredAssessedInputFrequency(std::optional<uint16_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedInputFrequency)
        {
            this->wiredAssessedInputFrequency = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetWiredAssessedCurrent(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredAssessedCurrent)
        {
            this->wiredAssessedCurrent = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetWiredPresent(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(WiredPresent)
        {
            SetAttributeValue(this->wiredPresent, val, PowerSource::Attributes::WiredPresent::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetActiveWiredFaults(Span<const WiredFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveWiredFaults)
        {
            uint8_t bitset = PowerSource::detail::SpanToBitSet<uint8_t>(PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeWiredFaultsBitSet == bitset)
            {
                return CHIP_NO_ERROR;
            }

            GenerateWiredFaultEventAndSetAndNotify(bitset);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR AddActiveWiredFault(WiredFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveWiredFaults)
        {
            uint8_t newBitSet = this->activeWiredFaultsBitSet | static_cast<uint8_t>(1 << to_underlying(val));
            if (this->activeWiredFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateWiredFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveWiredFault(WiredFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveWiredFaults)
        {
            uint8_t newBitSet = this->activeWiredFaultsBitSet & static_cast<uint8_t>(~(1 << to_underlying(val)));
            if (this->activeWiredFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateWiredFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatVoltage(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatVoltage)
        {
            this->batVoltage = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatPercentRemaining(std::optional<uint8_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPercentRemaining)
        {
            if (val.has_value())
            {
                // Maximum value of 200 representing 100% battery.
                VerifyOrReturnError(val.value() <= 200, CHIP_ERROR_INVALID_INTEGER_VALUE);
            }

            if (this->batPercentRemaining == val)
            {
                return CHIP_NO_ERROR;
            }

            if (!this->batPercentRemaining.has_value() || !val.has_value())
            {
                SetAttributeValue(this->batPercentRemaining, val, PowerSource::Attributes::BatPercentRemaining::Id);
                return CHIP_NO_ERROR;
            }

            if (this->batPercentRemainingNotifyTimerContext.timerExpired)
            {
                // there should be no normal way to get to this point without setting the timer delegate
                VerifyOrDie(this->mTimerDelegate != nullptr);
                SetAttributeValue(this->batPercentRemaining, val, PowerSource::Attributes::BatPercentRemaining::Id);
                ReturnErrorOnFailure(this->mTimerDelegate->StartTimer(&this->batPercentRemainingNotifyTimerContext, kNotifyTimerDuration));
                this->batPercentRemainingNotifyTimerContext.timerExpired = false;
                return CHIP_NO_ERROR;
            }

            this->batPercentRemaining = val;
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatTimeRemaining(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeRemaining)
        {
            if (this->batTimeRemaining == val)
            {
                return CHIP_NO_ERROR;
            }

            if (!this->batTimeRemaining.has_value() || !val.has_value())
            {
                SetAttributeValue(this->batTimeRemaining, val, PowerSource::Attributes::BatTimeRemaining::Id);
                return CHIP_NO_ERROR;
            }

            if (this->batTimeRemainingNotifyTimerContext.timerExpired)
            {
                // there should be no normal way to get to this point without setting the timer delegate
                VerifyOrDie(this->mTimerDelegate != nullptr);
                SetAttributeValue(this->batTimeRemaining, val, PowerSource::Attributes::BatTimeRemaining::Id);
                ReturnErrorOnFailure(this->mTimerDelegate->StartTimer(&this->batTimeRemainingNotifyTimerContext, kNotifyTimerDuration));
                this->batTimeRemainingNotifyTimerContext.timerExpired = false;
                return CHIP_NO_ERROR;
            }

            this->batTimeRemaining = val;
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeLevel)
        {
            SetAttributeValue(this->batChargeLevel, val, PowerSource::Attributes::BatChargeLevel::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatReplacementNeeded(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatReplacementNeeded)
        {
            SetAttributeValue(this->batReplacementNeeded, val, PowerSource::Attributes::BatReplacementNeeded::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatPresent(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatPresent)
        {
            SetAttributeValue(this->batPresent, val, PowerSource::Attributes::BatPresent::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetActiveBatFaults(Span<const BatFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatFaults)
        {
            uint8_t newBitSet = PowerSource::detail::SpanToBitSet<uint8_t>(PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeBatFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR AddActiveBatFault(BatFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatFaults)
        {
            uint8_t newBitSet = this->activeBatFaultsBitSet | static_cast<uint8_t>(1 << to_underlying(val));
            if (this->activeBatFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveBatFault(BatFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatFaults)
        {
            uint8_t newBitSet = this->activeBatFaultsBitSet & static_cast<uint8_t>(~(1 << to_underlying(val)));
            if (this->activeBatFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatChargeState(BatChargeStateEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargeState)
        {
            SetAttributeValue(this->batChargeState, val, PowerSource::Attributes::BatChargeState::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatTimeToFullCharge(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatTimeToFullCharge)
        {
            if (this->batTimeToFullCharge == val)
            {
                return CHIP_NO_ERROR;
            }

            if (!this->batTimeToFullCharge.has_value() || !val.has_value())
            {
                SetAttributeValue(this->batTimeToFullCharge, val, PowerSource::Attributes::BatTimeToFullCharge::Id);
                return CHIP_NO_ERROR;
            }

            if (this->batTimeToFullChargeNotifyTimerContext.timerExpired)
            {
                // there should be no normal way to get to this point without setting the timer delegate
                VerifyOrDie(this->mTimerDelegate != nullptr);
                SetAttributeValue(this->batTimeToFullCharge, val, PowerSource::Attributes::BatTimeToFullCharge::Id);
                ReturnErrorOnFailure(this->mTimerDelegate->StartTimer(&this->batTimeToFullChargeNotifyTimerContext, kNotifyTimerDuration));
                this->batTimeToFullChargeNotifyTimerContext.timerExpired = false;
                return CHIP_NO_ERROR;
            }

            this->batTimeToFullCharge = val;
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatFunctionalWhileCharging(bool val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatFunctionalWhileCharging)
        {
            SetAttributeValue(this->batFunctionalWhileCharging, val, PowerSource::Attributes::BatFunctionalWhileCharging::Id);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetBatChargingCurrent(std::optional<uint32_t> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(BatChargingCurrent)
        {
            this->batChargingCurrent = val;
            // no notifying because attribute marked with 'Changes Omitted' quality
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatChargeFaults)
        {
            uint16_t newBitSet = PowerSource::detail::SpanToBitSet<uint16_t>(PowerSource::detail::ConvertSpanType<const uint8_t>(val));
            if (this->activeBatChargeFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR AddActiveBatChargeFault(BatChargeFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatChargeFaults)
        {
            uint16_t newBitSet = this->activeBatChargeFaultsBitSet | static_cast<uint16_t>(1 << to_underlying(val));
            if (this->activeBatChargeFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR RemoveActiveBatChargeFault(BatChargeFaultEnum val)
    {
        ENABLE_IF_ATTRIBUTE_SUPPORTED(ActiveBatChargeFaults)
        {
            uint16_t newBitSet = this->activeBatChargeFaultsBitSet & static_cast<uint16_t>(~(1 << to_underlying(val)));
            if (this->activeBatChargeFaultsBitSet == newBitSet)
            {
                return CHIP_NO_ERROR;
            }

            GenerateBatChargeFaultEventAndSetAndNotify(newBitSet);
            return CHIP_NO_ERROR;
        }
    }
    CHIP_ERROR SetEndpointList(Span<const EndpointId> val)
    {
        if (GetEndpointList().data_equal(val))
        {
            return CHIP_NO_ERROR;
        }

        if (!this->endpointList.Alloc(val.size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        std::copy(val.begin(), val.end(), this->endpointList.Get());
        this->endpointListCount = static_cast<uint16_t>(val.size());
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

    void GenerateWiredFaultEventAndSetAndNotify(uint8_t newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveWiredFaults::Id))
        {
            return;
        }

        uint8_t oldBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)];
        uint8_t newBuf[to_underlying(WiredFaultEnum::kUnknownEnumValue)];

        auto oldSpanNonConverted = Span(oldBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(this->activeWiredFaultsBitSet, oldSpanNonConverted);
        auto oldSpan             = PowerSource::detail::ConvertSpanType<WiredFaultEnum>(oldSpanNonConverted);
        auto newSpanNonConverted = Span(newBuf, to_underlying(WiredFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(newBitSet, newSpanNonConverted);
        auto newSpan             = PowerSource::detail::ConvertSpanType<WiredFaultEnum>(newSpanNonConverted);

        PowerSource::Events::WiredFaultChange::Type event_data{ oldSpan, newSpan };
        if(mContext != nullptr)
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        this->activeWiredFaultsBitSet = newBitSet;
        NotifyAttributeChanged(PowerSource::Attributes::ActiveWiredFaults::Id);
    }

    void GenerateBatFaultEventAndSetAndNotify(uint8_t newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveBatFaults::Id))
        {
            return;
        }

        uint8_t oldBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)];
        uint8_t newBuf[to_underlying(BatFaultEnum::kUnknownEnumValue)];

        auto oldSpanNonConverted = Span(oldBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(this->activeBatFaultsBitSet, oldSpanNonConverted);
        auto oldSpan             = PowerSource::detail::ConvertSpanType<BatFaultEnum>(oldSpanNonConverted);
        auto newSpanNonConverted = Span(newBuf, to_underlying(BatFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(newBitSet, newSpanNonConverted);
        auto newSpan             = PowerSource::detail::ConvertSpanType<BatFaultEnum>(newSpanNonConverted);

        PowerSource::Events::BatFaultChange::Type event_data{ oldSpan, newSpan };
        if(mContext != nullptr)
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        this->activeBatFaultsBitSet = newBitSet;
        NotifyAttributeChanged(PowerSource::Attributes::ActiveBatFaults::Id);
    }

    void GenerateBatChargeFaultEventAndSetAndNotify(uint16_t newBitSet)
    {
        if constexpr (!supportedOptionalAttributeSet.IsSet(PowerSource::Attributes::ActiveBatChargeFaults::Id))
        {
            return;
        }

        uint8_t oldBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)];
        uint8_t newBuf[to_underlying(BatChargeFaultEnum::kUnknownEnumValue)];

        auto oldSpanNonConverted = Span(oldBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(this->activeBatChargeFaultsBitSet, oldSpanNonConverted);
        auto oldSpan             = PowerSource::detail::ConvertSpanType<BatChargeFaultEnum>(oldSpanNonConverted);
        auto newSpanNonConverted = Span(newBuf, to_underlying(BatChargeFaultEnum::kUnknownEnumValue));
        PowerSource::detail::BitSetToSpan(newBitSet, newSpanNonConverted);
        auto newSpan             = PowerSource::detail::ConvertSpanType<BatChargeFaultEnum>(newSpanNonConverted);

        PowerSource::Events::BatChargeFaultChange::Type event_data{ oldSpan, newSpan };
        if(mContext != nullptr)
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, mPath.mEndpointId);
        }

        this->activeBatChargeFaultsBitSet = newBitSet;
        NotifyAttributeChanged(PowerSource::Attributes::ActiveBatChargeFaults::Id);
    }

    static constexpr System::Clock::Timeout kNotifyTimerDuration = System::Clock::Seconds16(10);
};

using FullWiredPowerSourceConfig = PowerSourceClusterConfig<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), UINT32_MAX>;
using FullWiredPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), UINT32_MAX>;
using FullBatteryPowerSourceConfig = PowerSourceClusterConfig<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery, PowerSource::Feature::kReplaceable, PowerSource::Feature::kRechargeable).Raw(), UINT32_MAX>;
using FullBatteryPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery, PowerSource::Feature::kReplaceable, PowerSource::Feature::kRechargeable).Raw(), UINT32_MAX>;

using MinimalWiredPowerSourceConfig = PowerSourceClusterConfig<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), 0>;
using MinimalWiredPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), 0>;
using MinimalBatteryPowerSourceConfig = PowerSourceClusterConfig<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery).Raw(), 0>;
using MinimalBatteryPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery).Raw(), 0>;


} // namespace chip::app::Clusters
