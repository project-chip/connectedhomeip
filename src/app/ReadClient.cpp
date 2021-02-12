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
 *      This file defines read client for a CHIP Interaction Data model
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
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr  = apExchangeMgr;
    mpExchangeCtx  = nullptr;
    mpDelegate = apDelegate;
    mState = kState_Initialized;
    mMoreChunkedMessages = false;

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::Shutdown()
{
    if (mState == kState_Uninitialized)
        return;
    ClearExistingExchangeContext();
    mpExchangeMgr = nullptr;
    mpDelegate = nullptr;
    MoveToState(kState_Uninitialized);
}

void ReadClient::Reset()
{
    ClearExistingExchangeContext();
    MoveToState(kState_Initialized);
}

const char * ReadClient::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case kState_Uninitialized:
        return "UNINIT";
    case kState_Initialized:
        return "INIT";
    case kState_ReadRequestSending:
        return "READREQUESTSENDING";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadClient::MoveToState(const ClientState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Client[%u] moving to [%5.5s]", InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this),
                  GetStateStr());
}

void ReadClient::ClearState(void)
{
    MoveToState(kState_Uninitialized);
}

CHIP_ERROR ReadClient::SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf = NULL;
    chip::app::InteractionModelDelegate::InEventParam inParam;
    chip::app::InteractionModelDelegate::OutEventParam outParam;

    ChipLogDetail(DataManagement, "Client[%u] [%5.5s]", InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this),
                  GetStateStr());
    VerifyOrExit(kState_Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);

    if (nullptr != mpDelegate)
    {
        outParam.mReadRequestPrepareNeeded.eventPathParamsList = nullptr;
        mpDelegate->HandleEvent(chip::app::InteractionModelDelegate::EventId::kReadRequestPrepareNeeded, inParam,
                                outParam);
    }

    {
        chip::System::PacketBufferTLVWriter writer;
        ReadRequest::Builder request;

        msgBuf = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLength);
        VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        writer.Init(std::move(msgBuf));

        err = request.Init(&writer);
        SuccessOrExit(err);

        if (outParam.mReadRequestPrepareNeeded.eventPathParamsListSize > 0)
        {
            EventPathList::Builder & eventPathListBuilder = request.CreateEventPathListBuilder();
            EventPath::Builder eventPathBuilder           = eventPathListBuilder.CreateEventPathBuilder();
            for (size_t i = 0; i < outParam.mReadRequestPrepareNeeded.eventPathParamsListSize; ++i)
            {
                EventPathParams eventPath = outParam.mReadRequestPrepareNeeded.eventPathParamsList[i];

                if (eventPath.mNodeId != 0)
                    eventPathBuilder.NodeId(eventPath.mNodeId);

                if (eventPath.mEventId != 0)
                    eventPathBuilder.EventId(eventPath.mEventId);

                eventPathBuilder.EndpointId(eventPath.mEndpointId).ClusterId(eventPath.mClusterId).EndOfEventPath();
                SuccessOrExit(eventPathBuilder.GetError());
            }

            eventPathListBuilder.EndOfEventPathList();
            SuccessOrExit(eventPathListBuilder.GetError());

            if (outParam.mReadRequestPrepareNeeded.eventNumber != 0)
            {
                request.EventNumber(outParam.mReadRequestPrepareNeeded.eventNumber);
            }
        }

        request.EndOfReadRequest();
        SuccessOrExit(request.GetError());

        err = writer.Finalize(&msgBuf);
        SuccessOrExit(err);

        VerifyOrExit(msgBuf->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    ClearExistingExchangeContext();

    // Create a new exchange context.
    // TODO: temporary create a SecureSessionHandle from node id, will be fix in PR 3602
    mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0, aAdminId }, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(kImMesssageTimeout);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(kState_ReadRequestSending);

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                   const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrDie(apExchangeContext == mpExchangeCtx);

    ClearExistingExchangeContext();

    err = ProcessReportData(std::move(aPayload));
    SuccessOrExit(err);

exit:
    Reset();
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

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::InteractionModelDelegate::InEventParam inParam;
    chip::app::InteractionModelDelegate::OutEventParam outParam;
    ReportData::Parser report;

    bool isEventListPresent = false;
    bool suppressResponse   = false;

    chip::System::PacketBufferTLVReader reader;

    if (nullptr != mpDelegate)
    {
        inParam.mReportDataRequest.exchangeContext = mpExchangeCtx;
        mpDelegate->HandleEvent(chip::app::InteractionModelDelegate::EventId::kReportDataRequest, inParam, outParam);
    }

    reader.Init(std::move(aPayload));
    reader.Next();

    err = report.Init(reader);
    SuccessOrExit(err);

    err = report.CheckSchemaValidity();
    SuccessOrExit(err);

    err = report.GetSuppressResponse(&suppressResponse);
    if (CHIP_END_OF_TLV == err)
        err = CHIP_NO_ERROR;
    SuccessOrExit(err);

    err = report.GetMoreChunkedMessages(&mMoreChunkedMessages);
    if (CHIP_END_OF_TLV == err)
        err = CHIP_NO_ERROR;
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

        if (isEventListPresent && nullptr != mpDelegate)
        {
            // re-initialize the reader (reuse to save stack depth).
            inParam.Clear();
            outParam.Clear();
            eventList.GetReader(&reader);
            inParam.mEventStreamReceived.reader = &reader;
            mpDelegate->HandleEvent(chip::app::InteractionModelDelegate::EventId::kEventStreamReceived, inParam, outParam);
        }
    }

    if (!suppressResponse)
    {
        /*
        uint8_t * p = msgBuf->Start();
        chip::Encoding::LittleEndian::Write32(p, chip::Protocols::kChipProtocol_Common);
        chip::Encoding::LittleEndian::Write16(p, chip::Protocols::Common::kStatus_Success);

        err    = mpExchangeCtx->SendMessage(kChipProfile_Common, chip::protocols:::Common::kMsgType_StatusReport, msgBuf);
        msgBuf = NULL;
        SuccessOrExit(err);
         */
        // Todo: Add status report support
    }
    if (nullptr != mpDelegate)
    {
        inParam.Clear();
        outParam.Clear();
        mpDelegate->HandleEvent(chip::app::InteractionModelDelegate::EventId::kReportProcessed, inParam, outParam);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive report data from Exchange: %d", apExchangeContext->GetExchangeId());
    Reset();
}
}; // namespace app
}; // namespace chip
