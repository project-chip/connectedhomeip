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

#include <server-cluster/AttributeListBuilder.h>
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

#define IGNORE_ERROR (void)

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer, WiredConfiguration config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
    mFeatures(WiredFeatures()),
    mSystemLayer(systemLayer)
{
    mAttributes.wired = WiredAttributes{};
    CHIP_ERROR err;

    // mandatory attributes marked `Fixed` when `kWired` features is set
    VerifyOrDieWithMsg(err = SetDescription(config.description), NotSpecified, "Can't set the attribute `Description`, error: %" CHIP_ERROR_FORMAT, err.Format());
    VerifyOrDieWithMsg(err = SetWiredCurrentType(config.currentType), NotSpecified, "Can't set the attribute `WiredCurrentType`, error: %" CHIP_ERROR_FORMAT, err.Format());

    // optional attributes marked `Fixed` (deliberately ignoring errors)
    IGNORE_ERROR SetWiredNominalVoltage(config.nominalVoltage);
    IGNORE_ERROR SetWiredMaximumCurrent(config.maximumCurrent);
}

PowerSourceCluster::PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer, BatteryConfiguration config) :
    DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
    mFeatures(BatteryFeatures(config.isReplaceable(), config.isRechargeable())),
    mSystemLayer(systemLayer)
{
    mAttributes.battery = BatteryAttributes{};
    CHIP_ERROR err;

    // mandatory attributes marked `Fixed` when `kBattery` features is set
    VerifyOrDieWithMsg(err = SetDescription(config.description), NotSpecified, "Can't set the attribute `Description`, error: %" CHIP_ERROR_FORMAT, err.Format());
    VerifyOrDieWithMsg(err = SetBatReplaceability(config.replaceability), NotSpecified, "Can't set the attribute `BatReplaceability`, error: %" CHIP_ERROR_FORMAT, err.Format());

    if (config.isReplaceable())
    {
        // mandatory attributes marked `Fixed` when `kReplaceable` feature is set
        VerifyOrDieWithMsg(err = SetBatReplacementDescription(config.replacementDescription), NotSpecified, "Can't set the attribute `BatReplacementDescription`, error: %" CHIP_ERROR_FORMAT, err.Format());
        VerifyOrDieWithMsg(err = SetBatQuantity(config.quantity), NotSpecified, "Can't set the attribute `BatQuantity`, error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // no mandatory attributes marked `Fixed` when `kRehcargeable` feature is set

    // optional attributes marked `Fixed` (deliberately ignoring errors)
    IGNORE_ERROR SetBatCommonDesignation(config.commonDesignation);
    IGNORE_ERROR SetBatANSIDesignation(config.ansiDesignation);
    IGNORE_ERROR SetBatIECDesignation(config.iecDesignation);
    IGNORE_ERROR SetBatApprovedChemistry(config.approvedChemistry);
    IGNORE_ERROR SetBatCapacity(config.capacity);
}

#undef IGNORE_ERROR

DataModel::ActionReturnStatus PowerSourceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                AttributeValueEncoder & encoder)
{
    using namespace PowerSource::Attributes;
    // `ReadAttribute` is guaranteed to only be called for attributes that are supported by the cluster, so the code below is valid.
    switch (request.path.mAttributeId)
    {
    case Status::Id:
        return encoder.Encode(mAttrributes.status);
    case Order::Id:
        return encoder.Encode(mAttributes.order);
    case Description::Id:
        return encoder.Encode(mAttributes.description);
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
        return EncodeListOfValues(encoder, mAttributes.wired.GetActiveFaults());
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
        return EncodeListOfValues(encoder, mAttributes.battery.GetActiveFaults());
    case BatReplacementDescription::Id:
        return encoder.Encode(mAttributes.battery.replaceable.GetReplacementDescription());
    case BatCommonDesignation::Id:
        return encoder.Encode(mAttributes.battery.replaceable.GetCommonDesignation());
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
        return EncodeListOfValues(encoder, mAttributes.battery.rechargeable.GetActiveChargeFaults());
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
    forceOptionalAttributesValidity();
    AttributeListBuilder attributeListBuilder(builder);

    constexpr DataModel::AttributeEntry kOptionalAttributes[] = {
        WiredAssessedInputVoltage::Id,
        WiredAssessedInputFrequency::Id,
        WiredCurrentType::Id,
        WiredAssessedCurrent::Id,
        WiredNominalVoltage::Id,
        WiredMaximumCurrent::Id,
        WiredPresent::Id,
        ActiveWiredFaults::Id,
        BatVoltage::Id,
        BatPercentRemaining::Id,
        BatTimeRemaining::Id,
        BatChargeLevel::Id,
        BatReplacementNeeded::Id,
        BatReplaceability::Id,
        BatPresent::Id,
        ActiveBatFaults::Id,
        BatReplacementDescription::Id,
        BatCommonDesignation::Id,
        BatANSIDesignation::Id,
        BatIECDesignation::Id,
        BatApprovedChemistry::Id,
        BatCapacity::Id,
        BatQuantity::Id,
        BatChargeState::Id,
        BatTimeToFullCharge::Id,
        BatFunctionalWhileCharging::Id,
        BatChargingCurrent::Id,
        ActiveBatChargeFaults::Id
    };

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

#define VALIDATE_FEATURE(feature_name, attr_name) \
    VerifyOrDieWithMsg(Features().Has(Feature::##feature_name), NotSpecified, "Attempting to read attribute `" #attr_name "` when feature `" #feature_name "` is not set.");

#define VALIDATE_OPTIONAL_ATTRIBUTE(attr_name) \
    VerifyOrDieWithMsg(mOptionalAttributeSet.IsSet(##attr_name::Id), NotSpecified, "Attempting to read attribute `" #attr_name "` when it is not specified as supported optional attribute");

inline PowerSourceCluster::PowerSourceStatusEnum PowerSourceCluster::GetStatus() const { return mAttributes.status; }

inline uint8 PowerSourceCluster::GetOrder() const { return mAttributes.order; }

inline CharSpan PowerSourceCluster::GetDescription() const { return mAttributes.GetDescription(); }

inline Optional<uint32_t> PowerSourceCluster::GetWiredAssessedInputVoltage() const
{
    // here and down, where the attribute is optional, the feature validation is done only for logging purposes,
    // because the optional attribute validation is fully enough for code correctness
    VALIDATE_FEATURE(kWired, WiredAssessedInputVoltage);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedInputVoltage);

    return mAttributes.wired.assessedInputVoltage;
}

inline Optional<uint16_t> PowerSourceCluster::GetWiredAssessedInputFrequency() const
{
    VALIDATE_FEATURE(kWired, WiredAssessedInputFrequency);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedInputFrequency);

    return mAttributes.wired.assessedInputFrequency;
}

inline PowerSourceCluster::WiredCurrentTypeEnum PowerSourceCluster::GetWiredCurrentType() const
{
    VALIDATE_FEATURE(kWired, WiredCurrentType);
    VerifyOrDieWithMsg(mAttributes.wired.currentType != WiredCurrentTypeEnum::kUnknownEnumValue, NotSpecified, "WiredCurrentType is not set");

    return mAttributes.wired.currentType;
}

inline Optional<uint32_t> PowerSourceCluster::GetWiredAssessedCurrent() const
{
    VALIDATE_FEATURE(kWired, WiredAssessedCurrent);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedCurrent);

    return mAttributes.wired.assessedCurrent;
}

inline uint32_t PowerSourceCluster::GetWiredNominalVoltage() const
{
    VALIDATE_FEATURE(kWired, WiredNominalVoltage);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredNominalVoltage);

    return mAttributes.wired.nominalVoltage;
}

inline uint32_t PowerSourceCluster::GetWiredMaximumCurrent() const
{
    VALIDATE_FEATURE(kWired, WiredMaximumCurrent);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredMaximumCurrent);

    return mAttributes.wired.maximumCurrent;
}

inline bool PowerSourceCluster::GetWiredPresent() const
{
    VALIDATE_FEATURE(kWired, WiredPresent);
    VALIDATE_OPTIONAL_ATTRIBUTE(WiredPresent);

    return mAttributes.wired.isPresent;
}

inline Span<PowerSourceCluster::WiredFaultEnum> PowerSourceCluster::GetActiveWiredFaults() const
{
    VALIDATE_FEATURE(kWired, ActiveWiredFaults);
    VALIDATE_OPTIONAL_ATTRIBUTE(ActiveWiredFaults);

    return mAttributes.wired.GetActiveFaults();
}

inline Optional<uint32_t> PowerSourceCluster::GetBatVoltage() const
{
    VALIDATE_FEATURE(kBattery, BatVoltage);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatVoltage);

    return mAttributes.battery.voltage;
}

inline Optional<uint8_t> PowerSourceCluster::GetBatPercentRemaining() const
{
    VALIDATE_FEATURE(kBattery, BatPercentRemaining);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatPercentRemaining);

    return mAttributes.battery.percentRemaining;
}

