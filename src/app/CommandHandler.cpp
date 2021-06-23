/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#include "CommandHandler.h"
#include "Command.h"
#include "CommandSender.h"
#include "InteractionModelEngine.h"

#include <protocols/secure_channel/Constants.h>

using GeneralStatusCode = chip::Protocols::SecureChannel::GeneralStatusCode;

namespace chip {
namespace app {
CHIP_ERROR CommandHandler::OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                                  const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.

    mpExchangeCtx = ec;

    err = ProcessCommandMessage(std::move(payload), CommandRoleId::HandlerId);
    SuccessOrExit(err);

    err = SendCommandResponse();

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR CommandHandler::SendCommandResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle commandPacket;

    VerifyOrExit(mState == CommandState::AddCommand, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeCommandsMessage(commandPacket);
    SuccessOrExit(err);

    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse, std::move(commandPacket));
    SuccessOrExit(err);

    MoveToState(CommandState::Sending);

exit:
    Shutdown();
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR CommandHandler::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPath;
    chip::TLV::TLVReader commandDataReader;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    chip::EndpointId endpointId;

    err = aCommandElement.GetCommandPath(&commandPath);
    SuccessOrExit(err);
    err = commandPath.GetClusterId(&clusterId);
    SuccessOrExit(err);
    err = commandPath.GetCommandId(&commandId);
    SuccessOrExit(err);
    err = commandPath.GetEndpointId(&endpointId);
    SuccessOrExit(err);

    VerifyOrExit(ServerClusterCommandExists(clusterId, commandId, endpointId), err = CHIP_ERROR_INVALID_PROFILE_ID);

    err = aCommandElement.GetData(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
        ChipLogDetail(DataManagement, "Received command without data for cluster %" PRIx32, clusterId);
    }
    if (CHIP_NO_ERROR == err)
    {
        DispatchSingleClusterCommand(clusterId, commandId, endpointId, commandDataReader, this);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::app::CommandPathParams returnStatusParam = { endpointId,
                                                           0, // GroupId
                                                           clusterId, commandId, (chip::app::CommandPathFlags::kEndpointIdValid) };

        // The Path is the path in the request if there are any error occurred before we dispatch the command to clusters.
        // Currently, it could be failed to decode Path or failed to find cluster / command on desired endpoint.
        // TODO: The behavior when receiving a malformed message is not clear in the Spec. (Spec#3259)
        // TODO: The error code should be updated after #7072 added error codes required by IM.
        if (err == CHIP_ERROR_INVALID_PROFILE_ID)
        {
            ChipLogDetail(DataManagement, "No Cluster 0x%" PRIx32 " on Endpoint 0x%" PRIx16, clusterId, endpointId);
        }

        AddStatusCode(returnStatusParam,
                      err == CHIP_ERROR_INVALID_PROFILE_ID ? GeneralStatusCode::kNotFound : GeneralStatusCode::kInvalidArgument,
                      Protocols::InteractionModel::Id, Protocols::InteractionModel::ProtocolCode::InvalidCommand);
    }
    // We have handled the error status above and put the error status in response, now return success status so we can process
    // other commands in the invoke request.
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::AddStatusCode(const CommandPathParams & aCommandPathParams,
                                         const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                         const Protocols::Id aProtocolId,
                                         const Protocols::InteractionModel::ProtocolCode aProtocolCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Builder statusElementBuilder;

    err = PrepareCommand(aCommandPathParams, true /* isStatus */);
    SuccessOrExit(err);

    statusElementBuilder =
        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuilder().CreateStatusElementBuilder();
    statusElementBuilder
        .EncodeStatusElement(aGeneralCode, aProtocolId.ToFullyQualifiedSpecForm(),
                             Protocols::InteractionModel::ToUint16(aProtocolCode))
        .EndOfStatusElement();
    err = statusElementBuilder.GetError();
    SuccessOrExit(err);

    err = FinishCommand(true /* isStatus */);

exit:
    ChipLogFunctError(err);
    return err;
}

} // namespace app
} // namespace chip
