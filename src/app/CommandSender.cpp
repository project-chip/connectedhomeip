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
    mpCallback(apCallback), mpExchangeMgr(apExchangeMgr), mSuppressResponse(false), mTimedRequest(false)
{}

CHIP_ERROR CommandSender::AllocateBuffer()
{
    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrReturnError(!commandPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        ReturnErrorOnFailure(mInvokeRequestMessage.Init(&mCommandMessageWriter));

        mInvokeRequestMessage.SuppressResponse(mSuppressResponse).TimedRequest(mTimedRequest);
        ReturnErrorOnFailure(mInvokeRequestMessage.GetError());

        mInvokeRequestMessage.CreateInvokeRequests();
        ReturnErrorOnFailure(mInvokeRequestMessage.GetError());

        mCommandIndex    = 0;
        mBufferAllocated = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::SendCommandRequest(SessionHandle session, System::Clock::Timeout timeout)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle commandPacket;

    VerifyOrExit(mState == CommandState::AddedCommand, err = CHIP_ERROR_INCORRECT_STATE);

    err = Finalize(commandPacket);
    SuccessOrExit(err);

    // Create a new exchange context.
    mpExchangeCtx = mpExchangeMgr->NewContext(session, this);
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

    err = ProcessInvokeResponseMessage(std::move(aPayload));

exit:
    if (mpCallback != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            StatusIB status;
            status.mStatus = Protocols::InteractionModel::Status::Failure;
            mpCallback->OnError(this, status, err);
        }
    }

    Close();

    return err;
}

CHIP_ERROR CommandSender::ProcessInvokeResponseMessage(System::PacketBufferHandle && payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader invokeResponsesReader;
    InvokeResponseMessage::Parser invokeResponseMessage;
    InvokeResponses::Parser invokeResponses;
    bool suppressResponse = false;

    reader.Init(std::move(payload));
    err = reader.Next();
    SuccessOrExit(err);

    err = invokeResponseMessage.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeResponseMessage.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = invokeResponseMessage.GetSuppressResponse(&suppressResponse);
    SuccessOrExit(err);

    err = invokeResponseMessage.GetInvokeResponses(&invokeResponses);
    SuccessOrExit(err);

    invokeResponses.GetReader(&invokeResponsesReader);

    while (CHIP_NO_ERROR == (err = invokeResponsesReader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == invokeResponsesReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        InvokeResponseIB::Parser invokeResponse;

        err = invokeResponse.Init(invokeResponsesReader);
        SuccessOrExit(err);

        err = ProcessInvokeResponse(invokeResponse);
        SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));

    if (mpCallback != nullptr)
    {
        StatusIB status;
        status.mStatus = Protocols::InteractionModel::Status::Failure;
        mpCallback->OnError(this, status, CHIP_ERROR_TIMEOUT);
    }

    Close();
}

void CommandSender::Close()
{
    mSuppressResponse = false;
    mTimedRequest     = false;
    MoveToState(CommandState::AwaitingDestruction);

    Command::Close();

    if (mpCallback)
    {
        mpCallback->OnDone(this);
    }
}

CHIP_ERROR CommandSender::ProcessInvokeResponse(InvokeResponseIB::Parser & aInvokeResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ClusterId clusterId;
    CommandId commandId;
    EndpointId endpointId;
    // Default to success when an invoke response is received.
    StatusIB statusIB;

    {
        bool hasDataResponse = false;
        TLV::TLVReader commandDataReader;

        CommandStatusIB::Parser commandStatus;
        err = aInvokeResponse.GetStatus(&commandStatus);
        if (CHIP_NO_ERROR == err)
        {
            CommandPathIB::Parser commandPath;
            commandStatus.GetPath(&commandPath);
            err = commandPath.GetClusterId(&clusterId);
            SuccessOrExit(err);

            err = commandPath.GetCommandId(&commandId);
            SuccessOrExit(err);

            err = commandPath.GetEndpointId(&endpointId);
            SuccessOrExit(err);

            StatusIB::Parser status;
            commandStatus.GetErrorStatus(&status);
            err = status.DecodeStatusIB(statusIB);
            SuccessOrExit(err);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            hasDataResponse = true;

            CommandDataIB::Parser commandData;
            CommandPathIB::Parser commandPath;
            err = aInvokeResponse.GetCommand(&commandData);
            SuccessOrExit(err);
            commandData.GetPath(&commandPath);

            err = commandPath.GetClusterId(&clusterId);
            SuccessOrExit(err);

            err = commandPath.GetCommandId(&commandId);
            SuccessOrExit(err);

            err = commandPath.GetEndpointId(&endpointId);
            SuccessOrExit(err);

            commandData.GetData(&commandDataReader);
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
                mpCallback->OnResponse(this, ConcreteCommandPath(endpointId, clusterId, commandId), statusIB,
                                       hasDataResponse ? &commandDataReader : nullptr);
            }
            else
            {
                mpCallback->OnError(this, statusIB, CHIP_ERROR_IM_STATUS_CODE_RECEIVED);
            }
        }
    }

exit:
    return err;
}

CHIP_ERROR CommandSender::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Builder commandData;

    err = AllocateBuffer();
    SuccessOrExit(err);

    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrExit(mState == CommandState::Idle, err = CHIP_ERROR_INCORRECT_STATE);

    commandData = mInvokeRequestMessage.GetInvokeRequests().CreateCommandData();
    err         = commandData.GetError();
    SuccessOrExit(err);

    err = ConstructCommandPath(aCommandPathParams, commandData.CreatePath());
    SuccessOrExit(err);

    if (aStartDataStruct)
    {
        err = commandData.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kData)),
                                                      TLV::kTLVType_Structure, mDataElementContainerType);
    }

    MoveToState(CommandState::AddingCommand);

exit:
    return err;
}

CHIP_ERROR CommandSender::FinishCommand(bool aEndDataStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == CommandState::AddingCommand, err = CHIP_ERROR_INCORRECT_STATE);

    CommandDataIB::Builder commandData = mInvokeRequestMessage.GetInvokeRequests().GetCommandData();

    if (aEndDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->EndContainer(mDataElementContainerType));
    }

    ReturnErrorOnFailure(commandData.EndOfCommandDataIB().GetError());
    ReturnErrorOnFailure(mInvokeRequestMessage.GetInvokeRequests().EndOfInvokeRequests().GetError());
    ReturnErrorOnFailure(mInvokeRequestMessage.EndOfInvokeRequestMessage().GetError());

    MoveToState(CommandState::AddedCommand);

    return CHIP_NO_ERROR;
}

TLV::TLVWriter * CommandSender::GetCommandDataIBTLVWriter()
{
    if (mState != CommandState::AddingCommand)
    {
        return nullptr;
    }
    else
    {
        return mInvokeRequestMessage.GetInvokeRequests().GetCommandData().GetWriter();
    }
}

} // namespace app
} // namespace chip
