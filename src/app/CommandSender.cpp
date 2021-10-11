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
#include "protocols/Protocols.h"
#include "protocols/interaction_model/Constants.h"

#include <protocols/secure_channel/Constants.h>

using GeneralStatusCode = chip::Protocols::SecureChannel::GeneralStatusCode;

namespace chip {
namespace app {

CommandSender::CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr) :
    Command(apExchangeMgr), mpCallback(apCallback)
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
    mpExchangeCtx = mpExchangeMgr->NewContext(secureSession.ValueOr(SessionHandle(aNodeId, 0, 0, aFabricIndex)), this);
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

CHIP_ERROR CommandSender::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    chip::EndpointId endpointId;

    {
        CommandPath::Parser commandPath;

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
        StatusIB::Type StatusIB{ chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                           chip::Protocols::InteractionModel::Id.ToFullyQualifiedSpecForm(),
                                           to_underlying(Protocols::InteractionModel::Status::Success) };
        StatusIB::Parser StatusIBParser;
        err = aCommandElement.GetStatusIB(&StatusIBParser);
        if (CHIP_NO_ERROR == err)
        {
            err = StatusIBParser.DecodeStatusIB(StatusIB);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            hasDataResponse = true;
            err             = aCommandElement.GetData(&commandDataReader);
        }
        SuccessOrExit(err);

        if (mpCallback != nullptr)
        {
            if (StatusIB.protocolId == Protocols::InteractionModel::Id.ToFullyQualifiedSpecForm())
            {
                if (StatusIB.protocolCode == to_underlying(Protocols::InteractionModel::Status::Success))
                {
                    mpCallback->OnResponse(this, ConcreteCommandPath(endpointId, clusterId, commandId),
                                           hasDataResponse ? &commandDataReader : nullptr);
                }
                else
                {
                    mpCallback->OnError(this, static_cast<Protocols::InteractionModel::Status>(StatusIB.protocolCode),
                                        CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
                }
            }
            else
            {
                mpCallback->OnError(this, Protocols::InteractionModel::Status::Failure, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
            }
        }
    }

exit:
    return err;
}

} // namespace app
} // namespace chip
