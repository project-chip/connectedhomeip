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

CHIP_ERROR CommandSender::SendCommandRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> secureSession,
                                             uint32_t timeout)
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
    mpExchangeCtx = mpExchangeMgr->NewContext(secureSession.ValueOr(SessionHandle(aNodeId, 0, 0, aFabricIndex)), this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(timeout);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest, std::move(commandPacket),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(CommandState::Sending);

exit:
    if (err != CHIP_NO_ERROR)
    {
        AbortExistingExchangeContext();
    }

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
            mpCallback->OnError(this, Protocols::InteractionModel::ProtocolCode::Failure, err);
        }
    }

    ShutdownInternal();
    return err;
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));

    if (mpCallback != nullptr)
    {
        mpCallback->OnError(this, Protocols::InteractionModel::ProtocolCode::Failure, CHIP_ERROR_TIMEOUT);
    }

    ShutdownInternal();
}

CHIP_ERROR CommandSender::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mCommandIndex++;

    CommandPath::Type commandPath;
    {
        TLV::TLVReader commandPathReader;
        SuccessOrExit(
            err = aCommandElement.GetReaderOnTag(TLV::ContextTag(CommandDataElement::kCsTag_CommandPath), &commandPathReader));
        SuccessOrExit(err = commandPath.Decode(commandPathReader));
    }

    {
        bool hasCommandSpecificResponse = false;
        chip::TLV::TLVReader commandDataReader;

        // Default to success when command specify response is received.
        StatusElement::Type statusElement{ chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                           chip::Protocols::InteractionModel::Id.ToFullyQualifiedSpecForm(),
                                           to_underlying(Protocols::InteractionModel::ProtocolCode::Success) };
        StatusElement::Parser statusElementParser;
        err = aCommandElement.GetStatusElement(&statusElementParser);
        if (CHIP_NO_ERROR == err)
        {
            err = statusElementParser.DecodeStatusElement(statusElement);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            hasCommandSpecificResponse = true;
            err                        = aCommandElement.GetData(&commandDataReader);
        }
        SuccessOrExit(err);

        if (mpCallback != nullptr)
        {
            if (statusElement.protocolCode == to_underlying(Protocols::InteractionModel::ProtocolCode::Success))
            {
                mpCallback->OnResponse(this, commandPath, hasCommandSpecificResponse ? &commandDataReader : nullptr);
            }
            else
            {
                mpCallback->OnError(this, static_cast<Protocols::InteractionModel::ProtocolCode>(statusElement.protocolCode),
                                    CHIP_NO_ERROR);
            }
        }
    }

exit:
    if (err != CHIP_NO_ERROR && mpCallback != nullptr)
    {
        mpCallback->OnError(this, Protocols::InteractionModel::ProtocolCode::Failure, err);
    }
    return err;
}

void CommandSender::ShutdownInternal()
{
    // For CommandSender, ExchangeContext is the only thing it holds ownership by pointer.
    AbortExistingExchangeContext();
    if (mpCallback != nullptr)
    {
        mpCallback->OnFinal(this);
    }
}

} // namespace app
} // namespace chip
