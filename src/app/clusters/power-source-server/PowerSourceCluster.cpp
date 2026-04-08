/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PowerSourceCluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PowerSource/Metadata.h>

using namespace chip;
using namespace app;
using namespace app::Clusters;

namespace {
template <typename T>
CHIP_ERROR EncodeValue(AttributeValueEncoder & encoder, const T & value, CHIP_ERROR err)
{
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return encoder.EncodeNull();
    }
    if (err != CHIP_NO_ERROR)
    {
        return encoder.EncodeNull();
    }
    return encoder.Encode(value);
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
            ReturnErrorOnFailure(enc.Encode(value));
        }

        return CHIP_NO_ERROR;
    });
}
} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {

using namespace PowerSource;
using namespace PowerSource::Attributes;
using namespace PowerSource::Events;

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, System::Layer & systemLayer, const WiredConfiguration & config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }), mFeatures(WiredFeatures()), mSystemLayer(systemLayer)
{
    mAttributes.wired = WiredAttributes{};

    mAttributes.status = config.status;
    mAttributes.order  = config.order;

    SetStringWithoutNotifying(config.description, mAttributes.mDescriptionBuffer, sizeof(mAttributes.mDescriptionBuffer) - 1);
    mAttributes.wired.currentType   = config.currentType;
    mAttributes.wired.nominalVoltage = config.nominalVoltage;
    mAttributes.wired.maximumCurrent = config.maximumCurrent;

    mAttributes.wired.assessedInputVoltage   = config.assessedInputVoltage;
    mAttributes.wired.assessedInputFrequency = config.assessedInputFrequency;
    mAttributes.wired.assessedCurrent        = config.assessedCurrent;
    mAttributes.wired.isPresent              = config.isPresent;
}

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, System::Layer & systemLayer, const BatteryConfiguration & config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }),
    mFeatures(BatteryFeatures(config.isReplaceable(), config.isRechargeable())),
    mSystemLayer(systemLayer)
{
    mAttributes.battery = BatteryAttributes{};

    mAttributes.status = config.status;
    mAttributes.order  = config.order;
    SetStringWithoutNotifying(config.description, mAttributes.mDescriptionBuffer, sizeof(mAttributes.mDescriptionBuffer) - 1);

    mAttributes.battery.voltage            = config.voltage;
    if (config.percentRemaining.hasValue)
    {
        // the upper bound of percentRemaining is 200, which corresponds to 100% battery.
        mAttributes.battery.percentRemaining = MakeOptional<uint8_t>(config.percentRemaining.value < 200 ? config.percentRemaining.value : 200);
    }
    else
    {
        mAttributes.battery.percentRemaining = NullOptional;
    }
    mAttributes.battery.timeRemaining      = config.timeRemaining;
    mAttributes.battery.chargeLevel        = config.chargeLevel;
    mAttributes.battery.replacementNeeded = config.replacementNeeded;
    mAttributes.battery.replaceability = config.replaceability;
    mAttributes.battery.isPresent          = config.isPresent;

    if (mFeatures.Has(Feature::kReplaceable))
    {
        SetStringWithoutNotifying(config.replacementDescription, mAttributes.battery.replaceable.mReplacementDescriptionBuffer,
                        sizeof(mAttributes.battery.replaceable.mReplacementDescriptionBuffer) - 1);
        mAttributes.battery.replaceable.commonDesignation = config.commonDesignation;
        SetStringWithoutNotifying(config.ansiDesignation, mAttributes.battery.replaceable.mANSIDesignationBuffer,
                        sizeof(mAttributes.battery.replaceable.mANSIDesignationBuffer) - 1);
        SetStringWithoutNotifying(config.iecDesignation, mAttributes.battery.replaceable.mIECDesignationBuffer,
                        sizeof(mAttributes.battery.replaceable.mIECDesignationBuffer) - 1);
        mAttributes.battery.replaceable.approvedChemistry = config.approvedChemistry;
        mAttributes.battery.replaceable.quantity         = config.quantity;
    }

    if (mFeatures.Has(Feature::kRechargeable))
    {
        mAttributes.battery.rechargeable.chargeState          = config.chargeState;
        mAttributes.battery.rechargeable.timeToFullCharge     = config.timeToFullCharge;
        mAttributes.battery.rechargeable.functionalWhileCharging = config.functionalWhileCharging;
        mAttributes.battery.rechargeable.chargingCurrent      = config.chargingCurrent;
    }

    if (mFeatures.HasAny(Feature::kReplaceable, Feature::kRechargeable))
    {
        mAttributes.battery.capacity = config.capacity;
    }
}

