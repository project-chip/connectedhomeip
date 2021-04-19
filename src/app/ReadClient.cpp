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
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr = apExchangeMgr;
    mpExchangeCtx = nullptr;
    mpDelegate    = apDelegate;
    mState        = ClientState::Initialized;

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
                                       size_t aEventPathParamsListSize)
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

    ClearExistingExchangeContext();
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

    bool isEventListPresent  = false;
    bool suppressResponse    = false;
    bool moreChunkedMessages = false;

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

        VerifyOrExit(moreChunkedMessages == false, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

        if (isEventListPresent && nullptr != mpDelegate)
        {
            chip::TLV::TLVReader eventListReader;
            eventList.GetReader(&eventListReader);
            err = mpDelegate->EventStreamReceived(mpExchangeCtx, &eventListReader);
            SuccessOrExit(err);
        }
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
    ClearExistingExchangeContext();
    MoveToState(ClientState::Initialized);
    if (nullptr != mpDelegate)
    {
        mpDelegate->ReportError(this, CHIP_ERROR_TIMEOUT);
    }
}
}; // namespace app
}; // namespace chip
