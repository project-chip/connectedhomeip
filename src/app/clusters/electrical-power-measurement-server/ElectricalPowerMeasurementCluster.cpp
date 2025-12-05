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

#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ElectricalPowerMeasurement/Attributes.h>
#include <clusters/ElectricalPowerMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

CHIP_ERROR ElectricalPowerMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // If we don't have AlternatingCurrent feature, ensure that all dependents that require it are not enabled
    if (!mFeatureFlags.Has(Feature::kAlternatingCurrent))
    {
        // PolyphasePower,Harmonics and PowerQuality features conformance: [ALTC]
        VerifyOrReturnError(!mFeatureFlags.HasAny(Feature::kPolyphasePower, Feature::kHarmonics, Feature::kPowerQuality),
                            CHIP_ERROR_INCORRECT_STATE);

        // ReactiveCurrent conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::ReactiveCurrent::Id), CHIP_ERROR_INCORRECT_STATE);
        // ApparentCurrent conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::ApparentCurrent::Id), CHIP_ERROR_INCORRECT_STATE);
        // ReactivePower conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::ReactivePower::Id), CHIP_ERROR_INCORRECT_STATE);
        // ApparentPower conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::ApparentPower::Id), CHIP_ERROR_INCORRECT_STATE);
        // RMSVoltage conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::RMSVoltage::Id), CHIP_ERROR_INCORRECT_STATE);
        // RMSCurrent conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::RMSCurrent::Id), CHIP_ERROR_INCORRECT_STATE);
        // RMSPower conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::RMSPower::Id), CHIP_ERROR_INCORRECT_STATE);
        // Frequency conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::Frequency::Id), CHIP_ERROR_INCORRECT_STATE);
        // PowerFactor conformance: [ALTC]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::PowerFactor::Id), CHIP_ERROR_INCORRECT_STATE);
    }

    // If we don't have PolyphasePower feature, ensure that NeutralCurrent attribute is not enabled
    if (!mFeatureFlags.Has(Feature::kPolyphasePower))
    {
        // NeutralCurrent conformance: [POLY]
        VerifyOrReturnError(!mEnabledOptionalAttributes.IsSet(Attributes::NeutralCurrent::Id), CHIP_ERROR_INCORRECT_STATE);
    }

    // Note: HarmonicCurrents (conformance: HARM) and HarmonicPhases (conformance: PWRQ) are automatically set based on features in
    // constructor

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ElectricalPowerMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        ReturnErrorOnFailure(encoder.Encode(mFeatureFlags));
        break;
    case Attributes::ClusterRevision::Id:
        ReturnErrorOnFailure(encoder.Encode(kRevision));
        break;
    case Attributes::PowerMode::Id:
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetPowerMode()));
        break;
    case Attributes::NumberOfMeasurementTypes::Id:
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetNumberOfMeasurementTypes()));
        break;
    case Attributes::Accuracy::Id:
        ReturnErrorOnFailure(
            encoder.EncodeList([this](const auto & aEncoder) -> CHIP_ERROR { return this->EncodeAccuracy(aEncoder); }));
        break;
    case Attributes::Ranges::Id:
        // Requires Ranges optional attribute
        ReturnErrorOnFailure(
            encoder.EncodeList([this](const auto & aEncoder) -> CHIP_ERROR { return this->EncodeRanges(aEncoder); }));
        break;
    case Attributes::Voltage::Id:
        // Requires Voltage attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetVoltage()));
        break;
    case Attributes::ActiveCurrent::Id:
        // Requires Active Current attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetActiveCurrent()));
        break;
    case Attributes::ReactiveCurrent::Id:
        // Requires Alternating Current feature and Reactive Current attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetReactiveCurrent()));
        break;
    case Attributes::ApparentCurrent::Id:
        // Requires Alternating Current feature and Apparent Current attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetApparentCurrent()));
        break;
    case Attributes::ActivePower::Id:
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetActivePower()));
        break;
    case Attributes::ReactivePower::Id:
        // Requires Alternating Current feature and Reactive Power attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetReactivePower()));
        break;
    case Attributes::ApparentPower::Id:
        // Requires Alternating Current feature and Apparent Power attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetApparentPower()));
        break;
    case Attributes::RMSVoltage::Id:
        // Requires Alternating Current feature and RMS Voltage attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetRMSVoltage()));
        break;
    case Attributes::RMSCurrent::Id:
        // Requires Alternating Current feature and RMS Current attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetRMSCurrent()));
        break;
    case Attributes::RMSPower::Id:
        // Requires Alternating Current feature and RMS Power attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetRMSPower()));
        break;
    case Attributes::Frequency::Id:
        // Requires Alternating Current feature and Frequency attribut
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetFrequency()));
        break;
    case Attributes::HarmonicCurrents::Id:
        // Requires Harmonics feature
        ReturnErrorOnFailure(
            encoder.EncodeList([this](const auto & aEncoder) -> CHIP_ERROR { return this->EncodeHarmonicCurrents(aEncoder); }));
        break;
    case Attributes::HarmonicPhases::Id:
        // Requires Power Quality feature
        ReturnErrorOnFailure(
            encoder.EncodeList([this](const auto & aEncoder) -> CHIP_ERROR { return this->EncodeHarmonicPhases(aEncoder); }));
        break;
    case Attributes::PowerFactor::Id:
        // Requires Alternating Current feature and Power Factor attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetPowerFactor()));
        break;
    case Attributes::NeutralCurrent::Id:
        // Requires Polyphase Power feature and Neutral current attribute
        ReturnErrorOnFailure(encoder.Encode(mDelegate.GetNeutralCurrent()));
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        Attributes::Ranges::kMetadataEntry,          //
        Attributes::Voltage::kMetadataEntry,         //
        Attributes::ActiveCurrent::kMetadataEntry,   //
        Attributes::ReactiveCurrent::kMetadataEntry, //
        Attributes::ApparentCurrent::kMetadataEntry, //
        Attributes::ReactivePower::kMetadataEntry,   //
        Attributes::ApparentPower::kMetadataEntry,   //
        Attributes::RMSVoltage::kMetadataEntry,      //
        Attributes::RMSCurrent::kMetadataEntry,      //
        Attributes::RMSPower::kMetadataEntry,        //
        Attributes::Frequency::kMetadataEntry,       //
        Attributes::PowerFactor::kMetadataEntry,     //
        Attributes::NeutralCurrent::kMetadataEntry,  //
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR ElectricalPowerMeasurementCluster::EncodeAccuracy(const AttributeValueEncoder::ListEncodeHelper & encoder)
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
    TEMPORARY_RETURN_IGNORED mDelegate.EndAccuracyRead();
    return err;
}

CHIP_ERROR ElectricalPowerMeasurementCluster::EncodeRanges(const AttributeValueEncoder::ListEncodeHelper & encoder)
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
    TEMPORARY_RETURN_IGNORED mDelegate.EndRangesRead();
    return err;
}

CHIP_ERROR ElectricalPowerMeasurementCluster::EncodeHarmonicCurrents(const AttributeValueEncoder::ListEncodeHelper & encoder)
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

CHIP_ERROR ElectricalPowerMeasurementCluster::EncodeHarmonicPhases(const AttributeValueEncoder::ListEncodeHelper & encoder)
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
