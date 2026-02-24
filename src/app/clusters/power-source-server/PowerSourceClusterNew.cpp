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

#include "PowerSourceClusterNew.h"
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
        return encoder.Encode(GetBaseDelegate().GetStatus());
    case Order::Id:
        return encoder.Encode(GetBaseDelegate().GetOrder());
    case Description::Id:
        return encoder.Encode(GetBaseDelegate().GetDescription());
    case WiredAssessedInputVoltage::Id:
        Optional<uint32_t> value{};
        CHIP_ERROR err = GetWiredDelegate().GetAssessedInputVoltage(value);
        return EncodeOptional(encoder, value, err);
    case WiredAssessedInputFrequency::Id:
        Optional<uint16_t> frequency{};
        CHIP_ERROR err = GetWiredDelegate().GetAssessedInputFrequency(frequency);
        return EncodeOptional(encoder, frequency, err);
    case WiredCurrentType::Id:
        return encoder.Encode(GetWiredDelegate().GetCurrentType());
    case WiredAssessedCurrent::Id:
        Optional<uint16_t> current{};
        CHIP_ERROR err = GetWiredDelegate().GetAssessedCurrent(current);
        return EncodeOptional(encoder, current, err);
    case WiredNominalVoltage::Id:
        uint32_t nominalVoltage{};
        CHIP_ERROR err = GetWiredDelegate().GetNominalVoltage(nominalVoltage);
        return EncodeValue(encoder, nominalVoltage, err);
    case WiredMaximumCurrent::Id:
        uint32_t maximumCurrent{};
        CHIP_ERROR err = GetWiredDelegate().GetMaximumCurrent(maximumCurrent);
        return EncodeValue(encoder, maximumCurrent, err);
    case WiredPresent::Id:
        bool present{};
        CHIP_ERROR err = GetWiredDelegate().IsPresent(present);
        return EncodeValue(encoder, present, err);
    case ActiveWiredFaults::Id:
        Span<const WiredFaultEnum> activeWiredFaults{};
        CHIP_ERROR err = GetWiredDelegate().GetActiveWiredFaults(activeWiredFaults);
        return EncodeListOfValues(encoder, activeWiredFaults, err);
    case BatVoltage::Id:
        Optional<uint32_t> voltage{};
        CHIP_ERROR err = GetBatteryDelegate().GetVoltage(voltage);
        return EncodeOptional(encoder, voltage, err);
    case BatPercentRemaining::Id:
        Optional<uint8_t> percent{};
        CHIP_ERROR err = GetBatteryDelegate().GetPercentRemaining(percent);
        return EncodeOptional(encoder, percent, err);
    case BatTimeRemaining::Id:
        Optional<uint32_t> time{};
        CHIP_ERROR err = GetBatteryDelegate().GetTimeRemaining(time);
        return EncodeOptional(encoder, time, err);
    case BatChargeLevel::Id:
        return encoder.Encode(GetBatteryDelegate().GetChargeLevel());
    case BatReplacementNeeded::Id:
        return encoder.Encode(GetBatteryDelegate().IsReplacmentNeeded());
    case BatReplaceability::Id:
        return encoder.Encode(GetBatteryDelegate().GetReplaceability());
    case BatPresent::Id:
        bool present{};
        CHIP_ERROR err = GetBatteryDelegate().IsPresent(present);
        return EncodeValue(encoder, present, err);
    case ActiveBatFaults::Id:
        Span<const BatFaultEnum> activeBatFaults{};
        CHIP_ERROR err = GetBatteryDelegate().GetActiveFaults(activeBatFaults);
        return EncodeListOfValues(encoder, activeBatFaults, err);
    case BatReplacementDescription::Id:
        return encoder.Encode(GetReplacableBatteryDelegate().GetReplacementDescription());
    case BatCommonDesignation::Id:
        BatCommonDesignationEnum commonDesignation{};
        CHIP_ERROR err = GetReplacableBatteryDelegate().GetCommonDesignation(commonDesignation);
        return EncodeValue(encoder, commonDesignation, err);
    case BatANSIDesignation::Id:
        CharSpan ansiDesignation{};
        CHIP_ERROR err = GetReplacableBatteryDelegate().GetANSIDesignation(ansiDesignation);
        return EncodeValue(encoder, ansiDesignation, err);
    case BatIECDesignation::Id:
        CharSpan iecDesignation{};
        CHIP_ERROR err = GetReplacableBatteryDelegate().GetIECDesignation(iecDesignation);
        return EncodeValue(encoder, iecDesignation, err);
    case BatApprovedChemistry::Id:
        BatApprovedChemistryEnum approvedChemistry{};
        CHIP_ERROR err = GetReplacableBatteryDelegate().GetApprovedChemistry(approvedChemistry);
        return EncodeValue(encoder, approvedChemistry, err);
    case BatCapacity::Id:
        uint32_t capacity{};
        if (Features().Has(Feature::kReplaceable))
        {
            CHIP_ERROR err = GetReplacableBatteryDelegate().GetCapacity(capacity);
            return EncodeValue(encoder, capacity, err);
        }
        if (Features().Has(Feature::kRechargeable))
        {
            CHIP_ERROR err = GetRechargeableBatteryDelegate().GetCapacity(capacity);
            return EncodeValue(encoder, capacity, err);
        }
    case BatQuantity::Id:
        return encoder.Encode(GetReplacableBatteryDelegate().GetQuantity());
    case BatChargeState::Id:
        return encoder.Encode(GetRechargeableBatteryDelegate().GetChargeState());
    case BatTimeToFullCharge::Id:
        Optional<uint32_t> timeToFullCharge{};
        CHIP_ERROR err = GetRechargeableBatteryDelegate().GetTimeToFullCharge(timeToFullCharge);
        return EncodeOptional(encoder, timeToFullCharge, err);
    case BatFunctionalWhileCharging::Id:
        return encoder.Encode(GetRechargeableBatteryDelegate().IsFunctionalWhileCharging());
    case BatChargingCurrent::Id:
        Optional<uint32_t> chargingCurrent{};
        CHIP_ERROR err = GetRechargeableBatteryDelegate().GetChargingCurrent(chargingCurrent);
        return EncodeOptional(encoder, chargingCurrent, err);
    case ActiveBatChargeFaults::Id:
        Span<const BatChargeFaultEnum> activeChargeFaults{};
        CHIP_ERROR err = GetRechargeableBatteryDelegate().GetActiveChargeFaults(activeChargeFaults);
        return EncodeListOfValues(encoder, activeChargeFaults, err);
    case EndpointList::Id:
        return EncodeListOfValues(encoder, GetBaseDelegate().GetPoweredEndpoints());
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
