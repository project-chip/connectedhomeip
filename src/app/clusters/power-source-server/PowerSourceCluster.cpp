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

#define DieIfInvalidValue(expr, attr_name)                                                                                         \
    {                                                                                                                              \
        CHIP_ERROR error_val = (expr);                                                                                             \
        VerifyOrDieWithMsg(error_val == CHIP_NO_ERROR || error_val == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Zcl,                    \
                           "Unexpected error %" CHIP_ERROR_FORMAT " when trying to set attribute `" #attr_name "`.",               \
                           error_val.Format());                                                                                    \
    }

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                       System::Layer & systemLayer, const WiredConfiguration & config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }),
    mOptionalAttributeSet(optionalAttributeSet), mFeatures(WiredFeatures()), mSystemLayer(systemLayer)
{
    forceOptionalAttributesValidity();

    mAttributes.wired = WiredAttributes{};
    CHIP_ERROR err;

    // mandatory attributes marked `Fixed` when `kWired` features is set
    VerifyOrDieWithMsg((err = SetDescription(config.description)) == CHIP_NO_ERROR, Zcl,
                       "Can't set the attribute `Description`, error: %" CHIP_ERROR_FORMAT, err.Format());
    VerifyOrDieWithMsg((err = SetWiredCurrentType(config.currentType)) == CHIP_NO_ERROR, Zcl,
                       "Can't set the attribute `WiredCurrentType`, error: %" CHIP_ERROR_FORMAT, err.Format());

    // optional attributes marked `Fixed`
    DieIfInvalidValue(SetWiredMaximumCurrent(config.maximumCurrent), WiredMaximumCurrent);
}

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet,
                                       System::Layer & systemLayer, const BatteryConfiguration & config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }),
    mOptionalAttributeSet(optionalAttributeSet), mFeatures(BatteryFeatures(config.isReplaceable(), config.isRechargeable())),
    mSystemLayer(systemLayer)
{
    forceOptionalAttributesValidity();

    mAttributes.battery = BatteryAttributes{};
    CHIP_ERROR err;

    // mandatory attributes marked `Fixed` when `kBattery` features is set
    VerifyOrDieWithMsg((err = SetDescription(config.description)) == CHIP_NO_ERROR, Zcl,
                       "Can't set the attribute `Description`, error: %" CHIP_ERROR_FORMAT, err.Format());
    VerifyOrDieWithMsg((err = SetBatReplaceability(config.replaceability)) == CHIP_NO_ERROR, Zcl,
                       "Can't set the attribute `BatReplaceability`, error: %" CHIP_ERROR_FORMAT, err.Format());

    if (config.isReplaceable())
    {
        // mandatory attributes marked `Fixed` when `kReplaceable` feature is set
        VerifyOrDieWithMsg((err = SetBatReplacementDescription(config.replacementDescription)) == CHIP_NO_ERROR, Zcl,
                           "Can't set the attribute `BatReplacementDescription`, error: %" CHIP_ERROR_FORMAT, err.Format());
        VerifyOrDieWithMsg((err = SetBatQuantity(config.quantity)) == CHIP_NO_ERROR, Zcl,
                           "Can't set the attribute `BatQuantity`, error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // no mandatory attributes marked `Fixed` when `kRechargeable` feature is set

    // optional attributes marked `Fixed` (deliberately ignoring errors because these will check if the optional attribute is
    // enabled)
    DieIfInvalidValue(SetBatCommonDesignation(config.commonDesignation), BatCommonDesignation);
    DieIfInvalidValue(SetBatANSIDesignation(config.ansiDesignation), BatANSIDesignation);
    DieIfInvalidValue(SetBatIECDesignation(config.iecDesignation), BatIECDesignation);
    DieIfInvalidValue(SetBatApprovedChemistry(config.approvedChemistry), BatApprovedChemistry);
    DieIfInvalidValue(SetBatCapacity(config.capacity), BatCapacity);
}

#undef DieIfInvalidValue

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
    AttributeListBuilder attributeListBuilder(builder);

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

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(kOptionalAttributes), mOptionalAttributeSet);
}

