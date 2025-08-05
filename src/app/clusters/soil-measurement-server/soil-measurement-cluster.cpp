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
#include <clusters/SoilMeasurement/Metadata.h>
#include <tracing/macros.h>

namespace chip {
namespace app {
namespace Clusters {

SoilMeasurementCluster::SoilMeasurementCluster(EndpointId endpointId) : DefaultServerCluster({ endpointId, SoilMeasurement::Id }) {}

CHIP_ERROR SoilMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    mLogic.Startup(mPath.mEndpointId, context.attributeStorage);

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus SoilMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::Id: {
        return encoder.Encode(mLogic.GetSoilMoistureMeasurementLimits());
    }
    case SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id: {
        return encoder.Encode(mLogic.GetSoilMoistureMeasuredValue());
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
    return mLogic.Attributes(builder);
}

CHIP_ERROR
SoilMeasurementCluster::SetSoilMoistureMeasuredValue(
    EndpointId endpointId, const SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    VerifyOrReturnError(endpointId == mPath.mEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    return mLogic.SetSoilMoistureMeasuredValue(endpointId, soilMoistureMeasuredValue);
}

} // namespace Clusters
} // namespace app
} // namespace chip
