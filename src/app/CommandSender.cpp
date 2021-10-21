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
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {

CommandSender::CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr) :
    mpCallback(apCallback), mpExchangeMgr(apExchangeMgr)
{}

CHIP_ERROR CommandSender::SendCommandRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> secureSession,
                                             uint32_t timeout)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle commandPacket;

    VerifyOrExit(mState == CommandState::AddedCommand, err = CHIP_ERROR_INCORRECT_STATE);

    err = Finalize(commandPacket);
    SuccessOrExit(err);

    // Create a new exchange context.
    mpExchangeCtx = mpExchangeMgr->NewContext(secureSession.ValueOr(SessionHandle(aNodeId, 1, 1, aFabricIndex)), this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mpExchangeCtx->SetResponseTimeout(timeout);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest, std::move(commandPacket),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    MoveToState(CommandState::CommandSent);

exit:
    return err;
}

CHIP_ERROR CommandSender::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                            System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandResponse),
                 err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    SuccessOrExit(err = ProcessCommandMessage(std::move(aPayload), CommandRoleId::SenderId));

exit:
    if (mpCallback != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            mpCallback->OnError(this, Protocols::InteractionModel::Status::Failure, err);
        }
    }

    Close();

    return err;
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));

    if (mpCallback != nullptr)
    {
        mpCallback->OnError(this, Protocols::InteractionModel::Status::Failure, CHIP_ERROR_TIMEOUT);
    }

    Close();
}

void CommandSender::Close()
{
    MoveToState(CommandState::AwaitingDestruction);

    Command::Close();

    if (mpCallback)
    {
        mpCallback->OnDone(this);
    }
}

CHIP_ERROR CommandSender::ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    chip::EndpointId endpointId;

    {
        CommandPathIB::Parser commandPath;

        err = aCommandElement.GetCommandPath(&commandPath);
        SuccessOrExit(err);

        err = commandPath.GetClusterId(&clusterId);
        SuccessOrExit(err);

        err = commandPath.GetCommandId(&commandId);
        SuccessOrExit(err);

        err = commandPath.GetEndpointId(&endpointId);
        SuccessOrExit(err);
    }

    {
        bool hasDataResponse = false;
        chip::TLV::TLVReader commandDataReader;

        // Default to success when an invoke response is received.
        StatusIB statusIB;
        StatusIB::Parser statusIBParser;
        err = aCommandElement.GetStatusIB(&statusIBParser);
        if (CHIP_NO_ERROR == err)
        {
            err = statusIBParser.DecodeStatusIB(statusIB);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            hasDataResponse = true;
            err             = aCommandElement.GetData(&commandDataReader);
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "Received malformed Command Response, err=%" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            if (hasDataResponse)
            {
                ChipLogProgress(DataManagement,
                                "Received Command Response Data, Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                                " Command=" ChipLogFormatMEI,
                                endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
            }
            else
            {
                ChipLogProgress(DataManagement,
                                "Received Command Response Status for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                                " Command=" ChipLogFormatMEI " Status=0x%" PRIx16,
                                endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId),
                                to_underlying(statusIB.mStatus));
            }
        }
        SuccessOrExit(err);

        if (mpCallback != nullptr)
        {
            if (statusIB.mStatus == Protocols::InteractionModel::Status::Success)
            {
                mpCallback->OnResponse(this, ConcreteCommandPath(endpointId, clusterId, commandId),
                                       hasDataResponse ? &commandDataReader : nullptr);
            }
            else
            {
                mpCallback->OnError(this, statusIB.mStatus, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
            }
        }
    }

exit:
    return err;
}

} // namespace app
} // namespace chip
