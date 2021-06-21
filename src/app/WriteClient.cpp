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

#include <app/InteractionModelEngine.h>
#include <app/WriteClient.h>

namespace chip {
namespace app {

CHIP_ERROR WriteClient::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    VerifyOrReturnError(apExchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);

    AttributeDataList::Builder attributeDataListBuilder;
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    mMessageWriter.Init(std::move(packet));

    ReturnErrorOnFailure(mWriteRequestBuilder.Init(&mMessageWriter));

    attributeDataListBuilder = mWriteRequestBuilder.CreateAttributeDataListBuilder();
    ReturnErrorOnFailure(attributeDataListBuilder.GetError());

    ClearExistingExchangeContext();
    mpExchangeMgr         = apExchangeMgr;
    mpDelegate            = apDelegate;
    mAttributeStatusIndex = 0;
    MoveToState(State::Initialized);

    return CHIP_NO_ERROR;
}

void WriteClient::Shutdown()
{
    VerifyOrReturn(mState != State::Uninitialized);
    mMessageWriter.Reset();

    ClearExistingExchangeContext();

    mpExchangeMgr         = nullptr;
    mpDelegate            = nullptr;
    mAttributeStatusIndex = 0;
    ClearState();
}

void WriteClient::ClearExistingExchangeContext()
{
    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
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
    TLV::TLVReader attributeStatusListReader;
    WriteResponse::Parser writeResponse;
    AttributeStatusList::Parser attributeStatusListParser;

    reader.Init(std::move(payload));
    err = reader.Next();
    SuccessOrExit(err);

    err = writeResponse.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeResponse.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = writeResponse.GetAttributeStatusList(&attributeStatusListParser);
    SuccessOrExit(err);

    attributeStatusListParser.GetReader(&attributeStatusListReader);

    while (CHIP_NO_ERROR == (err = attributeStatusListReader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == attributeStatusListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(TLV::kTLVType_Structure == attributeStatusListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        AttributeStatusElement::Parser element;

        err = element.Init(attributeStatusListReader);
        SuccessOrExit(err);

        err = ProcessAttributeStatusElement(element);
        SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteClient::PrepareAttribute(const AttributePathParams & attributePathParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributeDataElement::Builder attributeDataElement =
        mWriteRequestBuilder.GetAttributeDataListBuilder().CreateAttributeDataElementBuilder();
    SuccessOrExit(attributeDataElement.GetError());
    err = ConstructAttributePath(attributePathParams, attributeDataElement);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteClient::FinishAttribute()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributeDataElement::Builder attributeDataElement =
        mWriteRequestBuilder.GetAttributeDataListBuilder().GetAttributeDataElementBuilder();

    // TODO: Add attribute version support
    attributeDataElement.DataVersion(0);
    attributeDataElement.EndOfAttributeDataElement();
    SuccessOrExit(err = attributeDataElement.GetError());
    MoveToState(State::AddAttribute);

exit:
    ChipLogFunctError(err);
    return err;
}

TLV::TLVWriter * WriteClient::GetAttributeDataElementTLVWriter()
{
    return mWriteRequestBuilder.GetAttributeDataListBuilder().GetAttributeDataElementBuilder().GetWriter();
}

CHIP_ERROR WriteClient::ConstructAttributePath(const AttributePathParams & aAttributePathParams,
                                               AttributeDataElement::Builder aAttributeDataElement)
{
    AttributePath::Builder attributePath = aAttributeDataElement.CreateAttributePathBuilder();
    if (aAttributePathParams.mFlags.Has(AttributePathParams::Flags::kFieldIdValid))
    {
        attributePath.FieldId(aAttributePathParams.mFieldId);
    }

    if (aAttributePathParams.mFlags.Has(AttributePathParams::Flags::kListIndexValid))
    {
        attributePath.ListIndex(aAttributePathParams.mListIndex);
    }

    attributePath.NodeId(aAttributePathParams.mNodeId)
        .ClusterId(aAttributePathParams.mClusterId)
        .EndpointId(aAttributePathParams.mEndpointId)
        .EndOfAttributePath();

    return attributePath.GetError();
}

CHIP_ERROR WriteClient::FinalizeMessage(System::PacketBufferHandle & aPacket)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataList::Builder attributeDataListBuilder;
    VerifyOrExit(mState == State::AddAttribute, err = CHIP_ERROR_INCORRECT_STATE);
    attributeDataListBuilder = mWriteRequestBuilder.GetAttributeDataListBuilder().EndOfAttributeDataList();
    err                      = attributeDataListBuilder.GetError();
    SuccessOrExit(err);

    mWriteRequestBuilder.EndOfWriteRequest();
    err = mWriteRequestBuilder.GetError();
    SuccessOrExit(err);

    err = mMessageWriter.Finalize(&aPacket);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
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

CHIP_ERROR WriteClient::SendWriteRequest(NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * apSecureSession)
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
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    // TODO: Hard code keyID to 0 to unblock IM end-to-end test. Complete solution is tracked in issue:4451
    if (apSecureSession == nullptr)
    {
        mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0, aAdminId }, this);
    }
    else
    {
        mpExchangeCtx = mpExchangeMgr->NewContext(*apSecureSession, this);
    }
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(kImMessageTimeoutMsec);

    err = mpExchangeCtx->SendMessage(
        Protocols::InteractionModel::MsgType::WriteRequest, std::move(packet),
        Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse).Set(Messaging::SendMessageFlags::kNoAutoRequestAck));
    SuccessOrExit(err);
    MoveToState(State::AwaitingResponse);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ClearExistingExchangeContext();
    }
    ChipLogFunctError(err);