inline Optional<uint32_t> PowerSourceCluster::GetBatTimeRemaining() const
{
    VALIDATE_FEATURE(kBattery, BatTimeRemaining);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatTimeRemaining);

    return mAttributes.battery.timeRemaining;
}

inline PowerSourceCluster::BatChargeLevelEnum PowerSourceCluster::GetBatChargeLevel() const
{
    VALIDATE_FEATURE(kBattery, BatChargeLevel);

    return mAttributes.battery.chargeLevel;
}

inline bool PowerSourceCluster::GetBatReplacementNeeded() const
{
    VALIDATE_FEATURE(kBattery, BatReplacementNeeded);

    return mAttributes.battery.replacementNeeded;
}

inline PowerSourceCluster::BatReplaceabilityEnum PowerSourceCluster::GetBatReplaceability() const
{
    VALIDATE_FEATURE(kBattery, BatReplaceability);

    return mAttributes.battery.replaceability;
}

inline bool PowerSourceCluster::GetBatPresent() const
{
    VALIDATE_FEATURE(kBattery, BatPresent);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatPresent);

    return mAttributes.battery.isPresent;
}

inline Span<PowerSourceCluster::BatFaultEnum> PowerSourceCluster::GetActiveBatFaults() const
{
    VALIDATE_FEATURE(kBattery, ActiveBatFaults);
    VALIDATE_OPTIONAL_ATTRIBUTE(ActiveBatFaults);

    return mAttributes.battery.GetActiveFaults();
}

