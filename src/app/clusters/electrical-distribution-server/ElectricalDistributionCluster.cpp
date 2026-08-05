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

#include "ElectricalDistributionCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ElectricalDistribution/EnumsCheck.h>
#include <clusters/ElectricalDistribution/Metadata.h>

namespace chip::app::Clusters {

using namespace ElectricalDistribution::Attributes;

ElectricalDistributionCluster::ElectricalDistributionCluster(EndpointId endpointId, const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, ElectricalDistribution::Id }), mMaxContinuousCurrent(config.maxContinuousCurrent),
    mMaxVoltage(config.maxVoltage), mNumberOfPoles(config.numberOfPoles), mEndOfLife(config.endOfLife),
    mServiceEntranceRated(config.serviceEntranceRated)
{}

DataModel::ActionReturnStatus ElectricalDistributionCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case MaxContinuousCurrent::Id:
        return encoder.Encode(mMaxContinuousCurrent);
    case MaxVoltage::Id:
        return encoder.Encode(mMaxVoltage);
    case NumberOfPoles::Id:
        return encoder.Encode(mNumberOfPoles);
    case EndOfLife::Id:
        return encoder.Encode(mEndOfLife);
    case ServiceEntranceRated::Id:
        return encoder.Encode(mServiceEntranceRated);
    case ClusterRevision::Id:
        return encoder.Encode(ElectricalDistribution::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ElectricalDistributionCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ElectricalDistribution::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR ElectricalDistributionCluster::SetEndOfLife(const EndOfLife::TypeInfo::Type & endOfLife)
{
    // EndOfLife is nullable, so null is a legal value. A non-null value must name a known enum
    // member: kUnknownEnumValue is the decode sentinel for values this build does not recognise,
    // and storing it would let an out-of-spec value be transmitted on a subsequent read.
    VerifyOrReturnError(endOfLife.IsNull() ||
                            EnsureKnownEnumValue(endOfLife.Value()) != ElectricalDistribution::EndOfLifeEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);

    SetAttributeValue(mEndOfLife, endOfLife, EndOfLife::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