    return err;
}

void WriteClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendWriteRequest is called
    // back-to-back, the second call will call Close() on the first exchange,
    // which clears the OnMessageReceived callback.

    VerifyOrDie(apExchangeContext == mpExchangeCtx);

    // Verify that the message is an Invoke Command Response.
    // If not, close the exchange and free the payload.
    if (!aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteResponse))
    {
        apExchangeContext->Close();
        mpExchangeCtx = nullptr;
        ExitNow();
    }

    // Close the current exchange after receiving the response since the response message marks the
    // end of conversation represented by the exchange. We should create an new exchange for a new
    // conversation defined in Interaction Model protocol.
    ClearExistingExchangeContext();

    err = ProcessWriteResponseMessage(std::move(aPayload));

exit:
    if (mpDelegate != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            mpDelegate->WriteResponseError(this, err);
        }
        else
        {
            mpDelegate->WriteResponseProcessed(this);
        }
    }
    Shutdown();
}

void WriteClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive write response from Exchange: %d",
                    apExchangeContext->GetExchangeId());

    if (mpDelegate != nullptr)
    {
        mpDelegate->WriteResponseError(this, CHIP_ERROR_TIMEOUT);
    }
    Shutdown();
}

CHIP_ERROR WriteClient::ProcessAttributeStatusElement(AttributeStatusElement::Parser & aAttributeStatusElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePath::Parser attributePath;
    Protocols::SecureChannel::GeneralStatusCode generalCode = Protocols::SecureChannel::GeneralStatusCode::kSuccess;
    uint32_t protocolId                                     = 0;
    uint16_t protocolCode                                   = 0;
    StatusElement::Parser statusElementParser;
    AttributePathParams attributePathParams;

    mAttributeStatusIndex++;
    err = aAttributeStatusElement.GetAttributePath(&attributePath);
    SuccessOrExit(err);
    err = attributePath.GetNodeId(&(attributePathParams.mNodeId));
    SuccessOrExit(err);
    err = attributePath.GetClusterId(&(attributePathParams.mClusterId));
    SuccessOrExit(err);
    err = attributePath.GetEndpointId(&(attributePathParams.mEndpointId));
    SuccessOrExit(err);

    err = attributePath.GetFieldId(&(attributePathParams.mFieldId));
    if (CHIP_NO_ERROR == err)
    {
        attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);
    }
    else if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = attributePath.GetListIndex(&(attributePathParams.mListIndex));
    if (CHIP_NO_ERROR == err)
    {
        VerifyOrExit(attributePathParams.mFlags.Has(AttributePathParams::Flags::kFieldIdValid),
                     err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
        attributePathParams.mFlags.Set(AttributePathParams::Flags::kListIndexValid);
    }

    err = aAttributeStatusElement.GetStatusElement(&(statusElementParser));
    if (CHIP_NO_ERROR == err)
    {
        err = statusElementParser.DecodeStatusElement(&generalCode, &protocolId, &protocolCode);
        SuccessOrExit(err);
        if (mpDelegate != nullptr)
        {
            mpDelegate->WriteResponseStatus(this, generalCode, protocolId, protocolCode, attributePathParams,
                                            mAttributeStatusIndex);
        }
    }

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR && mpDelegate != nullptr)
    {
        mpDelegate->WriteResponseProtocolError(this, mAttributeStatusIndex);
    }
    return err;
}

} // namespace app
} // namespace chip
