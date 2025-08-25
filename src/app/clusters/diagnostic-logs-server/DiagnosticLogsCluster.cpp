/*
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

#include "DiagnosticLogsCluster.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <protocols/interaction_model/StatusCode.h>

#include <clusters/DiagnosticLogs/Ids.h>
#include <clusters/DiagnosticLogs/Metadata.h>

using namespace chip::app::Clusters::DiagnosticLogs;

namespace chip {
namespace app {
namespace Clusters {

using Protocols::InteractionModel::Status;

// Implementation of virtual methods from DefaultServerCluster
DataModel::ActionReturnStatus DiagnosticLogsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id:
        // No features defined for the cluster ...
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(DiagnosticLogs::kRevision);
    }

    return Status::UnsupportedAttribute;
}

std::optional<DataModel::ActionReturnStatus> DiagnosticLogsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    using namespace DiagnosticLogs::Commands;

    switch (request.path.mCommandId)
    {
    case RetrieveLogsRequest::Id: {
        RetrieveLogsRequest::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        auto intent   = commandData.intent;
        auto protocol = commandData.requestedProtocol;
        if (intent == IntentEnum::kUnknownEnumValue || protocol == TransferProtocolEnum::kUnknownEnumValue)
        {
            handler->AddStatus(request.path, Status::InvalidCommand);
            return std::nullopt;
        }
        if (protocol == TransferProtocolEnum::kResponsePayload)
        {
            return HandleLogRequestForResponsePayload(handler, request.path, commandData.intent);
        }
        return HandleLogRequestForBdx(handler, request.path, commandData.intent, commandData.transferFileDesignator);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    return std::nullopt;
}

CHIP_ERROR DiagnosticLogsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::RetrieveLogsRequest::kMetadataEntry };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR DiagnosticLogsCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = { Commands::RetrieveLogsResponse::Id };
    return builder.ReferenceExisting(kGeneratedCommands);
}

} // namespace Clusters
} // namespace app
} // namespace chip
