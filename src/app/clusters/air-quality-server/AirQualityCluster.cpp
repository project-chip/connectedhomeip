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

#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/AirQuality/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {

using AirQuality::AirQualityEnum;
using AirQuality::Feature;

AirQualityCluster::AirQualityCluster(EndpointId endpointId) : DefaultServerCluster({ endpointId, AirQuality::Id }) {}

DataModel::ActionReturnStatus AirQualityCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case AirQuality::Attributes::AirQuality::Id:
        return encoder.Encode(mAirQuality);
    case AirQuality::Attributes::ClusterRevision::Id:
        return encoder.Encode(AirQuality::kRevision);
    case AirQuality::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeature.Raw());
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR AirQualityCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(AirQuality::Attributes::kMandatoryMetadata), {});
}

bool AirQualityCluster::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

Protocols::InteractionModel::Status AirQualityCluster::SetAirQuality(AirQualityEnum aNewAirQuality)
{
    // Check that the value is valid according to the enabled features.
    switch (aNewAirQuality)
    {
    case AirQualityEnum::kFair: {
        if (!HasFeature(Feature::kFair))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kModerate: {
        if (!HasFeature(Feature::kModerate))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kVeryPoor: {
        if (!HasFeature(Feature::kVeryPoor))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kExtremelyPoor: {
        if (!HasFeature(Feature::kExtremelyPoor))
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
    }
    break;
    case AirQualityEnum::kUnknown:
    case AirQualityEnum::kGood:
    case AirQualityEnum::kPoor:
        break;
    default: {
        return Protocols::InteractionModel::Status::InvalidValue;
    }
    }

    SetAttributeValue(mAirQuality, aNewAirQuality, AirQuality::Attributes::AirQuality::Id);
    return Protocols::InteractionModel::Status::Success;
}

AirQualityEnum AirQualityCluster::GetAirQuality() const
{
    return mAirQuality;
}

void AirQualityCluster::SetFeatureMap(BitFlags<Feature> features)
{
    mFeature = features;
}

} // namespace Clusters
} // namespace app
} // namespace chip