inline CharSpan PowerSourceCluster::GetBatReplacementDescription() const
{
    VALIDATE_FEATURE(kBattery, BatReplacementDescription);
    VALIDATE_FEATURE(kReplaceable, BatReplacementDescription);

    return mAttributes.battery.replaceable.GetReplacementDescription();
}

inline PowerSourceCluster::BatCommonDesignationEnum PowerSourceCluster::GetBatCommonDesignation() const
{
    VALIDATE_FEATURE(kBattery, BatCommonDesignation);
    VALIDATE_FEATURE(kReplaceable, BatCommonDesignation);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatCommonDesignation);

    return mAttributes.battery.replaceable.commonDesignation;
}

inline CharSpan PowerSourceCluster::GetBatANSIDesignation() const
{
    VALIDATE_FEATURE(kBattery, BatANSIDesignation);
    VALIDATE_FEATURE(kReplaceable, BatANSIDesignation);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatANSIDesignation);

    return mAttributes.battery.replaceable.GetANSIDesignation();
}

inline CharSpan PowerSourceCluster::GetBatIECDesignation() const
{
    VALIDATE_FEATURE(kBattery, BatIECDesignation);
    VALIDATE_FEATURE(kReplaceable, BatIECDesignation);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatIECDesignation);

    return mAttributes.battery.replaceable.GetIECDesignation();
}