void PowerSourceCluster::forceOptionalAttributesValidity()
{
    if (!Features().Has(Feature::kWired))
    {
        // disable all attributes related to kWired feature
        mOptionalAttributeSet.Set<WiredAssessedInputVoltage::Id>(false);
        mOptionalAttributeSet.Set<WiredAssessedInputFrequency::Id>(false);
        mOptionalAttributeSet.Set<WiredCurrentType::Id>(false);
        mOptionalAttributeSet.Set<WiredAssessedCurrent::Id>(false);
        mOptionalAttributeSet.Set<WiredNominalVoltage::Id>(false);
        mOptionalAttributeSet.Set<WiredMaximumCurrent::Id>(false);
        mOptionalAttributeSet.Set<WiredPresent::Id>(false);
        mOptionalAttributeSet.Set<ActiveWiredFaults::Id>(false);
    }
    else
    {
        // enable all mandatory attributes related to kWired feature
        mOptionalAttributeSet.Set<WiredCurrentType::Id>(true);
    }

    if (!Features().Has(Feature::kBattery))
    {
        // disable all attributes related to kBattery feature
        mOptionalAttributeSet.Set<BatVoltage::Id>(false);
        mOptionalAttributeSet.Set<BatPercentRemaining::Id>(false);
        mOptionalAttributeSet.Set<BatTimeRemaining::Id>(false);
        mOptionalAttributeSet.Set<BatChargeLevel::Id>(false);
        mOptionalAttributeSet.Set<BatReplacementNeeded::Id>(false);
        mOptionalAttributeSet.Set<BatReplaceability::Id>(false);
        mOptionalAttributeSet.Set<BatPresent::Id>(false);
        mOptionalAttributeSet.Set<ActiveBatFaults::Id>(false);
    }
    else
    {
        // enable all mandatory attributes related to kBattery feature
        mOptionalAttributeSet.Set<BatChargeLevel::Id>(true);
        mOptionalAttributeSet.Set<BatReplacementNeeded::Id>(true);
        mOptionalAttributeSet.Set<BatReplaceability::Id>(true);
    }

    if (!Features().Has(Feature::kReplaceable))
    {
        // disable all attributes related to kReplaceable feature
        mOptionalAttributeSet.Set<BatReplacementDescription::Id>(false);
        mOptionalAttributeSet.Set<BatCommonDesignation::Id>(false);
        mOptionalAttributeSet.Set<BatANSIDesignation::Id>(false);
        mOptionalAttributeSet.Set<BatIECDesignation::Id>(false);
        mOptionalAttributeSet.Set<BatApprovedChemistry::Id>(false);
        mOptionalAttributeSet.Set<BatQuantity::Id>(false);
    }
    else
    {
        // enable all mandatory attributes related to kReplaceable feature
        mOptionalAttributeSet.Set<BatReplacementDescription::Id>(true);
        mOptionalAttributeSet.Set<BatQuantity::Id>(true);
    }

    if (!Features().Has(Feature::kRechargeable))
    {
        // disable all attributes related to kRechargeable feature
        mOptionalAttributeSet.Set<BatChargeState::Id>(false);
        mOptionalAttributeSet.Set<BatTimeToFullCharge::Id>(false);
        mOptionalAttributeSet.Set<BatFunctionalWhileCharging::Id>(false);
        mOptionalAttributeSet.Set<BatChargingCurrent::Id>(false);
        mOptionalAttributeSet.Set<ActiveBatChargeFaults::Id>(false);
    }
    else
    {
        // enable all mandatory attributes related to kRechargeable feature
        mOptionalAttributeSet.Set<BatChargeState::Id>(true);
        mOptionalAttributeSet.Set<BatFunctionalWhileCharging::Id>(true);
    }

    // BatCapacity attribute is optional when either kReplaceable or kRechargeable feature is supported
    // so if none of those features are supported, BatCapacity attribute should be disabled as well
    if (!Features().Has(Feature::kReplaceable) && !Features().Has(Feature::kRechargeable))
    {
        mOptionalAttributeSet.Set<BatCapacity::Id>(false);
    }
}

// Getter implementations

