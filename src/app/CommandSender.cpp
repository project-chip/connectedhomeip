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
 *      This file defines objects for a CHIP IM Invoke Command Sender
 *
 */

#include "CommandSender.h"
#include "Command.h"
#include "CommandHandler.h"
#include "InteractionModelEngine.h"

#include <protocols/secure_channel/Constants.h>

using GeneralStatusCode = chip::Protocols::SecureChannel::GeneralStatusCode;

namespace chip {
namespace app {

CHIP_ERROR CommandSender::SendCommandRequest(NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * secureSession)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle commandPacket;

    VerifyOrExit(mState == CommandState::AddCommand, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeCommandsMessage(commandPacket);
    SuccessOrExit(err);

    // Discard any existing exchange context. Effectively we can only have one exchange per CommandSender
    // at any one time.
    AbortExistingExchangeContext();

    // Create a new exchange context.
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    // TODO: Hard code keyID to 0 to unblock IM end-to-end test. Complete solution is tracked in issue:4451
    if (secureSession == nullptr)
    {
        mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0, aAdminId }, this);
    }
    else
    {
        mpExchangeCtx = mpExchangeMgr->NewContext(*secureSession, this);
    }
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(kImMessageTimeoutMsec);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest, std::move(commandPacket),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(CommandState::Sending);

exit:
    if (err != CHIP_NO_ERROR)
    {
        AbortExistingExchangeContext();
    }
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR CommandSender::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                            const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandResponse),
                 err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    err = ProcessCommandMessage(std::move(aPayload), CommandRoleId::SenderId);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);

    // Null out mpExchangeCtx, so our Shutdown() call below won't try to abort
    // it and fail to send an ack for the message we just received.
    mpExchangeCtx = nullptr;

    if (mpDelegate != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            mpDelegate->CommandResponseError(this, err);
        }
        else
        {
            mpDelegate->CommandResponseProcessed(this);
        }
    }

    Shutdown();
    return err;
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: %d",
                    apExchangeContext->GetExchangeId());

    if (mpDelegate != nullptr)
    {
        mpDelegate->CommandResponseError(this, CHIP_ERROR_TIMEOUT);
    }

    Shutdown();
}

CHIP_ERROR CommandSender::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPath;
    chip::TLV::TLVReader commandDataReader;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    chip::EndpointId endpointId;
    Protocols::SecureChannel::GeneralStatusCode generalCode = Protocols::SecureChannel::GeneralStatusCode::kSuccess;
    uint32_t protocolId                                     = 0;
    uint16_t protocolCode                                   = 0;
    StatusElement::Parser statusElementParser;

    mCommandIndex++;
    err = aCommandElement.GetCommandPath(&commandPath);
    SuccessOrExit(err);

    err = commandPath.GetClusterId(&clusterId);
    SuccessOrExit(err);
    err = commandPath.GetCommandId(&commandId);
    SuccessOrExit(err);

    err = commandPath.GetEndpointId(&endpointId);
    SuccessOrExit(err);

    err = aCommandElement.GetStatusElement(&statusElementParser);
    if (CHIP_NO_ERROR == err)
    {
        err = statusElementParser.DecodeStatusElement(&generalCode, &protocolId, &protocolCode);
        SuccessOrExit(err);
        if (mpDelegate != nullptr)
        {
            mpDelegate->CommandResponseStatus(this, generalCode, protocolId, protocolCode, endpointId, clusterId, commandId,
                                              mCommandIndex);
        }
    }
    else if (CHIP_END_OF_TLV == err)
    {
        // TODO(Spec#3258): The endpoint id in response command is not clear, so we cannot do "ClientClusterCommandExists" check.
        err = aCommandElement.GetData(&commandDataReader);
        SuccessOrExit(err);
        // TODO(#4503): Should call callbacks of cluster that sends the command.
        DispatchSingleClusterCommand(clusterId, commandId, endpointId, commandDataReader, this);
    }

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR && mpDelegate != nullptr)
    {
        mpDelegate->CommandResponseProtocolError(this, mCommandIndex);
    }
    return err;
}

} // namespace app
} // namespace chip
