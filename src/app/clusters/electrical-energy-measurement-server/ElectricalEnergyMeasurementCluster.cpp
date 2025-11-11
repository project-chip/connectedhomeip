/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "ElectricalEnergyMeasurementCluster.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ElectricalEnergyMeasurement/Attributes.h>
#include <clusters/ElectricalEnergyMeasurement/Enums.h>
#include <clusters/ElectricalEnergyMeasurement/Events.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

using namespace ElectricalEnergyMeasurement::Attributes;
using namespace ElectricalEnergyMeasurement::Structs;
using namespace ElectricalEnergyMeasurement;

ElectricalEnergyMeasurementCluster::ElectricalEnergyMeasurementCluster(const Config & config) :
    DefaultServerCluster({ config.endpointId, ElectricalEnergyMeasurement::Id }), mFeatureFlags(config.mFeatureFlags),
    mEnabledOptionalAttributes(config.mEnabledOptionalAttributes)
{
    // Verify that optional attributes match the enabled features
    // CumulativeEnergyImported requires kCumulativeEnergy + kImportedEnergy
    VerifyOrDieWithMsg(!mEnabledOptionalAttributes.IsSet(CumulativeEnergyImported::Id) ||
                           (mFeatureFlags.Has(Feature::kCumulativeEnergy) && mFeatureFlags.Has(Feature::kImportedEnergy)),
                       Zcl, "CumulativeEnergyImported attribute requires kCumulativeEnergy and kImportedEnergy features");

    // CumulativeEnergyExported requires kCumulativeEnergy + kExportedEnergy
    VerifyOrDieWithMsg(!mEnabledOptionalAttributes.IsSet(CumulativeEnergyExported::Id) ||
                           (mFeatureFlags.Has(Feature::kCumulativeEnergy) && mFeatureFlags.Has(Feature::kExportedEnergy)),
                       Zcl, "CumulativeEnergyExported attribute requires kCumulativeEnergy and kExportedEnergy features");

    // PeriodicEnergyImported requires kPeriodicEnergy + kImportedEnergy
    VerifyOrDieWithMsg(!mEnabledOptionalAttributes.IsSet(PeriodicEnergyImported::Id) ||
                           (mFeatureFlags.Has(Feature::kPeriodicEnergy) && mFeatureFlags.Has(Feature::kImportedEnergy)),
                       Zcl, "PeriodicEnergyImported attribute requires kPeriodicEnergy and kImportedEnergy features");

    // PeriodicEnergyExported requires kPeriodicEnergy + kExportedEnergy
    VerifyOrDieWithMsg(!mEnabledOptionalAttributes.IsSet(PeriodicEnergyExported::Id) ||
                           (mFeatureFlags.Has(Feature::kPeriodicEnergy) && mFeatureFlags.Has(Feature::kExportedEnergy)),
                       Zcl, "PeriodicEnergyExported attribute requires kPeriodicEnergy and kExportedEnergy features");

    // CumulativeEnergyReset requires kCumulativeEnergy
    VerifyOrDieWithMsg(!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id) ||
                           mFeatureFlags.Has(Feature::kCumulativeEnergy),
                       Zcl, "CumulativeEnergyReset attribute requires kCumulativeEnergy feature");
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const
{
    outValue = mMeasurementData.measurementAccuracy;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyImported requires kCumulativeEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeImported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyExported requires kCumulativeEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeExported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.periodicImported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.periodicExported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyReset(Optional<CumulativeEnergyResetStruct> & outValue) const
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeReset;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetMeasurementAccuracy(const MeasurementAccuracyStruct & value)
{
    mMeasurementData.measurementAccuracy = value;
    NotifyAttributeChanged(Accuracy::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyImported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyImported requires kCumulativeEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    mMeasurementData.cumulativeImported = value;
    NotifyAttributeChanged(CumulativeEnergyImported::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyExported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyExported requires kCumulativeEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    mMeasurementData.cumulativeExported = value;
    NotifyAttributeChanged(CumulativeEnergyExported::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyImported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    mMeasurementData.periodicImported = value;
    NotifyAttributeChanged(PeriodicEnergyImported::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyExported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy) ||
        !mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    mMeasurementData.periodicExported = value;
    NotifyAttributeChanged(PeriodicEnergyExported::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyReset(const Optional<CumulativeEnergyResetStruct> & value)
{
    if (!mFeatureFlags.Has(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    mMeasurementData.cumulativeReset = value;
    NotifyAttributeChanged(CumulativeEnergyReset::Id);
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ElectricalEnergyMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(mFeatureFlags.Raw());

    case Accuracy::Id: {
        MeasurementAccuracyStruct accuracy;
        CHIP_ERROR err = GetMeasurementAccuracy(accuracy);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        return encoder.Encode(accuracy);
    }

    case CumulativeEnergyImported::Id: {
        Optional<EnergyMeasurementStruct> value;
        CHIP_ERROR err = GetCumulativeEnergyImported(value);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        if (!value.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(value.Value());
    }

    case CumulativeEnergyExported::Id: {
        Optional<EnergyMeasurementStruct> value;
        CHIP_ERROR err = GetCumulativeEnergyExported(value);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        if (!value.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(value.Value());
    }

    case PeriodicEnergyImported::Id: {
        Optional<EnergyMeasurementStruct> value;
        CHIP_ERROR err = GetPeriodicEnergyImported(value);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        if (!value.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(value.Value());
    }

    case PeriodicEnergyExported::Id: {
        Optional<EnergyMeasurementStruct> value;
        CHIP_ERROR err = GetPeriodicEnergyExported(value);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        if (!value.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(value.Value());
    }

    case CumulativeEnergyReset::Id: {
        if (!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id))
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        Optional<CumulativeEnergyResetStruct> value;
        CHIP_ERROR err = GetCumulativeEnergyReset(value);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        if (!value.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(value.Value());
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        CumulativeEnergyImported::kMetadataEntry, //
        CumulativeEnergyExported::kMetadataEntry, //
        PeriodicEnergyImported::kMetadataEntry,   //
        PeriodicEnergyExported::kMetadataEntry,   //
        CumulativeEnergyReset::kMetadataEntry,    //
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

std::optional<EventNumber>
ElectricalEnergyMeasurementCluster::CumulativeEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                                             const Optional<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturnValue(Features().Has(Feature::kCumulativeEnergy), std::nullopt);
    Events::CumulativeEnergyMeasured::Type event;

    if (Features().Has(Feature::kImportedEnergy))
    {
        SetCumulativeEnergyImported(energyImported);
        event.energyImported = energyImported;
    }

    if (Features().Has(Feature::kExportedEnergy))
    {
        SetCumulativeEnergyExported(energyExported);
        event.energyExported = energyExported;
    }

    VerifyOrReturnValue(mContext != nullptr, std::nullopt);
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber>
ElectricalEnergyMeasurementCluster::PeriodicEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                                           const Optional<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturnValue(Features().Has(Feature::kPeriodicEnergy), std::nullopt);
    Events::PeriodicEnergyMeasured::Type event;

    if (Features().Has(Feature::kImportedEnergy))
    {
        SetPeriodicEnergyImported(energyImported);
        event.energyImported = energyImported;
    }

    if (Features().Has(Feature::kExportedEnergy))
    {
        SetPeriodicEnergyExported(energyExported);
        event.energyExported = energyExported;
    }

    VerifyOrReturnValue(mContext != nullptr, std::nullopt);
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace Clusters
} // namespace app
} // namespace chip