#define ValidateFeature(feature_name, attr_name)                                                                                   \
    VerifyOrDieWithMsg(Features().Has(Feature::feature_name), Zcl,                                                                 \
                       "Attempting to read attribute `" #attr_name "` when feature `" #feature_name "` is not set.");

#define ValidateOptionalAttribute(attr_name)                                                                                       \
    VerifyOrDieWithMsg(mOptionalAttributeSet.IsSet(attr_name::Id), Zcl,                                                            \
                       "Attempting to read attribute `" #attr_name "` when it is not specified as supported optional attribute.");

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
    ValidateOptionalAttribute(WiredAssessedInputVoltage);

    return mAttributes.wired.assessedInputVoltage;
}

Optional<uint16_t> PowerSourceCluster::GetWiredAssessedInputFrequency() const
{
    ValidateFeature(kWired, WiredAssessedInputFrequency);
    ValidateOptionalAttribute(WiredAssessedInputFrequency);

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
    ValidateOptionalAttribute(WiredAssessedCurrent);

    return mAttributes.wired.assessedCurrent;
}

uint32_t PowerSourceCluster::GetWiredNominalVoltage() const
{
    ValidateFeature(kWired, WiredNominalVoltage);
    ValidateOptionalAttribute(WiredNominalVoltage);

    return mAttributes.wired.nominalVoltage;
}

uint32_t PowerSourceCluster::GetWiredMaximumCurrent() const
{
    ValidateFeature(kWired, WiredMaximumCurrent);
    ValidateOptionalAttribute(WiredMaximumCurrent);

    return mAttributes.wired.maximumCurrent;
}

bool PowerSourceCluster::GetWiredPresent() const
{
    ValidateFeature(kWired, WiredPresent);
    ValidateOptionalAttribute(WiredPresent);

    return mAttributes.wired.isPresent;
}

Span<const PowerSourceCluster::WiredFaultEnum> PowerSourceCluster::GetActiveWiredFaults() const
{
    ValidateFeature(kWired, ActiveWiredFaults);
    ValidateOptionalAttribute(ActiveWiredFaults);

    return BitSetToSpan(mAttributes.wired.activeFaults, mAttributes.wired.mActiveFaultsBuf);
}

Optional<uint32_t> PowerSourceCluster::GetBatVoltage() const
{
    ValidateFeature(kBattery, BatVoltage);
    ValidateOptionalAttribute(BatVoltage);

    return mAttributes.battery.voltage;
}

Optional<uint8_t> PowerSourceCluster::GetBatPercentRemaining() const
{
    ValidateFeature(kBattery, BatPercentRemaining);
    ValidateOptionalAttribute(BatPercentRemaining);

    return mAttributes.battery.percentRemaining;
}

Optional<uint32_t> PowerSourceCluster::GetBatTimeRemaining() const
{
    ValidateFeature(kBattery, BatTimeRemaining);
    ValidateOptionalAttribute(BatTimeRemaining);

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
    ValidateOptionalAttribute(BatPresent);

    return mAttributes.battery.isPresent;
}

Span<const PowerSourceCluster::BatFaultEnum> PowerSourceCluster::GetActiveBatFaults() const
{
    ValidateFeature(kBattery, ActiveBatFaults);
    ValidateOptionalAttribute(ActiveBatFaults);

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
    ValidateOptionalAttribute(BatCommonDesignation);

    return mAttributes.battery.replaceable.commonDesignation;
}

CharSpan PowerSourceCluster::GetBatANSIDesignation() const
{
    ValidateFeature(kBattery, BatANSIDesignation);
    ValidateFeature(kReplaceable, BatANSIDesignation);
    ValidateOptionalAttribute(BatANSIDesignation);

    return mAttributes.battery.replaceable.GetANSIDesignation();
}

CharSpan PowerSourceCluster::GetBatIECDesignation() const
{
    ValidateFeature(kBattery, BatIECDesignation);
    ValidateFeature(kReplaceable, BatIECDesignation);
    ValidateOptionalAttribute(BatIECDesignation);

    return mAttributes.battery.replaceable.GetIECDesignation();
}

PowerSourceCluster::BatApprovedChemistryEnum PowerSourceCluster::GetBatApprovedChemistry() const
{
    ValidateFeature(kBattery, BatApprovedChemistry);
    ValidateFeature(kReplaceable, BatApprovedChemistry);
    ValidateOptionalAttribute(BatApprovedChemistry);

    return mAttributes.battery.replaceable.approvedChemistry;
}

uint32_t PowerSourceCluster::GetBatCapacity() const
{
    ValidateFeature(kBattery, BatCapacity);
    VerifyOrDieWithMsg(
        Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), Zcl,
        "Attempting to read attribute `BatCapacity` when neither feature `kReplaceable` nor feature `kRechargeable` is set.");
    ValidateOptionalAttribute(BatCapacity);

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
    ValidateOptionalAttribute(BatTimeToFullCharge);

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
    ValidateOptionalAttribute(BatChargingCurrent);

    return mAttributes.battery.rechargeable.chargingCurrent;
}

Span<const PowerSourceCluster::BatChargeFaultEnum> PowerSourceCluster::GetActiveBatChargeFaults() const
{
    ValidateFeature(kBattery, ActiveBatChargeFaults);
    ValidateFeature(kRechargeable, ActiveBatChargeFaults);
    ValidateOptionalAttribute(ActiveBatChargeFaults);

    return BitSetToSpan(mAttributes.battery.rechargeable.activeFaults, mAttributes.battery.rechargeable.mActiveChargeFaultsBuf);
}

