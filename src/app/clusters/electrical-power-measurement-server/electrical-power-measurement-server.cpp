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
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->EncodeAccuracy(encoder); }));
        break;
    case Ranges::Id:
        if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeRanges))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->EncodeRanges(encoder); }));
        break;
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
        VerifyOrReturnError(
            HasFeature(ElectricalPowerMeasurement::Feature::kHarmonics), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Power Measurement: can not get HarmonicCurrents, feature is not supported"));
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->EncodeHarmonicCurrents(encoder); }));
        break;
    case HarmonicPhases::Id:
        VerifyOrReturnError(
            HasFeature(ElectricalPowerMeasurement::Feature::kPowerQuality), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Electrical Power Measurement: can not get HarmonicPhases, feature is not supported"));
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->EncodeHarmonicPhases(encoder); }));
        break;
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

CHIP_ERROR Instance::EncodeAccuracy(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mDelegate.StartAccuracyRead());
    for (uint8_t i = 0; true; i++)
    {
        Structs::MeasurementAccuracyStruct::Type accuracy;

        err = mDelegate.GetAccuracyByIndex(i, accuracy);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // Convert end of list to CHIP_NO_ERROR
            err = CHIP_NO_ERROR;
            goto exit;
        }

        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        err = encoder.Encode(accuracy);
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    mDelegate.EndAccuracyRead();
    return err;
}

CHIP_ERROR Instance::EncodeRanges(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mDelegate.StartRangesRead());
    for (uint8_t i = 0; true; i++)
    {
        Structs::MeasurementRangeStruct::Type range;

        err = mDelegate.GetRangeByIndex(i, range);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // Convert end of list to CHIP_NO_ERROR
            err = CHIP_NO_ERROR;
            goto exit;
        }

        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        err = encoder.Encode(range);
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    err = mDelegate.EndRangesRead();
    return err;
}

CHIP_ERROR Instance::EncodeHarmonicCurrents(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mDelegate.StartHarmonicCurrentsRead());
    for (uint8_t i = 0; true; i++)
    {
        Structs::HarmonicMeasurementStruct::Type current;

        err = mDelegate.GetHarmonicCurrentsByIndex(i, current);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // Convert end of list to CHIP_NO_ERROR
            err = CHIP_NO_ERROR;
            goto exit;
        }
        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        err = encoder.Encode(current);
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    err = mDelegate.EndHarmonicCurrentsRead();
    return err;
}

CHIP_ERROR Instance::EncodeHarmonicPhases(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mDelegate.StartHarmonicPhasesRead());
    for (uint8_t i = 0; true; i++)
    {
        Structs::HarmonicMeasurementStruct::Type phase;

        err = mDelegate.GetHarmonicPhasesByIndex(i, phase);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // Convert end of list to CHIP_NO_ERROR
            err = CHIP_NO_ERROR;
            goto exit;
        }
        // Check if another error occurred before trying to encode
        SuccessOrExit(err);

        err = encoder.Encode(phase);
        SuccessOrExit(err);
    }

exit:
    // Tell the delegate the read is complete
    err = mDelegate.EndHarmonicPhasesRead();
    return err;
}

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
