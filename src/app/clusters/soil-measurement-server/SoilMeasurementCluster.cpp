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

#include "SoilMeasurementCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/SoilMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace SoilMeasurement::Attributes;

SoilMeasurementCluster::SoilMeasurementCluster(
    EndpointId endpointId, const SoilMoistureMeasurementLimits::TypeInfo::Type & soilMoistureMeasurementLimits) :
    DefaultServerCluster({ endpointId, SoilMeasurement::Id }),
    mSoilMoistureMeasurementLimits(soilMoistureMeasurementLimits)
{
    mSoilMoistureMeasuredValue.SetNull();
}

DataModel::ActionReturnStatus SoilMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case SoilMoistureMeasurementLimits::Id:
        return encoder.Encode(mSoilMoistureMeasurementLimits);
    case SoilMoistureMeasuredValue::Id:
        return encoder.Encode(mSoilMoistureMeasuredValue);
    case ClusterRevision::Id:
        return encoder.Encode(SoilMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR SoilMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(SoilMeasurement::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR
SoilMeasurementCluster::SetSoilMoistureMeasuredValue(const SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    VerifyOrReturnError(mSoilMoistureMeasuredValue != soilMoistureMeasuredValue, CHIP_NO_ERROR);

    if (!soilMoistureMeasuredValue.IsNull())
    {
        VerifyOrReturnError(soilMoistureMeasuredValue.Value() >= mSoilMoistureMeasurementLimits.minMeasuredValue &&
                                soilMoistureMeasuredValue.Value() <= mSoilMoistureMeasurementLimits.maxMeasuredValue,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    mSoilMoistureMeasuredValue = soilMoistureMeasuredValue;

    NotifyAttributeChanged(SoilMoistureMeasuredValue::Id);

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
