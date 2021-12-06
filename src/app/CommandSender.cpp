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
#include "StatusResponse.h"
#include <app/TimedRequest.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {

CommandSender::CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest) :
    mpCallback(apCallback), mpExchangeMgr(apExchangeMgr), mSuppressResponse(false), mTimedRequest(aIsTimedRequest)
{}

CHIP_ERROR CommandSender::AllocateBuffer()
{
    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrReturnError(!commandPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        ReturnErrorOnFailure(mInvokeRequestBuilder.Init(&mCommandMessageWriter));

        mInvokeRequestBuilder.SuppressResponse(mSuppressResponse).TimedRequest(mTimedRequest);
        ReturnErrorOnFailure(mInvokeRequestBuilder.GetError());

        mInvokeRequestBuilder.CreateInvokeRequests();
        ReturnErrorOnFailure(mInvokeRequestBuilder.GetError());

        mCommandIndex    = 0;
        mBufferAllocated = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::SendCommandRequest(SessionHandle session, System::Clock::Timeout timeout)
{
    VerifyOrReturnError(mState == CommandState::AddedCommand, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Finalize(mPendingInvokeData));

    // Create a new exchange context.
    mpExchangeCtx = mpExchangeMgr->NewContext(session, this);
    VerifyOrReturnError(mpExchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    mpExchangeCtx->SetResponseTimeout(timeout);

    if (mTimedInvokeTimeoutMs.HasValue())
    {
        ReturnErrorOnFailure(TimedRequest::Send(mpExchangeCtx, mTimedInvokeTimeoutMs.Value()));
        MoveToState(CommandState::AwaitingTimedStatus);
        return CHIP_NO_ERROR;
    }

    return SendInvokeRequest();
}

CHIP_ERROR CommandSender::SendInvokeRequest()
{
    using namespace Protocols::InteractionModel;
    using namespace Messaging;

    ReturnErrorOnFailure(mpExchangeCtx->SendMessage(MsgType::InvokeCommandRequest, std::move(mPendingInvokeData),
                                                    SendMessageFlags::kExpectResponse));
    MoveToState(CommandState::CommandSent);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                            System::PacketBufferHandle && aPayload)
{
    if (mState == CommandState::CommandSent)
    {
        MoveToState(CommandState::ResponseReceived);
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB status(Protocols::InteractionModel::Status::Failure);
    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);

    if (mState == CommandState::AwaitingTimedStatus)
    {
        err = HandleTimedStatus(aPayloadHeader, std::move(aPayload), status);
        // Skip all other processing here (which is for the response to the
        // invoke request), no matter whether err is success or not.
        goto exit;
    }

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandResponse))
    {
        err = ProcessInvokeResponse(std::move(aPayload));
        SuccessOrExit(err);
        status.mStatus = Protocols::InteractionModel::Status::Success;
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        err = StatusResponse::ProcessStatusResponse(std::move(aPayload), status);
        SuccessOrExit(err);
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    if (mpCallback != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            mpCallback->OnError(this, status, err);
        }
    }

    if (mState != CommandState::CommandSent)
    {
        Close();
    }
    // Else we got a response to a Timed Request and just sent the invoke.

    return err;
}

CHIP_ERROR CommandSender::ProcessInvokeResponse(System::PacketBufferHandle && payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader invokeResponsesReader;
    InvokeResponseMessage::Parser invokeResponseMessage;
    InvokeResponseIBs::Parser invokeResponses;
    bool suppressResponse = false;

    reader.Init(std::move(payload));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(invokeResponseMessage.Init(reader));

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(invokeResponseMessage.CheckSchemaValidity());
#endif

    ReturnErrorOnFailure(invokeResponseMessage.GetSuppressResponse(&suppressResponse));
    ReturnErrorOnFailure(invokeResponseMessage.GetInvokeResponses(&invokeResponses));
    invokeResponses.GetReader(&invokeResponsesReader);

    while (CHIP_NO_ERROR == (err = invokeResponsesReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag == invokeResponsesReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        InvokeResponseIB::Parser invokeResponse;
        ReturnErrorOnFailure(invokeResponse.Init(invokeResponsesReader));
        ReturnErrorOnFailure(ProcessInvokeResponseIB(invokeResponse));
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
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

CHIP_ERROR CommandSender::ProcessInvokeResponseIB(InvokeResponseIB::Parser & aInvokeResponse)
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
            ReturnErrorOnFailure(commandStatus.GetPath(&commandPath));
            ReturnErrorOnFailure(commandPath.GetClusterId(&clusterId));
            ReturnErrorOnFailure(commandPath.GetCommandId(&commandId));
            ReturnErrorOnFailure(commandPath.GetEndpointId(&endpointId));

            StatusIB::Parser status;
            commandStatus.GetErrorStatus(&status);
            ReturnErrorOnFailure(status.DecodeStatusIB(statusIB));
        }
        else if (CHIP_END_OF_TLV == err)
        {
            CommandDataIB::Parser commandData;
            CommandPathIB::Parser commandPath;
            ReturnErrorOnFailure(aInvokeResponse.GetCommand(&commandData));
            ReturnErrorOnFailure(commandData.GetPath(&commandPath));
            ReturnErrorOnFailure(commandPath.GetEndpointId(&endpointId));
            ReturnErrorOnFailure(commandPath.GetClusterId(&clusterId));
            ReturnErrorOnFailure(commandPath.GetCommandId(&commandId));
            commandData.GetData(&commandDataReader);
            err             = CHIP_NO_ERROR;
            hasDataResponse = true;
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
        ReturnErrorOnFailure(err);

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
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct)
{
    ReturnLogErrorOnFailure(AllocateBuffer());

    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == CommandState::Idle, CHIP_ERROR_INCORRECT_STATE);
    InvokeRequests::Builder & invokeRequests = mInvokeRequestBuilder.GetInvokeRequests();
    CommandDataIB::Builder & invokeRequest   = invokeRequests.CreateCommandData();
    ReturnErrorOnFailure(invokeRequests.GetError());
    CommandPathIB::Builder & path = invokeRequest.CreatePath();
    ReturnErrorOnFailure(invokeRequest.GetError());
    ReturnErrorOnFailure(path.Encode(aCommandPathParams));

    if (aStartDataStruct)
    {
        ReturnLogErrorOnFailure(invokeRequest.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kData)),
                                                                          TLV::kTLVType_Structure, mDataElementContainerType));
    }

    MoveToState(CommandState::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::FinishCommand(bool aEndDataStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == CommandState::AddingCommand, err = CHIP_ERROR_INCORRECT_STATE);

    CommandDataIB::Builder commandData = mInvokeRequestBuilder.GetInvokeRequests().GetCommandData();

    if (aEndDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->EndContainer(mDataElementContainerType));
    }

    ReturnErrorOnFailure(commandData.EndOfCommandDataIB().GetError());
    ReturnErrorOnFailure(mInvokeRequestBuilder.GetInvokeRequests().EndOfInvokeRequests().GetError());
    ReturnErrorOnFailure(mInvokeRequestBuilder.EndOfInvokeRequestMessage().GetError());

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
        return mInvokeRequestBuilder.GetInvokeRequests().GetCommandData().GetWriter();
    }
}

CHIP_ERROR CommandSender::HandleTimedStatus(const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload,
                                            StatusIB & aStatusIB)
{
    ReturnErrorOnFailure(TimedRequest::HandleResponse(aPayloadHeader, std::move(aPayload), aStatusIB));

    return SendInvokeRequest();
}

CHIP_ERROR CommandSender::FinishCommand(const Optional<uint16_t> & aTimedInvokeTimeoutMs)
{
    ReturnErrorOnFailure(FinishCommand(/* aEndDataStruct = */ false));
    if (!mTimedInvokeTimeoutMs.HasValue())
    {
        mTimedInvokeTimeoutMs = aTimedInvokeTimeoutMs;
    }
    else if (aTimedInvokeTimeoutMs.HasValue())
    {
        uint16_t newValue = std::min(mTimedInvokeTimeoutMs.Value(), aTimedInvokeTimeoutMs.Value());
        mTimedInvokeTimeoutMs.SetValue(newValue);
    }
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
