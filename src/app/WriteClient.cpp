/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the initiator side of a CHIP Write Interaction.
 *
 */

#include "lib/core/CHIPError.h"
#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>

namespace chip {
namespace app {

CHIP_ERROR WriteClient::Init(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback)
{
    VerifyOrReturnError(apExchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);

    AttributeDataIBs::Builder attributeDataIBsBuilder;
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    mMessageWriter.Init(std::move(packet));

    ReturnErrorOnFailure(mWriteRequestBuilder.Init(&mMessageWriter));
    mWriteRequestBuilder.TimedRequest(false);
    ReturnErrorOnFailure(mWriteRequestBuilder.GetError());
    attributeDataIBsBuilder = mWriteRequestBuilder.CreateWriteRequests();
    ReturnErrorOnFailure(attributeDataIBsBuilder.GetError());
    ClearExistingExchangeContext();
    mpExchangeMgr         = apExchangeMgr;
    mpCallback            = apCallback;
    mAttributeStatusIndex = 0;
    MoveToState(State::Initialized);

    return CHIP_NO_ERROR;
}

void WriteClient::Shutdown()
{
    VerifyOrReturn(mState != State::Uninitialized);
    ClearExistingExchangeContext();
    ShutdownInternal();
}

void WriteClient::ShutdownInternal()
{
    mMessageWriter.Reset();

    mpExchangeMgr         = nullptr;
    mpExchangeCtx         = nullptr;
    mAttributeStatusIndex = 0;
    ClearState();

    mpCallback->OnDone(this);
}

void WriteClient::ClearExistingExchangeContext()
{
    // Discard any existing exchange context. Effectively we can only have one IM exchange with
    // a single node at any one time.
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
}

CHIP_ERROR WriteClient::ProcessWriteResponseMessage(System::PacketBufferHandle && payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader attributeStatusesReader;
    WriteResponseMessage::Parser writeResponse;
    AttributeStatusIBs::Parser attributeStatusesParser;

    reader.Init(std::move(payload));
    err = reader.Next();
    SuccessOrExit(err);

    err = writeResponse.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeResponse.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = writeResponse.GetWriteResponses(&attributeStatusesParser);
    SuccessOrExit(err);

    attributeStatusesParser.GetReader(&attributeStatusesReader);

    while (CHIP_NO_ERROR == (err = attributeStatusesReader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == attributeStatusesReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        AttributeStatusIB::Parser element;

        err = element.Init(attributeStatusesReader);
        SuccessOrExit(err);

        err = ProcessAttributeStatusIB(element);
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

CHIP_ERROR WriteClient::PrepareAttribute(const AttributePathParams & attributePathParams)
{
    VerifyOrReturnError(attributePathParams.IsValidAttributePath(), CHIP_ERROR_INVALID_PATH_LIST);
    AttributeDataIB::Builder attributeDataIB = mWriteRequestBuilder.GetWriteRequests().CreateAttributeDataIBBuilder();
    ReturnErrorOnFailure(attributeDataIB.GetError());
    // TODO: Add attribute version support
    attributeDataIB.DataVersion(0);
    ReturnErrorOnFailure(attributeDataIB.GetError());
    ReturnErrorOnFailure(attributeDataIB.CreatePath().Encode(attributePathParams));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteClient::FinishAttribute()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributeDataIB::Builder AttributeDataIB = mWriteRequestBuilder.GetWriteRequests().GetAttributeDataIBBuilder();
    AttributeDataIB.EndOfAttributeDataIB();
    SuccessOrExit(err = AttributeDataIB.GetError());
    MoveToState(State::AddAttribute);

exit:
    return err;
}

TLV::TLVWriter * WriteClient::GetAttributeDataIBTLVWriter()
{
    return mWriteRequestBuilder.GetWriteRequests().GetAttributeDataIBBuilder().GetWriter();
}

CHIP_ERROR WriteClient::FinalizeMessage(System::PacketBufferHandle & aPacket)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataIBs::Builder AttributeDataIBsBuilder;
    VerifyOrExit(mState == State::AddAttribute, err = CHIP_ERROR_INCORRECT_STATE);
    AttributeDataIBsBuilder = mWriteRequestBuilder.GetWriteRequests().EndOfAttributeDataIBs();
    err                     = AttributeDataIBsBuilder.GetError();
    SuccessOrExit(err);

    mWriteRequestBuilder.IsFabricFiltered(false).EndOfWriteRequestMessage();
    err = mWriteRequestBuilder.GetError();
    SuccessOrExit(err);

    err = mMessageWriter.Finalize(&aPacket);
    SuccessOrExit(err);

exit:
    return err;
}

const char * WriteClient::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Uninitialized:
        return "Uninitialized";

    case State::Initialized:
        return "Initialized";

    case State::AddAttribute:
        return "AddAttribute";

    case State::AwaitingResponse:
        return "AwaitingResponse";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void WriteClient::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "WriteClient moving to [%10.10s]", GetStateStr());
}

void WriteClient::ClearState()
{
    MoveToState(State::Uninitialized);
}

CHIP_ERROR WriteClient::SendWriteRequest(SessionHandle session, System::Clock::Timeout timeout)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle packet;

