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

#include <app/EventLogging.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/BooleanState/Metadata.h>

namespace chip::app::Clusters {

using namespace BooleanState::Attributes;

BooleanStateCluster::BooleanStateCluster(EndpointId endpointId) :
    DefaultServerCluster({ endpointId, BooleanState::Id }), mStateValue(false)
{}

DataModel::ActionReturnStatus BooleanStateCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case StateValue::Id:
        return encoder.Encode(mStateValue);
    case ClusterRevision::Id:
        return encoder.Encode(BooleanState::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR BooleanStateCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(BooleanState::Attributes::kMandatoryMetadata), {});
}

std::optional<EventNumber> BooleanStateCluster::SetStateValue(bool stateValue)
{
    VerifyOrReturnValue(SetAttributeValue(mStateValue, stateValue, StateValue::Id), std::nullopt);
    VerifyOrReturnValue(mContext != nullptr, std::nullopt);
    BooleanState::Events::StateChange::Type event{ stateValue };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters
