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
 *      This file defines the initiator side of a CHIP Read Interaction.
 *
 */

#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>

namespace chip {
namespace app {

CHIP_ERROR ReadClient::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(apExchangeMgr != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;
    mState        = ClientState::Initialized;

    AbortExistingExchangeContext();

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::Shutdown()
{
    AbortExistingExchangeContext();
    mpExchangeMgr = nullptr;
    mpDelegate    = nullptr;
    MoveToState(ClientState::Uninitialized);
}

const char * ReadClient::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case ClientState::Uninitialized:
        return "UNINIT";
    case ClientState::Initialized:
        return "INIT";
    case ClientState::AwaitingResponse:
        return "AwaitingResponse";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadClient::MoveToState(const ClientState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Client[%u] moving to [%s]", InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this),
                  GetStateStr());
}

CHIP_ERROR ReadClient::SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId, EventPathParams * apEventPathParamsList,
                                       size_t aEventPathParamsListSize, AttributePathParams * apAttributePathParamsList,
                                       size_t aAttributePathParamsListSize, EventNumber aEventNumber)
{
    // TODO: SendRequest parameter is too long, need to have the structure to represent it
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;
    ChipLogDetail(DataManagement, "%s: Client[%u] [%5.5s]", __func__,
                  InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this), GetStateStr());
    VerifyOrExit(ClientState::Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Discard any existing exchange context. Effectively we can only have one exchange per ReadClient
    // at any one time.
    AbortExistingExchangeContext();

    {
        System::PacketBufferTLVWriter writer;
        ReadRequest::Builder request;

        msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        writer.Init(std::move(msgBuf));

        err = request.Init(&writer);
        SuccessOrExit(err);

        if (aEventPathParamsListSize != 0 && apEventPathParamsList != nullptr)
        {
            EventPathList::Builder & eventPathListBuilder = request.CreateEventPathListBuilder();
            EventPath::Builder eventPathBuilder           = eventPathListBuilder.CreateEventPathBuilder();
            for (size_t eventIndex = 0; eventIndex < aEventPathParamsListSize; ++eventIndex)
            {
                EventPathParams eventPath = apEventPathParamsList[eventIndex];
                eventPathBuilder.NodeId(eventPath.mNodeId)
                    .EventId(eventPath.mEventId)
                    .EndpointId(eventPath.mEndpointId)
                    .ClusterId(eventPath.mClusterId)
                    .EndOfEventPath();
                SuccessOrExit(err = eventPathBuilder.GetError());
            }

            eventPathListBuilder.EndOfEventPathList();
            SuccessOrExit(err = eventPathListBuilder.GetError());

            if (aEventNumber != 0)
            {
                // EventNumber is optional
                request.EventNumber(aEventNumber);
            }
        }

        if (aAttributePathParamsListSize != 0 && apAttributePathParamsList != nullptr)
        {
            err = GenerateAttributePathList(request, apAttributePathParamsList, aAttributePathParamsListSize);
            SuccessOrExit(err);
        }

        request.EndOfReadRequest();
        SuccessOrExit(err = request.GetError());

        err = writer.Finalize(&msgBuf);
        SuccessOrExit(err);
    }

    mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0, aAdminId }, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(kImMessageTimeoutMsec);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(ClientState::AwaitingResponse);

exit:
    ChipLogFunctError(err);

    if (err != CHIP_NO_ERROR)
    {
        AbortExistingExchangeContext();
    }

    return err;
}

CHIP_ERROR ReadClient::GenerateAttributePathList(ReadRequest::Builder & aRequest, AttributePathParams * apAttributePathParamsList,
                                                 size_t aAttributePathParamsListSize)
{
    AttributePathList::Builder attributePathListBuilder = aRequest.CreateAttributePathListBuilder();
    ReturnErrorOnFailure(attributePathListBuilder.GetError());
    for (size_t index = 0; index < aAttributePathParamsListSize; index++)
    {
        AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
        attributePathBuilder.NodeId(apAttributePathParamsList[index].mNodeId)
            .EndpointId(apAttributePathParamsList[index].mEndpointId)
            .ClusterId(apAttributePathParamsList[index].mClusterId);
        if (apAttributePathParamsList[index].mFlags.Has(AttributePathParams::Flags::kFieldIdValid))
        {
            attributePathBuilder.FieldId(apAttributePathParamsList[index].mFieldId);
        }

        if (apAttributePathParamsList[index].mFlags.Has(AttributePathParams::Flags::kListIndexValid))
        {
            VerifyOrReturnError(apAttributePathParamsList[index].mFlags.Has(AttributePathParams::Flags::kFieldIdValid),
                                CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
            attributePathBuilder.ListIndex(apAttributePathParamsList[index].mListIndex);
        }

        attributePathBuilder.EndOfAttributePath();
        ReturnErrorOnFailure(attributePathBuilder.GetError());
    }
    attributePathListBuilder.EndOfAttributePathList();
    return attributePathListBuilder.GetError();
}

void ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                   const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData),
                 err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    err = ProcessReportData(std::move(aPayload));

