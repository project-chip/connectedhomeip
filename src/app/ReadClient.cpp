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

CHIP_ERROR ReadClient::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate,
                            CatalogInterface<ClusterDataSink> * apClusterSinkCatalog)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(apExchangeMgr != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr        = apExchangeMgr;
    mpExchangeCtx        = nullptr;
    mpDelegate           = apDelegate;
    mState               = ClientState::Initialized;
    mpClusterSinkCatalog = apClusterSinkCatalog;

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::Shutdown()
{
    ClearExistingExchangeContext();
    mpExchangeMgr = nullptr;
    mpDelegate    = nullptr;
    MoveToState(ClientState::Uninitialized);
    mpClusterSinkCatalog = nullptr;
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
                                       size_t aEventPathParamsListSize, AttributePathParams * apClusterPathList,
                                       size_t aClusterPathListSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;
    ChipLogDetail(DataManagement, "%s: Client[%u] [%5.5s]", __func__,
                  InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this), GetStateStr());
    VerifyOrExit(ClientState::Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

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
            // TODO: fill to construct event paths
        }

        if (aClusterPathListSize != 0 && apClusterPathList != nullptr && mpClusterSinkCatalog != nullptr)
        {
            AttributePathList::Builder attributePathListBuilder = request.CreateAttributePathListBuilder();
            for (size_t index = 0; index < aClusterPathListSize; index++)
            {
                EndpointId endpointId                       = 0;
                ClusterId clusterId                         = 0;
                AttributePath::Builder attributePathBuilder = attributePathListBuilder.CreateAttributePathBuilder();
                err = mpClusterSinkCatalog->GetEndpointId(apClusterPathList[index].mClusterDataHandle, endpointId);
                SuccessOrExit(err);
                err = mpClusterSinkCatalog->GetClusterId(apClusterPathList[index].mClusterDataHandle, clusterId);
                SuccessOrExit(err);
                attributePathBuilder.EndpointId(endpointId).ClusterId(clusterId);
                SuccessOrExit(attributePathBuilder.GetError());
            }
        }

        request.EndOfReadRequest();
        SuccessOrExit(request.GetError());

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
    return err;
}

void ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                   const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData),
                 err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    err = ProcessReportData(std::move(aPayload));

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR && mpDelegate != nullptr)
    {
        mpDelegate->ReportError(this, err);
    }
    ClearExistingExchangeContext();
    MoveToState(ClientState::Initialized);
    return;
}

CHIP_ERROR ReadClient::ClearExistingExchangeContext()
{
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportData::Parser report;

    bool isEventListPresent         = false;
    bool isAttributeDataListPresent = false;
    bool suppressResponse           = false;
    bool moreChunkedMessages        = false;

    System::PacketBufferTLVReader reader;

    reader.Init(std::move(aPayload));
    reader.Next();

    err = report.Init(reader);
    SuccessOrExit(err);

    err = report.CheckSchemaValidity();
    SuccessOrExit(err);

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

    {
        EventList::Parser eventList;

        err = report.GetEventDataList(&eventList);
        if (CHIP_NO_ERROR == err)
        {
            isEventListPresent = true;
        }
        else if (CHIP_END_OF_TLV == err)
        {
            isEventListPresent = false;
            err                = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        if (isEventListPresent && nullptr != mpDelegate && !moreChunkedMessages)
        {
            chip::TLV::TLVReader eventListReader;
            eventList.GetReader(&eventListReader);
            err = mpDelegate->EventStreamReceived(mpExchangeCtx, &eventListReader);
            SuccessOrExit(err);
        }
    }

    {
        AttributeDataList::Parser attributeDataList;

        err = report.GetAttributeDataList(&attributeDataList);
        if (CHIP_NO_ERROR == err)
        {
            isAttributeDataListPresent = true;
        }
        else if (CHIP_END_OF_TLV == err)
        {
            isAttributeDataListPresent = false;
            err                        = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        if (isAttributeDataListPresent && nullptr != mpDelegate && !moreChunkedMessages)
        {
            chip::TLV::TLVReader attributeDataListReader;
            attributeDataList.GetReader(&attributeDataListReader);
            err = ProcessAttributeDataList(attributeDataListReader);
            SuccessOrExit(err);
        }
    }
    if (!suppressResponse)
    {
        // TODO: Add status report support and correspond handler in ReadHandler, particular for situation when there
        // are multiple reports
    }
    if (nullptr != mpDelegate && !moreChunkedMessages)
    {
        err = mpDelegate->ReportProcessed(this);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR ReadClient::ProcessAttributeDataList(TLV::TLVReader & aAttributeDataListReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mpClusterSinkCatalog != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    while (CHIP_NO_ERROR == (err = aAttributeDataListReader.Next()))
    {
        AttributeDataElement::Parser element;
        AttributePath::Parser attributePathParser;
        TLV::TLVReader reader      = aAttributeDataListReader;
        ClusterDataSink * dataSink = nullptr;
        ClusterDataHandle clusterDataHandle;
        err = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetAttributePath(&attributePathParser);
        SuccessOrExit(err);

        err = mpClusterSinkCatalog->LocateClusterDataHandle(attributePathParser, clusterDataHandle);
        SuccessOrExit(err);

        err = mpClusterSinkCatalog->LocateClusterInstance(clusterDataHandle, &dataSink);
        SuccessOrExit(err);

        reader = aAttributeDataListReader;
        err    = dataSink->StoreDataElement(kRootAttributePathHandle, reader);
        SuccessOrExit(err);
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive report data from Exchange: %d",
                    apExchangeContext->GetExchangeId());
    ClearExistingExchangeContext();
    MoveToState(ClientState::Initialized);
    if (nullptr != mpDelegate)
    {
        mpDelegate->ReportError(this, CHIP_ERROR_TIMEOUT);
    }
}
}; // namespace app
}; // namespace chip
