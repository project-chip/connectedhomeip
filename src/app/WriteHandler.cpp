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

#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPath.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <support/TypeTraits.h>

namespace chip {
namespace app {
CHIP_ERROR WriteHandler::Init(InteractionModelDelegate * apDelegate)
{
    VerifyOrReturnError(apDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpExchangeCtx = nullptr;
    mpDelegate    = apDelegate;

    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    mMessageWriter.Init(std::move(packet));
    ReturnErrorOnFailure(mWriteResponseBuilder.Init(&mMessageWriter));

    AttributeStatusList::Builder attributeStatusListBuilder = mWriteResponseBuilder.CreateAttributeStatusListBuilder();
    ReturnErrorOnFailure(attributeStatusListBuilder.GetError());

    MoveToState(State::Initialized);

    return CHIP_NO_ERROR;
}

void WriteHandler::Shutdown()
{
    VerifyOrReturn(mState != State::Uninitialized);
    mMessageWriter.Reset();
    ClearExistingExchangeContext();
    mpDelegate = nullptr;
    ClearState();
}

void WriteHandler::ClearExistingExchangeContext()
{
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Close();
        mpExchangeCtx = nullptr;
    }
}

CHIP_ERROR WriteHandler::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mpExchangeCtx  = apExchangeContext;

    err = ProcessWriteRequest(std::move(aPayload));
    SuccessOrExit(err);
    err = SendWriteResponse();

exit:
    ChipLogFunctError(err);
    // Keep Shutdown() from double-closing our exchange.
    mpExchangeCtx = nullptr;
    Shutdown();
    return err;
}

CHIP_ERROR WriteHandler::FinalizeMessage(System::PacketBufferHandle & packet)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusList::Builder attributeStatusList;
    VerifyOrExit(mState == State::AddAttributeStatusCode, err = CHIP_ERROR_INCORRECT_STATE);
    attributeStatusList = mWriteResponseBuilder.GetAttributeStatusListBuilder().EndOfAttributeStatusList();
    err                 = attributeStatusList.GetError();
    SuccessOrExit(err);

    mWriteResponseBuilder.EndOfWriteResponse();
    err = mWriteResponseBuilder.GetError();
    SuccessOrExit(err);

    err = mMessageWriter.Finalize(&packet);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteHandler::SendWriteResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle packet;

    VerifyOrExit(mState == State::AddAttributeStatusCode, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeMessage(packet);
    SuccessOrExit(err);

    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::WriteResponse, std::move(packet));
    SuccessOrExit(err);

    MoveToState(State::Sending);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteHandler::ProcessAttributeDataList(TLV::TLVReader & aAttributeDataListReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = aAttributeDataListReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataElement::Parser element;
        AttributePath::Parser attributePath;
        ClusterInfo clusterInfo;
        TLV::TLVReader reader = aAttributeDataListReader;
        err                   = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetAttributePath(&attributePath);
        SuccessOrExit(err);

        err = attributePath.GetNodeId(&(clusterInfo.mNodeId));
        SuccessOrExit(err);

        err = attributePath.GetEndpointId(&(clusterInfo.mEndpointId));
        SuccessOrExit(err);

        err = attributePath.GetClusterId(&(clusterInfo.mClusterId));
        SuccessOrExit(err);

        err = attributePath.GetFieldId(&(clusterInfo.mFieldId));
        if (CHIP_NO_ERROR == err)
        {
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        err = attributePath.GetListIndex(&(clusterInfo.mListIndex));
        if (CHIP_NO_ERROR == err)
        {
            VerifyOrExit(clusterInfo.mFlags.Has(ClusterInfo::Flags::kFieldIdValid), err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
        }

        err = element.GetData(&dataReader);
        SuccessOrExit(err);
        err = WriteSingleClusterData(clusterInfo, dataReader, this);
        SuccessOrExit(err);
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteHandler::ProcessWriteRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    WriteRequest::Parser writeRequestParser;
    AttributeDataList::Parser attributeDataListParser;
    TLV::TLVReader attributeDataListReader;
    bool needSuppressResponse = false;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = writeRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = writeRequestParser.GetSuppressResponse(&needSuppressResponse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = writeRequestParser.GetAttributeDataList(&attributeDataListParser);
    SuccessOrExit(err);
    attributeDataListParser.GetReader(&attributeDataListReader);
    err = ProcessAttributeDataList(attributeDataListReader);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR WriteHandler::ConstructAttributePath(const AttributePathParams & aAttributePathParams,
                                                AttributeStatusElement::Builder aAttributeStatusElement)
{
    AttributePath::Builder attributePath = aAttributeStatusElement.CreateAttributePathBuilder();
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

CHIP_ERROR WriteHandler::AddAttributeStatusCode(const AttributePathParams & aAttributePathParams,
                                                const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                const Protocols::Id aProtocolId,
                                                const Protocols::InteractionModel::ProtocolCode aProtocolCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusElement::Builder statusElementBuilder;
    AttributeStatusElement::Builder attributeStatusElement =
        mWriteResponseBuilder.GetAttributeStatusListBuilder().CreateAttributeStatusBuilder();
    err = attributeStatusElement.GetError();
    SuccessOrExit(err);

    err = ConstructAttributePath(aAttributePathParams, attributeStatusElement);
    SuccessOrExit(err);

    statusElementBuilder = attributeStatusElement.CreateStatusElementBuilder();
    statusElementBuilder
        .EncodeStatusElement(aGeneralCode, aProtocolId.ToFullyQualifiedSpecForm(), chip::to_underlying(aProtocolCode))
        .EndOfStatusElement();
    err = statusElementBuilder.GetError();
    SuccessOrExit(err);

    attributeStatusElement.EndOfAttributeStatusElement();
    err = attributeStatusElement.GetError();
    SuccessOrExit(err);
    MoveToState(State::AddAttributeStatusCode);

exit:
    ChipLogFunctError(err);
    return err;
}

const char * WriteHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Uninitialized:
        return "Uninitialized";

    case State::Initialized:
        return "Initialized";

    case State::AddAttributeStatusCode:
        return "AddAttributeStatusCode";
    case State::Sending:
        return "Sending";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void WriteHandler::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());
}

void WriteHandler::ClearState()
{
    MoveToState(State::Uninitialized);
}

} // namespace app
} // namespace chip