inline PowerSourceCluster::BatApprovedChemistryEnum PowerSourceCluster::GetBatApprovedChemistry() const
{
    VALIDATE_FEATURE(kBattery, BatApprovedChemistry);
    VALIDATE_FEATURE(kReplaceable, BatApprovedChemistry);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatApprovedChemistry);

    return mAttributes.battery.replaceable.approvedChemistry;
}

inline uint32_t PowerSourceCluster::GetBatCapacity() const
{
    VALIDATE_FEATURE(kBattery, BatCapacity);
    VerifyOrDieWithMsg(Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), NotSpecified, "Attempting to read attribute `BatCapacity` when neither feature `kReplaceable` nor feature `kRechargeable` is set.");
    VALIDATE_OPTIONAL_ATTRIBUTE(BatCapacity);

    return mAttributes.battery.capacity;
}

inline uint8_t PowerSourceCluster::GetBatQuantity() const
{
    VALIDATE_FEATURE(kBattery, BatQuantity);
    VALIDATE_FEATURE(kReplaceable, BatQuantity);

    return mAttributes.battery.replaceable.quantity;
}

inline PowerSourceCluster::BatChargeStateEnum PowerSourceCluster::GetBatChargeState() const
{
    VALIDATE_FEATURE(kBattery, BatChargeState);
    VALIDATE_FEATURE(kRechargeable, BatChargeState);

    return mAttributes.battery.rechargeable.chargeState;
}

inline Optional<uint32> PowerSourceCluster::GetBatTimeToFullCharge() const
{
    VALIDATE_FEATURE(kBattery, BatTimeToFullCharge);
    VALIDATE_FEATURE(kRechargeable, BatTimeToFullCharge);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatTimeToFullCharge);

    return mAttributes.battery.rechargeable.timeToFullCharge;
}

inline bool PowerSourceCluster::GetBatFunctionalWhileCharging() const
{
    VALIDATE_FEATURE(kBattery, BatFunctionalWhileCharging);
    VALIDATE_FEATURE(kRechargeable, BatFunctionalWhileCharging);

    return mAttributes.battery.rechargeable.functionalWhileCharging;
}

inline Optional<uint32_t> PowerSourceCluster::GetBatChargingCurrent() const
{
    VALIDATE_FEATURE(kBattery, BatChargingCurrent);
    VALIDATE_FEATURE(kRechargeable, BatChargingCurrent);
    VALIDATE_OPTIONAL_ATTRIBUTE(BatChargingCurrent);

    return mAttributes.battery.rechargeable.chargingCurrent;
}

inline Span<PowerSourceCluster::BatChargeFaultEnum> PowerSourceCluster::GetActiveBatChargeFaults() const
{
    VALIDATE_FEATURE(kBattery, ActiveBatChargeFaults);
    VALIDATE_FEATURE(kRechargeable, ActiveBatChargeFaults);
    VALIDATE_OPTIONAL_ATTRIBUTE(ActiveBatChargeFaults);

    return mAttributes.battery.rechargeable.GetActiveChargeFaults();
}

inline Span<EndpointId> PowerSourceCluster::GetEndpointList() const { return mAttributes.GetPoweredEndpoints(); }

// Setter implementations

#undef VALIDATE_FEATURE
#undef VALIDATE_OPTIONAL_ATTRIBUTE

#define VERIFY_FEATURE_OR_RETURN_ERROR(feature_name) \
    VerifyOrReturnError(Features().Has(Feature::##feature_name), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(attr_name) \
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(##attr_name::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(list_var_name, max_size) \
    VerifyOrReturnError(list_var_name##.size() <= max_size, CHIP_ERROR_INVALID_LIST_LENGTH);

#define VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(string_var_name, max_size) \
    VerifyOrReturnError(string_var_name##.size() <= max_size, CHIP_ERROR_INVALID_STRING_LENGTH);

inline CHIP_ERROR PowerSourceCluster::SetStatus(PowerSourceStatusEnum val)
{
    SetAndNotify(mAttributes.status, val, Status::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetOrder(uint8_t val)
{
    SetAndNotify(mAttributes.order, val, Order::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputVoltage(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedInputVoltage);

    mAttributes.wired.assessedInputVoltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredAssessedInputFrequency(Optional<uint16_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedInputFrequency);

    mAttributes.wired.assessedInputFrequency = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredAssessedCurrent(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedCurrent);

    mAttributes.wired.assessedCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredPresent(bool val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredPresent);

    SetAndNotify(mAttributes.wired.isPresent, val, WiredPresent::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetActiveWiredFaults(Span<WiredFaultEnum> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveWiredFaults);
    VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.wired.kMaxActiveFaults);

    if (mAttributes.wired.GetActiveFaults().data_equal(val))
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    PowerSource::Events::WiredFaultChange::Type event_data{mAttributes.wired.GetActiveFaults(), val};
    CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
    if (err != CHIP_NO_ERROR) return err;

    std::copy(val.begin(), val.end(), mAttributes.wired.mActiveFaultsBuffer);
    NotifyAttributeChanged(ActiveWiredFaults::Id);

    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatVoltage(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatVoltage);

    mAttributes.battery.voltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatPercentRemaining(Optional<uint8_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatPercentRemaining);
    if (val.HasValue())
    {
        // Maximum value of 200 representing 100% battery.
        VerifyOrReturnError(*val <= 200, CHIP_ERROR_INVALID_INTEGER_VALUE);
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
        mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatPercentRemainingNotifyTimerExpired);
        mBatPercentRemainingNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.percentRemaining = val;
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatTimeRemaining(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatTimeRemaining);

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
        mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeRemainingNotifyTimerExpired);
        mBatTimeRemainingNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.timeRemaining = val;
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatChargeLevel(BatChargeLevelEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);

    SetAndNotify(mAttributes.battery.replacementNeeded, val, BatReplacementNeeded::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatPresent(bool val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatPresent);

    SetAndNotify(mAttributes.battery.isPresent, val, BatPresent::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetActiveBatFaults(Span<BatFaultEnum> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveBatFaults);
    VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.battery.kMaxActiveFaults);

    if (mAttributes.battery.GetActiveFaults().data_equal(val))
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    PowerSource::Events::BatFaultChange::Type event_data{mAttributes.battery.GetActiveFaults(), val};
    CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
    if (err != CHIP_NO_ERROR) return err;

    std::copy(val.begin(), val.end(), mAttributes.battery.mActiveFaultsBuffer);
    NotifyAttributeChanged(ActiveBatFaults::Id);

    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatChargeState(BatChargeStateEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);

    SetAndNotify(mAttributes.battery.rechargeable.chargeState, val, BatChargeState::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatTimeToFullCharge(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatTimeToFullCharge);

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
        mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeToFullChargeNotifyTimerExpired);
        mBatTimeToFullChargeNotifyTimerExpired = false;
        return CHIP_NO_ERROR;
    }

    // Otherwise, the reporting interval is still active, do not notify.
    mAttributes.battery.rechargeable.timeToFullCharge = val;
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatFunctionalWhileCharging(bool val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);

    SetAndNotify(mAttributes.battery.rechargeable.functionalWhileCharging, val, BatFunctionalWhileCharging::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatChargingCurrent(Optional<uint32_t> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatChargingCurrent);

    mAttributes.battery.rechargeable.chargingCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetActiveBatChargeFaults(Span<BatChargeFaultEnum> val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveBatChargeFaults);
    VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.battery.rechargeable.kMaxActiveChargeFaults);

    if (mAttributes.battery.rechargeable.GetActiveChargeFaults().data_equal(val))
    {
        return CHIP_NO_ERROR; // no-op if equal
    }

    PowerSource::Events::BatChargeFaultChange::Type event_data{mAttributes.battery.rechargeable.GetActiveChargeFaults(), val};
    CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
    if (err != CHIP_NO_ERROR) return err;

    std::copy(val.begin(), val.end(), mAttributes.battery.rechargeable.mActiveChargeFaultsBuffer);
    NotifyAttributeChanged(ActiveBatChargeFaults::Id);

    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetEndpointList(Span<EndpointId> val)
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

inline CHIP_ERROR PowerSourceCluster::SetDescription(CharSpan val)
{
    VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, Description::TypeInfo::MaxLength());

    return SetStringAndNotify(
        val,
        mAttributes.GetDescription(),
        mAttributes.mDescriptionBuffer,
        Description::TypeInfo::MaxLength(),
        Description::Id
    );
}

inline CHIP_ERROR PowerSourceCluster::SetWiredCurrentType(WiredCurrentTypeEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);

    SetAndNotify(mAttributes.wired.currentType, val, WiredCurrentType::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredNominalVoltage(uint32_t val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredNominalVoltage);

    SetAndNotify(mAttributes.wired.nominalVoltage, val, WiredNominalVoltage::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetWiredMaximumCurrent(uint32_t val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredMaximumCurrent);

    SetAndNotify(mAttributes.wired.maximumCurrent, val, WiredMaximumCurrent::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatReplaceability(BatReplaceabilityEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);

    SetAndNotify(mAttributes.battery.replaceability, val, BatReplaceability::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatReplacementDescription(CharSpan val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatReplacementDescription);
    VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, BatReplacementDescription::TypeInfo::MaxLength());

    return SetStringAndNotify(
        val,
        mAttributes.battery.replaceable.GetReplacementDescription(),
        mAttributes.battery.replaceable.mReplacementDescriptionBuffer,
        BatReplacementDescription::TypeInfo::MaxLength(),
        BatReplacementDescription::Id
    );
}

inline CHIP_ERROR PowerSourceCluster::SetBatCommonDesignation(BatCommonDesignationEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatCommonDesignation);

    SetAndNotify(mAttributes.battery.replaceable.commonDesignation, val, BatCommonDesignation::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatANSIDesignation(CharSpan val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatANSIDesignation);
    VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, BatANSIDesignation::TypeInfo::MaxLength());

    return SetStringAndNotify(
        val,
        mAttributes.battery.replaceable.GetANSIDesignation(),
        mAttributes.battery.replaceable.mANSIDesignationBuffer,
        BatANSIDesignation::TypeInfo::MaxLength(),
        BatANSIDesignation::Id
    );
}

inline CHIP_ERROR PowerSourceCluster::SetBatIECDesignation(CharSpan val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatIECDesignation);
    VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, BatIECDesignation::TypeInfo::MaxLength());

    return SetStringAndNotify(
        val,
        mAttributes.battery.replaceable.GetIECDesignation(),
        mAttributes.battery.replaceable.mIECDesignationBuffer,
        BatIECDesignation::TypeInfo::MaxLength(),
        BatIECDesignation::Id
    );
}

inline CHIP_ERROR PowerSourceCluster::SetBatApprovedChemistry(BatApprovedChemistryEnum val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatApprovedChemistry);

    SetAndNotify(mAttributes.battery.replaceable.approvedChemistry, val, BatApprovedChemistry::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatCapacity(uint32_t val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VerifyOrReturnError(Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatCapacity);

    SetAndNotify(mAttributes.battery.capacity, val, BatCapacity::Id);
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR PowerSourceCluster::SetBatQuantity(uint8_t val)
{
    VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
    VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);

    SetAndNotify(mAttributes.battery.replaceable.quantity, val, BatQuantity::Id);
    return CHIP_NO_ERROR;
}

#undef VERIFY_FEATURE_OR_RETURN_ERROR
#undef VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR
#undef VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR
#undef VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR

} // namespace Clusters
} // namespace app
} // namespace chip
