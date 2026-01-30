/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/illuminance-measurement-server/IlluminanceMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>

namespace chip::app::Clusters {

using namespace IlluminanceMeasurement::Attributes;

IlluminanceMeasurementCluster::IlluminanceMeasurementCluster(EndpointId endpointId,
                                                             const OptionalAttributeSet & optionalAttributeSet,
                                                             const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, IlluminanceMeasurement::Id }),
    mOptionalAttributeSet(optionalAttributeSet), mConfig(config)
{}

DataModel::ActionReturnStatus IlluminanceMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(IlluminanceMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    case MinMeasuredValue::Id:
        return encoder.Encode(mConfig.minMeasuredValue);
    case MaxMeasuredValue::Id:
        return encoder.Encode(mConfig.maxMeasuredValue);
    case Tolerance::Id:
        return encoder.Encode(mConfig.tolerance);
    case LightSensorType::Id:
        return encoder.Encode(mConfig.lightSensorType);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR IlluminanceMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        Tolerance::kMetadataEntry,       //
        LightSensorType::kMetadataEntry, //
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributeSet);
}

CHIP_ERROR IlluminanceMeasurementCluster::SetMeasuredValue(DataModel::Nullable<uint16_t> measuredValue)
{
    if (measuredValue.ValueOr(0) != 0)
    {
        // According to the spec, absolute minimum value is 1
        VerifyOrReturnError(measuredValue.Value() >= mConfig.minMeasuredValue.ValueOr(1), CHIP_ERROR_INVALID_ARGUMENT);

        // According to the spec, absolute maximum value is 0xFFFE / 65534
        VerifyOrReturnError(measuredValue.Value() <= mConfig.maxMeasuredValue.ValueOr(0xFFFE), CHIP_ERROR_INVALID_ARGUMENT);
    }
    SetAttributeValue(mMeasuredValue, measuredValue, MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
