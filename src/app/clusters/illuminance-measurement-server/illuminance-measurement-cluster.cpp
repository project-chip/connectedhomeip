/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/illuminance-measurement-server/illuminance-measurement-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>

#include <platform/RuntimeOptionsProvider.h>

#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IlluminanceMeasurement;
using namespace chip::app::Clusters::IlluminanceMeasurement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

IlluminanceMeasurementCluster::IlluminanceMeasurementCluster(EndpointId endpointId,
                                                             const OptionalAttributeSet & optionalAttributeSet,
                                                             const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, IlluminanceMeasurement::Id }),
    mLightSensorType(config.lightSensorType), mTolerance(config.tolerance), mOptionalAttributeSet(optionalAttributeSet)
{}

DataModel::ActionReturnStatus IlluminanceMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case LightSensorType::Id:
        return encoder.Encode(mLightSensorType);
    case MaxMeasuredValue::Id:
        return encoder.Encode(mMaxMeasuredValue);
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    case MinMeasuredValue::Id:
        return encoder.Encode(mMinMeasuredValue);
    case Tolerance::Id:
        return encoder.Encode(mTolerance);
    case ClusterRevision::Id:
        return encoder.Encode(IlluminanceMeasurement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR IlluminanceMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        LightSensorType::kMetadataEntry, //
        Tolerance::kMetadataEntry,       //
    };

    // Full attribute set, to combine real "optional" attributes but also
    // attributes controlled by feature flags.
    app::OptionalAttributeSet<LightSensorType::Id, //
                              Tolerance::Id        //
                              >
        optionalAttributeSet(mOptionalAttributeSet);

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet);
}

CHIP_ERROR IlluminanceMeasurementCluster::SetMeasuredValue(MeasuredValue::TypeInfo::Type measuredValue)
{
    mMeasuredValue = measuredValue;
    NotifyAttributeChanged(MeasuredValue::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
