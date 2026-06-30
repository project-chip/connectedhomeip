/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/dynamic-lighting-server/DynamicLightingCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/DynamicLighting/Commands.h>
#include <clusters/DynamicLighting/Metadata.h>

namespace chip::app::Clusters {

using namespace DynamicLighting;
using namespace DynamicLighting::Attributes;

DynamicLightingCluster::DynamicLightingCluster(EndpointId endpointId) : DefaultServerCluster({ endpointId, DynamicLighting::Id }) {}

DataModel::ActionReturnStatus DynamicLightingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(DynamicLighting::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case AvailableEffects::Id:
        return encoder.EncodeEmptyList();
    case CurrentEffectID::Id:
        return encoder.EncodeNull();
    case CurrentSpeed::Id:
        return encoder.Encode(DataModel::Nullable<uint16_t>{ DataModel::NullNullable });
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus DynamicLightingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                     AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case CurrentSpeed::Id:
        return Protocols::InteractionModel::Status::InvalidInState;
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR DynamicLightingCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(DynamicLighting::Attributes::kMandatoryMetadata), {});
}

std::optional<DataModel::ActionReturnStatus> DynamicLightingCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                   chip::TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::StartEffect::Id:
    case Commands::StopEffect::Id:
        return Protocols::InteractionModel::Status::InvalidCommand;
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR DynamicLightingCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({
        Commands::StartEffect::kMetadataEntry,
        Commands::StopEffect::kMetadataEntry,
    });
}

} // namespace chip::app::Clusters
