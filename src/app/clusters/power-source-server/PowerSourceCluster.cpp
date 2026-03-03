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
        // Again, Is the use of `WiredCurrentTypeEnum::kUnknownEnumValue` as an invalid value permitted?
        VerifyOrReturnError(mAttributes.wired.currentType != WiredCurrentTypeEnum::kUnknownEnumValue, CHIP_ERROR_INCORRECT_STATE);
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
        return encoder.Encode(mAttributes.battery.replaceable.GetAnsiDesignation());
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

} // namespace Clusters
} // namespace app
} // namespace chip