CHIP_ERROR PowerSourceCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // the `Order` attribute is marked as `Persistent`.
    uint8_t order;

    AttributePersistence attributePersistence(context.attributeStorage);
    if (attributePersistence.LoadNativeEndianValue<uint8_t>({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, order, 0))
    {
        ReturnErrorOnFailure(SetOrder(order));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus PowerSourceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                AttributeValueEncoder & encoder)
{
    using namespace PowerSource::Attributes;
    // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is valid.
    switch (request.path.mAttributeId)
    {
    case Status::Id:
        return encoder.Encode(mAttributes.status);
    case Order::Id:
        return encoder.Encode(mAttributes.order);
    case Description::Id:
        return encoder.Encode(mAttributes.GetDescription());
    case WiredAssessedInputVoltage::Id:
        return EncodeOptional(encoder, mAttributes.wired.assessedInputVoltage);
    case WiredAssessedInputFrequency::Id:
        return EncodeOptional(encoder, mAttributes.wired.assessedInputFrequency);
    case WiredCurrentType::Id:
        return encoder.Encode(mAttributes.wired.currentType);
    case WiredAssessedCurrent::Id:
        return EncodeOptional(encoder, mAttributes.wired.assessedCurrent);
    case WiredNominalVoltage::Id:
        return encoder.Encode(mAttributes.wired.nominalVoltage);
    case WiredMaximumCurrent::Id:
        return encoder.Encode(mAttributes.wired.maximumCurrent);
    case WiredPresent::Id:
        return encoder.Encode(mAttributes.wired.isPresent);
    case ActiveWiredFaults::Id:
        return EncodeListOfValues(encoder, BitSetToSpan(mAttributes.wired.activeFaults, mAttributes.wired.mActiveFaultsBuf));
    case BatVoltage::Id:
        return EncodeOptional(encoder, mAttributes.battery.voltage);
    case BatPercentRemaining::Id:
        return EncodeOptional(encoder, mAttributes.battery.percentRemaining);
    case BatTimeRemaining::Id:
        return EncodeOptional(encoder, mAttributes.battery.timeRemaining);
    case BatChargeLevel::Id:
        return encoder.Encode(mAttributes.battery.chargeLevel);
    case BatReplacementNeeded::Id:
        return encoder.Encode(mAttributes.battery.replacementNeeded);
    case BatReplaceability::Id:
        return encoder.Encode(mAttributes.battery.replaceability);
    case BatPresent::Id:
        return encoder.Encode(mAttributes.battery.isPresent);
    case ActiveBatFaults::Id:
        return EncodeListOfValues(encoder, BitSetToSpan(mAttributes.battery.activeFaults, mAttributes.battery.mActiveFaultsBuf));
    case BatReplacementDescription::Id:
        return encoder.Encode(mAttributes.battery.replaceable.GetReplacementDescription());
    case BatCommonDesignation::Id:
        return encoder.Encode(mAttributes.battery.replaceable.commonDesignation);
    case BatANSIDesignation::Id:
        return encoder.Encode(mAttributes.battery.replaceable.GetANSIDesignation());
    case BatIECDesignation::Id:
        return encoder.Encode(mAttributes.battery.replaceable.GetIECDesignation());
    case BatApprovedChemistry::Id:
        return encoder.Encode(mAttributes.battery.replaceable.approvedChemistry);
    case BatCapacity::Id:
        return encoder.Encode(mAttributes.battery.capacity);
    case BatQuantity::Id:
        return encoder.Encode(mAttributes.battery.replaceable.quantity);
    case BatChargeState::Id:
        return encoder.Encode(mAttributes.battery.rechargeable.chargeState);
    case BatTimeToFullCharge::Id:
        return EncodeOptional(encoder, mAttributes.battery.rechargeable.timeToFullCharge);
    case BatFunctionalWhileCharging::Id:
        return encoder.Encode(mAttributes.battery.rechargeable.functionalWhileCharging);
    case BatChargingCurrent::Id:
        return EncodeOptional(encoder, mAttributes.battery.rechargeable.chargingCurrent);
    case ActiveBatChargeFaults::Id:
        return EncodeListOfValues(
            encoder,
            BitSetToSpan(mAttributes.battery.rechargeable.activeFaults, mAttributes.battery.rechargeable.mActiveChargeFaultsBuf));
    case EndpointList::Id:
        return EncodeListOfValues(encoder, mAttributes.GetPoweredEndpoints());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(Features());
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR PowerSourceCluster::Attributes(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // Workaround to save flash.
    constexpr uint32_t wiredAttributeBits =
        OptionalAttributeSet<WiredAssessedInputVoltage::Id, WiredAssessedInputFrequency::Id, WiredCurrentType::Id,
                             WiredAssessedCurrent::Id, WiredNominalVoltage::Id, WiredMaximumCurrent::Id, WiredPresent::Id,
                             ActiveWiredFaults::Id>::All();

    constexpr uint32_t batteryAttributeBits =
        OptionalAttributeSet<BatVoltage::Id, BatPercentRemaining::Id, BatTimeRemaining::Id, BatChargeLevel::Id,
                             BatReplacementNeeded::Id, BatReplaceability::Id, BatPresent::Id, ActiveBatFaults::Id>::All();

    constexpr uint32_t replaceableBatteryAttributeBits =
        OptionalAttributeSet<BatReplacementDescription::Id, BatCommonDesignation::Id, BatANSIDesignation::Id, BatIECDesignation::Id,
                             BatApprovedChemistry::Id, BatQuantity::Id>::All();

    constexpr uint32_t rechargeableBatteryAttributeBits =
        OptionalAttributeSet<BatChargeState::Id, BatTimeToFullCharge::Id, BatFunctionalWhileCharging::Id, BatChargingCurrent::Id,
                             ActiveBatChargeFaults::Id>::All();

    constexpr uint32_t capacityAttributeBit = OptionalAttributeSet<BatCapacity::Id>::All();

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

    AttributeSet optAttributeSet(Features().Has(Feature::kWired) * wiredAttributeBits |
                                 Features().Has(Feature::kBattery) * batteryAttributeBits |
                                 Features().Has(Feature::kReplaceable) * replaceableBatteryAttributeBits |
                                 Features().Has(Feature::kRechargeable) * rechargeableBatteryAttributeBits |
                                 Features().HasAny(Feature::kReplaceable, Feature::kRechargeable) * capacityAttributeBit);

    AttributeListBuilder attributeListBuilder(builder);

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(kOptionalAttributes), optAttributeSet);
}

// Getter implementations

#define ValidateFeature(feature_name, attr_name)                                                                                   \
    VerifyOrDieWithMsg(Features().Has(Feature::feature_name), Zcl,                                                                 \
                       "Attempting to read attribute `" #attr_name "` when feature `" #feature_name "` is not set.");

PowerSourceCluster::PowerSourceStatusEnum PowerSourceCluster::GetStatus() const
{
    return mAttributes.status;
}

uint8_t PowerSourceCluster::GetOrder() const
{
    return mAttributes.order;
}

CharSpan PowerSourceCluster::GetDescription() const
{
    return mAttributes.GetDescription();
}

Optional<uint32_t> PowerSourceCluster::GetWiredAssessedInputVoltage() const
{
    // here and down, where the attribute is optional, the feature validation is done only for logging purposes,
    // because the optional attribute validation is fully enough for code correctness
    ValidateFeature(kWired, WiredAssessedInputVoltage);

    return mAttributes.wired.assessedInputVoltage;
}

Optional<uint16_t> PowerSourceCluster::GetWiredAssessedInputFrequency() const
{
    ValidateFeature(kWired, WiredAssessedInputFrequency);

    return mAttributes.wired.assessedInputFrequency;
}

PowerSourceCluster::WiredCurrentTypeEnum PowerSourceCluster::GetWiredCurrentType() const
{
    ValidateFeature(kWired, WiredCurrentType);

    return mAttributes.wired.currentType;
}

Optional<uint32_t> PowerSourceCluster::GetWiredAssessedCurrent() const
{
    ValidateFeature(kWired, WiredAssessedCurrent);

    return mAttributes.wired.assessedCurrent;
}

uint32_t PowerSourceCluster::GetWiredNominalVoltage() const
{
    ValidateFeature(kWired, WiredNominalVoltage);

    return mAttributes.wired.nominalVoltage;
}

uint32_t PowerSourceCluster::GetWiredMaximumCurrent() const
{
    ValidateFeature(kWired, WiredMaximumCurrent);

    return mAttributes.wired.maximumCurrent;
}

bool PowerSourceCluster::GetWiredPresent() const
{
    ValidateFeature(kWired, WiredPresent);

    return mAttributes.wired.isPresent;
}

Span<const PowerSourceCluster::WiredFaultEnum> PowerSourceCluster::GetActiveWiredFaults() const
{
    ValidateFeature(kWired, ActiveWiredFaults);

    return BitSetToSpan(mAttributes.wired.activeFaults, mAttributes.wired.mActiveFaultsBuf);
}

Optional<uint32_t> PowerSourceCluster::GetBatVoltage() const
{
    ValidateFeature(kBattery, BatVoltage);

    return mAttributes.battery.voltage;
}

Optional<uint8_t> PowerSourceCluster::GetBatPercentRemaining() const
{
    ValidateFeature(kBattery, BatPercentRemaining);

    return mAttributes.battery.percentRemaining;
}

Optional<uint32_t> PowerSourceCluster::GetBatTimeRemaining() const
{
    ValidateFeature(kBattery, BatTimeRemaining);

    return mAttributes.battery.timeRemaining;
}

PowerSourceCluster::BatChargeLevelEnum PowerSourceCluster::GetBatChargeLevel() const
{
    ValidateFeature(kBattery, BatChargeLevel);

    return mAttributes.battery.chargeLevel;
}

bool PowerSourceCluster::GetBatReplacementNeeded() const
{
    ValidateFeature(kBattery, BatReplacementNeeded);

    return mAttributes.battery.replacementNeeded;
}

PowerSourceCluster::BatReplaceabilityEnum PowerSourceCluster::GetBatReplaceability() const
{
    ValidateFeature(kBattery, BatReplaceability);

    return mAttributes.battery.replaceability;
}

bool PowerSourceCluster::GetBatPresent() const
{
    ValidateFeature(kBattery, BatPresent);

    return mAttributes.battery.isPresent;
}

Span<const PowerSourceCluster::BatFaultEnum> PowerSourceCluster::GetActiveBatFaults() const
{
    ValidateFeature(kBattery, ActiveBatFaults);

    return BitSetToSpan(mAttributes.battery.activeFaults, mAttributes.battery.mActiveFaultsBuf);
}

CharSpan PowerSourceCluster::GetBatReplacementDescription() const
{
    ValidateFeature(kBattery, BatReplacementDescription);
    ValidateFeature(kReplaceable, BatReplacementDescription);

    return mAttributes.battery.replaceable.GetReplacementDescription();
}

PowerSourceCluster::BatCommonDesignationEnum PowerSourceCluster::GetBatCommonDesignation() const
{
    ValidateFeature(kBattery, BatCommonDesignation);
    ValidateFeature(kReplaceable, BatCommonDesignation);

    return mAttributes.battery.replaceable.commonDesignation;
}

CharSpan PowerSourceCluster::GetBatANSIDesignation() const
{
    ValidateFeature(kBattery, BatANSIDesignation);
    ValidateFeature(kReplaceable, BatANSIDesignation);

    return mAttributes.battery.replaceable.GetANSIDesignation();
}

CharSpan PowerSourceCluster::GetBatIECDesignation() const
{
    ValidateFeature(kBattery, BatIECDesignation);
    ValidateFeature(kReplaceable, BatIECDesignation);

    return mAttributes.battery.replaceable.GetIECDesignation();
}

PowerSourceCluster::BatApprovedChemistryEnum PowerSourceCluster::GetBatApprovedChemistry() const
{
    ValidateFeature(kBattery, BatApprovedChemistry);
    ValidateFeature(kReplaceable, BatApprovedChemistry);

    return mAttributes.battery.replaceable.approvedChemistry;
}

uint32_t PowerSourceCluster::GetBatCapacity() const
{
    ValidateFeature(kBattery, BatCapacity);
    VerifyOrDieWithMsg(
        Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), Zcl,
        "Attempting to read attribute `BatCapacity` when neither feature `kReplaceable` nor feature `kRechargeable` is set.");

    return mAttributes.battery.capacity;
}

uint8_t PowerSourceCluster::GetBatQuantity() const
{
    ValidateFeature(kBattery, BatQuantity);
    ValidateFeature(kReplaceable, BatQuantity);

    return mAttributes.battery.replaceable.quantity;
}

PowerSourceCluster::BatChargeStateEnum PowerSourceCluster::GetBatChargeState() const
{
    ValidateFeature(kBattery, BatChargeState);
    ValidateFeature(kRechargeable, BatChargeState);

    return mAttributes.battery.rechargeable.chargeState;
}

Optional<uint32_t> PowerSourceCluster::GetBatTimeToFullCharge() const
{
    ValidateFeature(kBattery, BatTimeToFullCharge);
    ValidateFeature(kRechargeable, BatTimeToFullCharge);

    return mAttributes.battery.rechargeable.timeToFullCharge;
}

bool PowerSourceCluster::GetBatFunctionalWhileCharging() const
{
    ValidateFeature(kBattery, BatFunctionalWhileCharging);
    ValidateFeature(kRechargeable, BatFunctionalWhileCharging);

    return mAttributes.battery.rechargeable.functionalWhileCharging;
}

Optional<uint32_t> PowerSourceCluster::GetBatChargingCurrent() const
{
    ValidateFeature(kBattery, BatChargingCurrent);
    ValidateFeature(kRechargeable, BatChargingCurrent);

    return mAttributes.battery.rechargeable.chargingCurrent;
}

Span<const PowerSourceCluster::BatChargeFaultEnum> PowerSourceCluster::GetActiveBatChargeFaults() const
{
    ValidateFeature(kBattery, ActiveBatChargeFaults);
    ValidateFeature(kRechargeable, ActiveBatChargeFaults);

    return BitSetToSpan(mAttributes.battery.rechargeable.activeFaults, mAttributes.battery.rechargeable.mActiveChargeFaultsBuf);
}

Span<const EndpointId> PowerSourceCluster::GetEndpointList() const
{
    return mAttributes.GetPoweredEndpoints();
}

// Setter implementations

#undef ValidateFeature

#define VerifyFeatureOrReturnError(feature_name)                                                                                   \
    VerifyOrReturnError(Features().Has(Feature::feature_name), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VerifyStringAttributeSizeOrReturnError(string_var_name, max_size)                                                          \
    VerifyOrReturnError(string_var_name.size() <= max_size, CHIP_ERROR_INVALID_STRING_LENGTH);

CHIP_ERROR PowerSourceCluster::SetStatus(PowerSourceStatusEnum val)
{
    SetAttributeValue(mAttributes.status, val, Status::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetOrder(uint8_t val)
{
    // This attribute is marked as `Persistent`.
    if (mContext != nullptr && val != mAttributes.order)
    {
        AttributePersistence attributePersistence(mContext->attributeStorage);

        attributePersistence.StoreNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, Order::Id }, val);
    }

    SetAttributeValue(mAttributes.order, val, Order::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputVoltage(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kWired);

    mAttributes.wired.assessedInputVoltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputFrequency(Optional<uint16_t> val)
{
    VerifyFeatureOrReturnError(kWired);

    mAttributes.wired.assessedInputFrequency = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedCurrent(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kWired);

    mAttributes.wired.assessedCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredPresent(bool val)
{
    VerifyFeatureOrReturnError(kWired);

    SetAttributeValue(mAttributes.wired.isPresent, val, WiredPresent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveWiredFaults(Span<const WiredFaultEnum> val)
{
    VerifyFeatureOrReturnError(kWired);

    auto bitset = SpanToBitSet(val);
    if (mAttributes.wired.activeFaults == bitset)
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    GenerateEvent<WiredFaultChange::Type>(mAttributes.wired.mActiveFaultsBuf, mAttributes.wired.activeFaults, bitset);

    mAttributes.wired.activeFaults = bitset;
    NotifyAttributeChanged(ActiveWiredFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::AddActiveWiredFault(WiredFaultEnum val)
{
    VerifyFeatureOrReturnError(kWired);

    if (mAttributes.wired.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if already present
    }

    auto bitset = mAttributes.wired.activeFaults;
    bitset.set(to_underlying(val));

    GenerateEvent<WiredFaultChange::Type>(mAttributes.wired.mActiveFaultsBuf, mAttributes.wired.activeFaults, bitset);

    mAttributes.wired.activeFaults = bitset;
    NotifyAttributeChanged(ActiveWiredFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::RemoveActiveWiredFault(WiredFaultEnum val)
{
    VerifyFeatureOrReturnError(kWired);

    if (!mAttributes.wired.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if not present
    }

    auto bitset = mAttributes.wired.activeFaults;
    bitset.set(to_underlying(val), false);

    GenerateEvent<WiredFaultChange::Type>(mAttributes.wired.mActiveFaultsBuf, mAttributes.wired.activeFaults, bitset);

    mAttributes.wired.activeFaults = bitset;
    NotifyAttributeChanged(ActiveWiredFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatVoltage(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);

    mAttributes.battery.voltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatPercentRemaining(Optional<uint8_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    if (val.HasValue())
    {
        // Maximum value of 200 representing 100% battery.
        VerifyOrReturnError(val.Value() <= 200, CHIP_ERROR_INVALID_INTEGER_VALUE);
    }

    // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
    // Or this attribute is to be reported if it changes to or from null.

    // If value changes from or to null, change, notify and return.
    if (mAttributes.battery.percentRemaining == NullOptional || val == NullOptional)
    {
        SetAttributeValue(mAttributes.battery.percentRemaining, val, BatPercentRemaining::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatPercentRemainingNotifyTimerExpired)
    {
        SetAttributeValue(mAttributes.battery.percentRemaining, val, BatPercentRemaining::Id);
        CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatPercentRemainingNotifyTimerExpired);
        ReturnErrorOnFailure(err);
        mBatPercentRemainingNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.percentRemaining = val;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatTimeRemaining(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);

    // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
    // Or this attribute is to be reported if it changes to or from null.

    // If value changes from or to null, change, notify and return.
    if (mAttributes.battery.timeRemaining == NullOptional || val == NullOptional)
    {
        SetAttributeValue(mAttributes.battery.timeRemaining, val, BatTimeRemaining::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatTimeRemainingNotifyTimerExpired)
    {
        SetAttributeValue(mAttributes.battery.timeRemaining, val, BatTimeRemaining::Id);
        CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeRemainingNotifyTimerExpired);
        ReturnErrorOnFailure(err);
        mBatTimeRemainingNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.timeRemaining = val;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatChargeLevel(BatChargeLevelEnum val)
{
    VerifyFeatureOrReturnError(kBattery);

    SetAttributeValue(mAttributes.battery.chargeLevel, val, BatChargeLevel::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatReplacementNeeded(bool val)
{
    VerifyFeatureOrReturnError(kBattery);

    SetAttributeValue(mAttributes.battery.replacementNeeded, val, BatReplacementNeeded::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatPresent(bool val)
{
    VerifyFeatureOrReturnError(kBattery);

    SetAttributeValue(mAttributes.battery.isPresent, val, BatPresent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveBatFaults(Span<const BatFaultEnum> val)
{
    VerifyFeatureOrReturnError(kBattery);

    auto bitset = SpanToBitSet(val);
    if (mAttributes.battery.activeFaults == bitset)
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    GenerateEvent<BatFaultChange::Type>(mAttributes.battery.mActiveFaultsBuf, mAttributes.battery.activeFaults, bitset);

    mAttributes.battery.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::AddActiveBatFault(BatFaultEnum val)
{
    VerifyFeatureOrReturnError(kBattery);

    if (mAttributes.battery.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if already present
    }

    auto bitset = mAttributes.battery.activeFaults;
    bitset.set(to_underlying(val));

    GenerateEvent<BatFaultChange::Type>(mAttributes.battery.mActiveFaultsBuf, mAttributes.battery.activeFaults, bitset);

    mAttributes.battery.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::RemoveActiveBatFault(BatFaultEnum val)
{
    VerifyFeatureOrReturnError(kBattery);

    if (!mAttributes.battery.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if not present
    }

    auto bitset = mAttributes.battery.activeFaults;
    bitset.set(to_underlying(val), false);

    GenerateEvent<BatFaultChange::Type>(mAttributes.battery.mActiveFaultsBuf, mAttributes.battery.activeFaults, bitset);

    mAttributes.battery.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatChargeState(BatChargeStateEnum val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    SetAttributeValue(mAttributes.battery.rechargeable.chargeState, val, BatChargeState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatTimeToFullCharge(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
    // Or this attribute is to be reported if it changes to or from null.

    // If value changes from or to null, change, notify and return.
    if (mAttributes.battery.rechargeable.timeToFullCharge == NullOptional || val == NullOptional)
    {
        SetAttributeValue(mAttributes.battery.rechargeable.timeToFullCharge, val, BatTimeToFullCharge::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatTimeToFullChargeNotifyTimerExpired)
    {
        SetAttributeValue(mAttributes.battery.rechargeable.timeToFullCharge, val, BatTimeToFullCharge::Id);
        CHIP_ERROR err = mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeToFullChargeNotifyTimerExpired);
        ReturnErrorOnFailure(err);
        mBatTimeToFullChargeNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.rechargeable.timeToFullCharge = val;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatFunctionalWhileCharging(bool val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    SetAttributeValue(mAttributes.battery.rechargeable.functionalWhileCharging, val, BatFunctionalWhileCharging::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatChargingCurrent(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    mAttributes.battery.rechargeable.chargingCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    auto bitset = SpanToBitSet(val);
    if (mAttributes.battery.rechargeable.activeFaults == bitset)
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    GenerateEvent<BatChargeFaultChange::Type>(mAttributes.battery.rechargeable.mActiveChargeFaultsBuf,
                                              mAttributes.battery.rechargeable.activeFaults, bitset);

    mAttributes.battery.rechargeable.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatChargeFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::AddActiveBatChargeFault(BatChargeFaultEnum val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);

    if (mAttributes.battery.rechargeable.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if already present
    }

    auto bitset = mAttributes.battery.rechargeable.activeFaults;
    bitset.set(to_underlying(val));

    GenerateEvent<BatChargeFaultChange::Type>(mAttributes.battery.rechargeable.mActiveChargeFaultsBuf,
                                              mAttributes.battery.rechargeable.activeFaults, bitset);

    mAttributes.battery.rechargeable.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatChargeFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::RemoveActiveBatChargeFault(BatChargeFaultEnum val)
{
    VerifyFeatureOrReturnError(kBattery);

    if (!mAttributes.battery.rechargeable.activeFaults.test(to_underlying(val)))
    {
        return CHIP_NO_ERROR; // no-op if not present
    }

    auto bitset = mAttributes.battery.rechargeable.activeFaults;
    bitset.set(to_underlying(val), false);

    GenerateEvent<BatChargeFaultChange::Type>(mAttributes.battery.rechargeable.mActiveChargeFaultsBuf,
                                              mAttributes.battery.rechargeable.activeFaults, bitset);

    mAttributes.battery.rechargeable.activeFaults = bitset;
    NotifyAttributeChanged(ActiveBatChargeFaults::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetEndpointList(Span<const EndpointId> val)
{
    if (mAttributes.GetPoweredEndpoints().data_equal(val))
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    if (val.size() > mAttributes.mPoweredEndpointsBuffer.AllocatedSize())
    {
        mAttributes.mPoweredEndpointsBuffer.Calloc(val.size());
    }

    mAttributes.mPoweredEndpointsCount = val.size();

    std::copy(val.begin(), val.end(), mAttributes.mPoweredEndpointsBuffer.Get());
    NotifyAttributeChanged(EndpointList::Id);
    return CHIP_NO_ERROR;
}

#undef VerifyFeatureOrReturnError
#undef VerifyStringAttributeSizeOrReturnError

} // namespace Clusters
} // namespace app
} // namespace chip
