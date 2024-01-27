/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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
#include "electrical-power-measurement-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterface.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Structs;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    unregisterAttributeAccessOverride(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case PowerMode::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPowerMode()));
        break;
    case NumberOfMeasurementTypes::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetNumberOfMeasurementTypes()));
        break;
    case Accuracy::Id:
        return ReadAccuracy(aEncoder);
    case Ranges::Id:
        return ReadRanges(aEncoder);
    case Voltage::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeVoltage))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetVoltage()));
        break;
    case ActiveCurrent::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetActiveCurrent()));
        break;
    case ReactiveCurrent::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeReactiveCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(
            HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Power Measurement: can not get ReactiveCurrent, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetReactiveCurrent()));
        break;
    case ApparentCurrent::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeApparentCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(
            HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Power Measurement: can not get ApparentCurrent, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetApparentCurrent()));
        break;
    case ActivePower::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetActivePower()));
        break;
    case ReactivePower::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeReactivePower))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get ReactivePower, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetReactivePower()));
        break;
    case ApparentPower::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeApparentPower))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get ApparentPower, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetApparentPower()));
        break;
    case RMSVoltage::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeRMSVoltage))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get RMSVoltage, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetRMSVoltage()));
        break;
    case RMSCurrent::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeRMSCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get RMSCurrent, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetRMSCurrent()));
        break;
    case RMSPower::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeRMSPower))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get RMSPower, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetRMSPower()));
        break;
    case Frequency::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeFrequency))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get Frequency, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetFrequency()));
        break;
    case HarmonicCurrents::Id:
        return ReadHarmonicCurrents(aEncoder);
    case HarmonicPhases::Id:
        return ReadHarmonicPhases(aEncoder);
    case PowerFactor::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributePowerFactor))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Electrical Power Measurement: can not get PowerFactor, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetPowerFactor()));
        break;
    case NeutralCurrent::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeNeutralCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        VerifyOrReturnError(
            HasFeature(ElectricalPowerMeasurement::Feature::kPolyphasePower), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Power Measurement: can not get NeutralCurrent, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate.GetNeutralCurrent()));
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::ReadAccuracy(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    auto accuracies = mDelegate.IterateAccuracy();
    VerifyOrReturnError(accuracies != nullptr, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    if (accuracies->Count() == 0)
    {
        err = aEncoder.EncodeEmptyList();
    }
    else
    {
        err = aEncoder.EncodeList([&accuracies](const auto & encoder) -> CHIP_ERROR {
            Structs::MeasurementAccuracyStruct::Type accuracy;
            while (accuracies->Next(accuracy))
            {
                encoder.Encode(accuracy);
            }

            return CHIP_NO_ERROR;
        });
    }
    accuracies->Release();
    return err;
}

CHIP_ERROR Instance::ReadRanges(AttributeValueEncoder & aEncoder)
{
    if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeRanges))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto ranges    = mDelegate.IterateRanges();
    VerifyOrReturnError(ranges != nullptr, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    if (ranges->Count() == 0)
    {
        err = aEncoder.EncodeEmptyList();
    }
    else
    {
        err = aEncoder.EncodeList([&ranges](const auto & encoder) -> CHIP_ERROR {
            Structs::MeasurementRangeStruct::Type range;
            while (ranges->Next(range))
            {
                encoder.Encode(range);
            }

            return CHIP_NO_ERROR;
        });
    }
    ranges->Release();
    return err;
}

CHIP_ERROR Instance::ReadHarmonicCurrents(AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kHarmonics), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Electrical Power Measurement: can not get HarmonicCurrents, feature is not supported"));

    CHIP_ERROR err = CHIP_NO_ERROR;
    auto currents  = mDelegate.IterateHarmonicCurrents();
    VerifyOrReturnError(currents != nullptr, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    if (currents->Count() == 0)
    {
        err = aEncoder.EncodeEmptyList();
    }
    else
    {
        err = aEncoder.EncodeList([&currents](const auto & encoder) -> CHIP_ERROR {
            Structs::HarmonicMeasurementStruct::Type current;
            while (currents->Next(current))
            {
                encoder.Encode(current);
            }

            return CHIP_NO_ERROR;
        });
    }
    currents->Release();
    return err;
}

CHIP_ERROR Instance::ReadHarmonicPhases(AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(HasFeature(ElectricalPowerMeasurement::Feature::kPowerQuality), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Electrical Power Measurement: can not get HarmonicPhases, feature is not supported"));
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto phases    = mDelegate.IterateHarmonicPhases();
    VerifyOrReturnError(phases != nullptr, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    if (phases->Count() == 0)
    {
        err = aEncoder.EncodeEmptyList();
    }
    else
    {
        err = aEncoder.EncodeList([&phases](const auto & encoder) -> CHIP_ERROR {
            Structs::HarmonicMeasurementStruct::Type phase;
            while (phases->Next(phase))
            {
                encoder.Encode(phase);
            }

            return CHIP_NO_ERROR;
        });
    }
    phases->Release();
    return err;
}

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