Span<const EndpointId> PowerSourceCluster::GetEndpointList() const
{
    return mAttributes.GetPoweredEndpoints();
}

// Setter implementations

#undef ValidateFeature
#undef ValidateOptionalAttribute

#define VerifyFeatureOrReturnError(feature_name)                                                                                   \
    VerifyOrReturnError(Features().Has(Feature::feature_name), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VerifyOptionalAttributeOrReturnError(attr_name)                                                                            \
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(attr_name::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VerifyStringAttributeSizeOrReturnError(string_var_name, max_size)                                                          \
    VerifyOrReturnError(string_var_name.size() <= max_size, CHIP_ERROR_INVALID_STRING_LENGTH);

CHIP_ERROR PowerSourceCluster::SetStatus(PowerSourceStatusEnum val)
{
    SetAndNotify(mAttributes.status, val, Status::Id);
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

    SetAndNotify(mAttributes.order, val, Order::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputVoltage(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredAssessedInputVoltage);

    mAttributes.wired.assessedInputVoltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputFrequency(Optional<uint16_t> val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredAssessedInputFrequency);

    mAttributes.wired.assessedInputFrequency = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredAssessedCurrent(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredAssessedCurrent);

    mAttributes.wired.assessedCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredPresent(bool val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredPresent);

    SetAndNotify(mAttributes.wired.isPresent, val, WiredPresent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveWiredFaults(Span<const WiredFaultEnum> val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(ActiveWiredFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveWiredFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveWiredFaults);

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
    VerifyOptionalAttributeOrReturnError(BatVoltage);

    mAttributes.battery.voltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatPercentRemaining(Optional<uint8_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyOptionalAttributeOrReturnError(BatPercentRemaining);
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
        SetAndNotify(mAttributes.battery.percentRemaining, val, BatPercentRemaining::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatPercentRemainingNotifyTimerExpired)
    {
        SetAndNotify(mAttributes.battery.percentRemaining, val, BatPercentRemaining::Id);
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
    VerifyOptionalAttributeOrReturnError(BatTimeRemaining);

    // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
    // Or this attribute is to be reported if it changes to or from null.

    // If value changes from or to null, change, notify and return.
    if (mAttributes.battery.timeRemaining == NullOptional || val == NullOptional)
    {
        SetAndNotify(mAttributes.battery.timeRemaining, val, BatTimeRemaining::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatTimeRemainingNotifyTimerExpired)
    {
        SetAndNotify(mAttributes.battery.timeRemaining, val, BatTimeRemaining::Id);
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

    SetAndNotify(mAttributes.battery.chargeLevel, val, BatChargeLevel::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatReplacementNeeded(bool val)
{
    VerifyFeatureOrReturnError(kBattery);

    SetAndNotify(mAttributes.battery.replacementNeeded, val, BatReplacementNeeded::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatPresent(bool val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyOptionalAttributeOrReturnError(BatPresent);

    SetAndNotify(mAttributes.battery.isPresent, val, BatPresent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveBatFaults(Span<const BatFaultEnum> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyOptionalAttributeOrReturnError(ActiveBatFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveBatFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveBatFaults);

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

    SetAndNotify(mAttributes.battery.rechargeable.chargeState, val, BatChargeState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatTimeToFullCharge(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);
    VerifyOptionalAttributeOrReturnError(BatTimeToFullCharge);

    // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
    // Or this attribute is to be reported if it changes to or from null.

    // If value changes from or to null, change, notify and return.
    if (mAttributes.battery.rechargeable.timeToFullCharge == NullOptional || val == NullOptional)
    {
        SetAndNotify(mAttributes.battery.rechargeable.timeToFullCharge, val, BatTimeToFullCharge::Id);
        return CHIP_NO_ERROR;
    }

    // If the reporting interval has expired, update the value, notify, and restart the timer.
    if (mBatTimeToFullChargeNotifyTimerExpired)
    {
        SetAndNotify(mAttributes.battery.rechargeable.timeToFullCharge, val, BatTimeToFullCharge::Id);
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

    SetAndNotify(mAttributes.battery.rechargeable.functionalWhileCharging, val, BatFunctionalWhileCharging::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatChargingCurrent(Optional<uint32_t> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);
    VerifyOptionalAttributeOrReturnError(BatChargingCurrent);

    mAttributes.battery.rechargeable.chargingCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetActiveBatChargeFaults(Span<const BatChargeFaultEnum> val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kRechargeable);
    VerifyOptionalAttributeOrReturnError(ActiveBatChargeFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveBatChargeFaults);

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
    VerifyOptionalAttributeOrReturnError(ActiveBatChargeFaults);

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

// Private setter implementations for Fixed attributes

CHIP_ERROR PowerSourceCluster::SetDescription(CharSpan val)
{
    VerifyStringAttributeSizeOrReturnError(val, Description::TypeInfo::MaxLength());

    return SetStringAndNotify(val, mAttributes.GetDescription(), mAttributes.mDescriptionBuffer, Description::TypeInfo::MaxLength(),
                              Description::Id);
}

CHIP_ERROR PowerSourceCluster::SetWiredCurrentType(WiredCurrentTypeEnum val)
{
    VerifyFeatureOrReturnError(kWired);

    SetAndNotify(mAttributes.wired.currentType, val, WiredCurrentType::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredNominalVoltage(uint32_t val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredNominalVoltage);

    SetAndNotify(mAttributes.wired.nominalVoltage, val, WiredNominalVoltage::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetWiredMaximumCurrent(uint32_t val)
{
    VerifyFeatureOrReturnError(kWired);
    VerifyOptionalAttributeOrReturnError(WiredMaximumCurrent);

    SetAndNotify(mAttributes.wired.maximumCurrent, val, WiredMaximumCurrent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatReplaceability(BatReplaceabilityEnum val)
{
    VerifyFeatureOrReturnError(kBattery);

    SetAndNotify(mAttributes.battery.replaceability, val, BatReplaceability::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatReplacementDescription(CharSpan val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);
    VerifyOptionalAttributeOrReturnError(BatReplacementDescription);
    VerifyStringAttributeSizeOrReturnError(val, BatReplacementDescription::TypeInfo::MaxLength());

    return SetStringAndNotify(val, mAttributes.battery.replaceable.GetReplacementDescription(),
                              mAttributes.battery.replaceable.mReplacementDescriptionBuffer,
                              BatReplacementDescription::TypeInfo::MaxLength(), BatReplacementDescription::Id);
}

CHIP_ERROR PowerSourceCluster::SetBatCommonDesignation(BatCommonDesignationEnum val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);
    VerifyOptionalAttributeOrReturnError(BatCommonDesignation);

    SetAndNotify(mAttributes.battery.replaceable.commonDesignation, val, BatCommonDesignation::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatANSIDesignation(CharSpan val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);
    VerifyOptionalAttributeOrReturnError(BatANSIDesignation);
    VerifyStringAttributeSizeOrReturnError(val, BatANSIDesignation::TypeInfo::MaxLength());

    return SetStringAndNotify(val, mAttributes.battery.replaceable.GetANSIDesignation(),
                              mAttributes.battery.replaceable.mANSIDesignationBuffer, BatANSIDesignation::TypeInfo::MaxLength(),
                              BatANSIDesignation::Id);
}

CHIP_ERROR PowerSourceCluster::SetBatIECDesignation(CharSpan val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);
    VerifyOptionalAttributeOrReturnError(BatIECDesignation);
    VerifyStringAttributeSizeOrReturnError(val, BatIECDesignation::TypeInfo::MaxLength());

    return SetStringAndNotify(val, mAttributes.battery.replaceable.GetIECDesignation(),
                              mAttributes.battery.replaceable.mIECDesignationBuffer, BatIECDesignation::TypeInfo::MaxLength(),
                              BatIECDesignation::Id);
}

CHIP_ERROR PowerSourceCluster::SetBatApprovedChemistry(BatApprovedChemistryEnum val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);
    VerifyOptionalAttributeOrReturnError(BatApprovedChemistry);

    SetAndNotify(mAttributes.battery.replaceable.approvedChemistry, val, BatApprovedChemistry::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatCapacity(uint32_t val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyOrReturnError(Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOptionalAttributeOrReturnError(BatCapacity);

    SetAndNotify(mAttributes.battery.capacity, val, BatCapacity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceCluster::SetBatQuantity(uint8_t val)
{
    VerifyFeatureOrReturnError(kBattery);
    VerifyFeatureOrReturnError(kReplaceable);

    SetAndNotify(mAttributes.battery.replaceable.quantity, val, BatQuantity::Id);
    return CHIP_NO_ERROR;
}

#undef VerifyFeatureOrReturnError
#undef VerifyOptionalAttributeOrReturnError
#undef VerifyStringAttributeSizeOrReturnError

} // namespace Clusters
} // namespace app
} // namespace chip
