/**
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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

#include <app/clusters/keypad-input-server/KeypadInputCluster.h>

#include <app/CommandResponseHelper.h>
#include <app/data-model/Decode.h>
#include <clusters/KeypadInput/Attributes.h>
#include <clusters/KeypadInput/Commands.h>
#include <clusters/KeypadInput/Ids.h>
#include <clusters/KeypadInput/Metadata.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters {

using namespace KeypadInput;
using Protocols::InteractionModel::Status;

KeypadInputCluster::KeypadInputCluster(EndpointId endpointId, KeypadInput::Delegate & delegate) :
    DefaultServerCluster({ endpointId, KeypadInput::Id }), mDelegate(delegate)
{}

DataModel::ActionReturnStatus KeypadInputCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(KeypadInput::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mDelegate.GetFeatureMap(mPath.mEndpointId));
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> KeypadInputCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                               TLV::TLVReader & input_arguments,
                                                                               CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SendKey::Id: {
        Commands::SendKey::DecodableType data;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, data));

        CommandResponseHelper<Commands::SendKeyResponse::Type> responder(handler, request.path);
        mDelegate.HandleSendKey(responder, data.keyCode);
        if (responder.HasSentResponse())
        {
            return std::nullopt;
        }
        ChipLogError(Zcl, "KeypadInput: SendKey on endpoint %u was not answered by the delegate",
                     static_cast<unsigned>(mPath.mEndpointId));
        return Status::Failure;
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR KeypadInputCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = { Commands::SendKey::kMetadataEntry };
    return builder.ReferenceExisting(Span(kCommands));
}

CHIP_ERROR KeypadInputCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kCommands[] = { Commands::SendKeyResponse::Id };
    return builder.ReferenceExisting(Span(kCommands));
}

} // namespace chip::app::Clusters