    VerifyOrExit(mState == State::AddAttribute, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeMessage(packet);
    SuccessOrExit(err);

    // Discard any existing exchange context. Effectively we can only have one exchange per WriteClient
    // at any one time.
    ClearExistingExchangeContext();

    // Create a new exchange context.
    mpExchangeCtx = mpExchangeMgr->NewContext(session, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mpExchangeCtx->SetResponseTimeout(timeout);

    // kExpectResponse is ignored by ExchangeContext in case of groupcast
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::WriteRequest, std::move(packet),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    MoveToState(State::AwaitingResponse);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Write client failed to SendWriteRequest");
        ClearExistingExchangeContext();
    }

    if (session.IsGroupSession())
    {
        // Always shutdown on Group communication
        ChipLogDetail(DataManagement, "Closing on group Communication ");

        // onDone is called
        ShutdownInternal();
    }

    return err;
}

CHIP_ERROR WriteClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                          System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendWriteRequest is called
    // back-to-back, the second call will call Close() on the first exchange,
    // which clears the OnMessageReceived callback.
    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteResponse))
    {
        err = ProcessWriteResponseMessage(std::move(aPayload));
        SuccessOrExit(err);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        StatusIB status;
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
            mpCallback->OnError(this, err);
        }
    }
    ShutdownInternal();
    return err;
}

void WriteClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive write response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));

    if (mpCallback != nullptr)
    {
        mpCallback->OnError(this, CHIP_ERROR_TIMEOUT);
    }
    ShutdownInternal();
}

CHIP_ERROR WriteClient::ProcessAttributeStatusIB(AttributeStatusIB::Parser & aAttributeStatusIB)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Parser attributePath;
    StatusIB statusIB;
    StatusIB::Parser StatusIBParser;
    AttributePathParams attributePathParams;

    mAttributeStatusIndex++;
    err = aAttributeStatusIB.GetPath(&attributePath);
    SuccessOrExit(err);
    err = attributePath.GetCluster(&(attributePathParams.mClusterId));
    SuccessOrExit(err);
    err = attributePath.GetEndpoint(&(attributePathParams.mEndpointId));
    SuccessOrExit(err);
    err = attributePath.GetAttribute(&(attributePathParams.mAttributeId));
    SuccessOrExit(err);
    err = attributePath.GetListIndex(&(attributePathParams.mListIndex));
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    // TODO: (#11423) Attribute paths has a pattern of invalid paths, should add a function for checking invalid paths here.
    // NOTE: We don't support wildcard write for now, reject all wildcard paths.
    VerifyOrExit(!attributePathParams.HasAttributeWildcard() && attributePathParams.IsValidAttributePath(),
                 err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    err = aAttributeStatusIB.GetErrorStatus(&(StatusIBParser));
    if (CHIP_NO_ERROR == err)
    {
        err = StatusIBParser.DecodeStatusIB(statusIB);
        SuccessOrExit(err);
        if (mpCallback != nullptr)
        {
            ConcreteAttributePath path(attributePathParams.mEndpointId, attributePathParams.mClusterId,
                                       attributePathParams.mAttributeId);
            mpCallback->OnResponse(this, path, statusIB);
        }
    }

exit:
    return err;
}

CHIP_ERROR WriteClientHandle::SendWriteRequest(SessionHandle session, System::Clock::Timeout timeout)
{
    CHIP_ERROR err = mpWriteClient->SendWriteRequest(session, timeout);

    // Transferring ownership of the underlying WriteClient to the IM layer. IM will manage its lifetime.
    // For groupcast writes, there is no transfer of ownership since the interaction is done upon transmission of the action
    if (err == CHIP_NO_ERROR)
    {
        // Release the WriteClient without closing it.
        mpWriteClient = nullptr;
    }
    else
    {
        SetWriteClient(nullptr);
    }
    return err;
}

} // namespace app
} // namespace chip