exit:
    ChipLogFunctError(err);

    // Close the exchange cleanly so that the ExchangeManager will send an ack for the message we just received.
    mpExchangeCtx->Close();
    mpExchangeCtx = nullptr;
    MoveToState(ClientState::Initialized);

    if (mpDelegate != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            mpDelegate->ReportError(this, err);
        }
        else
        {
            mpDelegate->ReportProcessed(this);
        }
    }

    return;
}

CHIP_ERROR ReadClient::AbortExistingExchangeContext()
{
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Parser report;

    bool isEventListPresent         = false;
    bool isAttributeDataListPresent = false;
    bool suppressResponse           = false;
    bool moreChunkedMessages        = false;
    EventList::Parser eventList;
    AttributeDataList::Parser attributeDataList;
    System::PacketBufferTLVReader reader;

    reader.Init(std::move(aPayload));
    reader.Next();

    err = report.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = report.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = report.GetSuppressResponse(&suppressResponse);
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = report.GetMoreChunkedMessages(&moreChunkedMessages);
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err                = report.GetEventDataList(&eventList);
    isEventListPresent = (err == CHIP_NO_ERROR);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    if (isEventListPresent && nullptr != mpDelegate)
    {
        chip::TLV::TLVReader eventListReader;
        eventList.GetReader(&eventListReader);
        err = mpDelegate->EventStreamReceived(mpExchangeCtx, &eventListReader);
        SuccessOrExit(err);
    }

    err                        = report.GetAttributeDataList(&attributeDataList);
    isAttributeDataListPresent = (err == CHIP_NO_ERROR);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    if (isAttributeDataListPresent && nullptr != mpDelegate && !moreChunkedMessages)
    {
        chip::TLV::TLVReader attributeDataListReader;
        attributeDataList.GetReader(&attributeDataListReader);
        err = ProcessAttributeDataList(attributeDataListReader);
        SuccessOrExit(err);
    }

    if (!suppressResponse)
    {
        // TODO: Add status report support and correspond handler in ReadHandler, particular for situation when there
        // are multiple reports
    }

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive report data from Exchange: %d",
                    apExchangeContext->GetExchangeId());
    AbortExistingExchangeContext();
    MoveToState(ClientState::Initialized);
    if (nullptr != mpDelegate)
    {
        mpDelegate->ReportError(this, CHIP_ERROR_TIMEOUT);
    }
}

CHIP_ERROR ReadClient::ProcessAttributeDataList(TLV::TLVReader & aAttributeDataListReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = aAttributeDataListReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataElement::Parser element;
        AttributePath::Parser attributePathParser;
        ClusterInfo clusterInfo;
        TLV::TLVReader reader = aAttributeDataListReader;
        err                   = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetAttributePath(&attributePathParser);
        SuccessOrExit(err);

        err = attributePathParser.GetNodeId(&(clusterInfo.mNodeId));
        SuccessOrExit(err);

        err = attributePathParser.GetEndpointId(&(clusterInfo.mEndpointId));
        SuccessOrExit(err);

        err = attributePathParser.GetClusterId(&(clusterInfo.mClusterId));
        SuccessOrExit(err);

        err = attributePathParser.GetFieldId(&(clusterInfo.mFieldId));
        if (CHIP_NO_ERROR == err)
        {
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        err = attributePathParser.GetListIndex(&(clusterInfo.mListIndex));
        if (CHIP_NO_ERROR == err)
        {
            VerifyOrExit(clusterInfo.mFlags.Has(ClusterInfo::Flags::kFieldIdValid), err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        err = element.GetData(&dataReader);
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
}; // namespace app
}; // namespace chip
