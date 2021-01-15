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

namespace chip {
namespace app {
void CommandHandler::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                       uint8_t msgType, System::PacketBufferHandle payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.

    mpExchangeCtx = ec;

    err = ProcessCommandMessage(std::move(payload), kCommandHandlerId);
    SuccessOrExit(err);

    SendCommandResponse();

exit:
    ChipLogFunctError(err);
}

CHIP_ERROR CommandHandler::SendCommandResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = FinalizeCommandsMessage();
    SuccessOrExit(err);

    VerifyOrExit(mpExchangeCtx != NULL, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::kProtocol_InteractionModel, kMsgType_InvokeCommandResponse,
                                     std::move(mCommandMessageBuf), Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
    SuccessOrExit(err);

    MoveToState(kState_Sending);

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

    err = aCommandElement.GetCommandPath(&commandPath);
    SuccessOrExit(err);

    err = commandPath.GetNamespacedClusterId(&clusterId);
    SuccessOrExit(err);

    err = commandPath.GetCommandId(&commandId);
    SuccessOrExit(err);

    err = aCommandElement.GetData(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        // Empty Command, Add status code in invoke command response, notify cluster handler to hand it further.
        err = CHIP_NO_ERROR;
        ChipLogDetail(DataManagement, "Add Status code for empty command, cluster Id is %d", clusterId);
        // Todo: Define ProtocolCode for StatusCode.
        AddStatusCode(COMMON_STATUS_SUCCESS, chip::Protocols::kProtocol_Protocol_Common, 0, clusterId);
    }
    else if (CHIP_NO_ERROR == err)
    {
        InteractionModelEngine::GetInstance()->ProcessCommand(clusterId, commandId, commandDataReader, this, kCommandHandlerId);
    }

exit:
    return err;
}
} // namespace app
} // namespace chip
