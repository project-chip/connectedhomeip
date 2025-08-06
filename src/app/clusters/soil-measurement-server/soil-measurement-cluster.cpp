/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "soil-measurement-cluster.h"
#include <app/reporting/reporting.h>
#include <clusters/SoilMeasurement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <tracing/macros.h>

namespace chip {
namespace app {
namespace Clusters {

SoilMeasurementCluster::SoilMeasurementCluster(EndpointId endpointId) : DefaultServerCluster({ endpointId, SoilMeasurement::Id })
{
    mSoilMoistureMeasuredValue.SetNull();
}

CHIP_ERROR SoilMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // VerifyOrReturnError(context.attributeStorage != nullptr, CHIP_INVALID_ARGUMENT);

    SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type measuredValue;
    MutableByteSpan measuredValueBytes(reinterpret_cast<uint8_t *>(&measuredValue), sizeof(measuredValue));
    CHIP_ERROR error = context.attributeStorage->ReadValue(
        { mPath.mEndpointId, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id }, measuredValueBytes);

    if (error != CHIP_NO_ERROR)
    {
        measuredValue.SetNull();
    }

    mSoilMoistureMeasuredValue = measuredValue;
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus SoilMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::Id: {
        return encoder.Encode(GetSoilMoistureMeasurementLimits());
    }
    case SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id: {
        return encoder.Encode(GetSoilMoistureMeasuredValue());
    }
    case SoilMeasurement::Attributes::ClusterRevision::Id: {
        return encoder.Encode(SoilMeasurement::kRevision);
    }
    case SoilMeasurement::Attributes::FeatureMap::Id: {
        return encoder.Encode(static_cast<uint32_t>(0));
    }
    default:
        return Protocols::InteractionModel::Status::UnreportableAttribute;
    }
}

CHIP_ERROR SoilMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // Ensure capacity just in case
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(2 + DefaultServerCluster::GlobalAttributes().size()));
    // Mandatory attributes
    ReturnErrorOnFailure(builder.Append(SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::kMetadataEntry));
    ReturnErrorOnFailure(builder.Append(SoilMeasurement::Attributes::SoilMoistureMeasuredValue::kMetadataEntry));

    // Finally, the global attributes
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR
SoilMeasurementCluster::SetSoilMoistureMeasuredValue(
    EndpointId endpointId, const SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    if (mSoilMoistureMeasuredValue != soilMoistureMeasuredValue)
    {
        mSoilMoistureMeasuredValue = soilMoistureMeasuredValue;

        MatterReportingAttributeChangeCallback(endpointId, SoilMeasurement::Id,
                                               SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id);
    }

    return CHIP_NO_ERROR;
}

SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type
SoilMeasurementCluster::GetSoilMoistureMeasurementLimits()
{
    return mSoilMoistureMeasurementLimits;
}

SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type SoilMeasurementCluster::GetSoilMoistureMeasuredValue()
{
    return mSoilMoistureMeasuredValue;
}

CHIP_ERROR
SoilMeasurementCluster::SetSoilMoistureMeasurementLimits(
    const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type & soilMoistureMeasurementLimits)
{
    mSoilMoistureMeasurementLimits = soilMoistureMeasurementLimits;
    mSoilMoistureMeasuredValue.SetNull();

    return CHIP_NO_ERROR;
}

} // namespace Clusters
} // namespace app
} // namespace chip
