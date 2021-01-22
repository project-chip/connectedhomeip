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

namespace chip {
namespace app {

CHIP_ERROR CommandSender::SendCommandRequest(NodeId aNodeId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = FinalizeCommandsMessage();
    SuccessOrExit(err);

    ClearExistingExchangeContext();

    // Create a new exchange context.
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, Transport::kAnyKeyId }, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(CHIP_INVOKE_COMMAND_RSP_TIMEOUT);

    err = mpExchangeCtx->SendMessage(Protocols::kProtocol_InteractionModel, kMsgType_InvokeCommandRequest,
                                     std::move(mCommandMessageBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(kState_Sending);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ClearExistingExchangeContext();
    }
    ChipLogFunctError(err);

    return err;
}

void CommandSender::OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader, uint32_t aProtocolId,
                                      uint8_t aMsgType, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendCommandRequest is called
    // back-to-back, the second call will call Close() on the first exchange,
    // which clears the OnMessageReceived callback.

    VerifyOrDie(apEc == mpExchangeCtx);

    // Verify that the message is an Invoke Command Response.
    // If not, close the exchange and free the payload.
    if (aProtocolId != Protocols::kProtocol_InteractionModel || aMsgType != kMsgType_InvokeCommandResponse)
    {
        apEc->Close();
        mpExchangeCtx = nullptr;
        goto exit;
    }

    // Remove the EC from the app state now. OnMessageReceived can call
    // SendCommandRequest and install a new one. We abort rather than close
    // because we no longer care whether the echo request message has been
    // acknowledged at the transport layer.
    ClearExistingExchangeContext();

    err = ProcessCommandMessage(std::move(aPayload), kCommandSenderId);
    SuccessOrExit(err);

exit:
    Reset();
    return;
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apEc)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: %d", apEc->GetExchangeId());
    Reset();
}

CHIP_ERROR CommandSender::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPath;
    chip::TLV::TLVReader commandDataReader;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    uint16_t generalCode  = 0;
    uint32_t protocolId   = 0;
    uint16_t protocolCode = 0;
    StatusElement::Parser statusElementParser;

    err = aCommandElement.GetStatusElement(&statusElementParser);
    if (CHIP_NO_ERROR == err)
    {
        // Response has status element since either there is error in command response or it is empty response
        err = statusElementParser.CheckSchemaValidity();
        SuccessOrExit(err);

        err = statusElementParser.DecodeStatusElement(&generalCode, &protocolId, &protocolCode, &clusterId);
        SuccessOrExit(err);
    }
    else if (CHIP_END_OF_TLV == err)
    {
        err = aCommandElement.GetCommandPath(&commandPath);
        SuccessOrExit(err);

        err = commandPath.GetNamespacedClusterId(&clusterId);
        SuccessOrExit(err);

        err = commandPath.GetCommandId(&commandId);
        SuccessOrExit(err);

        err = aCommandElement.GetData(&commandDataReader);
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
            ChipLogDetail(DataManagement, "Add Status code for empty command, cluster Id is %d", clusterId);
            // Todo: Define protocol code for StatusCode
            AddStatusCode(0, chip::Protocols::kProtocol_Protocol_Common, 0, clusterId);
        }
        InteractionModelEngine::GetInstance()->ProcessCommand(clusterId, commandId, commandDataReader, this, kCommandSenderId);
    }

exit:
    return err;
}

} // namespace app
} // namespace chip
