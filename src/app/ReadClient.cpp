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

CHIP_ERROR ReadClient::Init(Messaging::ExchangeManager * apExchangeMgr, EventCallback const aEventCallback, void * const apAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    Reset();
    mpAppState     = apAppState;
    mpExchangeMgr  = apExchangeMgr;
    mpExchangeCtx  = nullptr;
    mEventCallback = aEventCallback;

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::Reset()
{
    mpAppState = nullptr;
    ClearExistingExchangeContext();
    MoveToState(kState_Initialized);
    return;
}

void ReadClient::Shutdown()
{
    VerifyOrExit(mState != kState_Uninitialized, );
    mpAppState = nullptr;
    ClearExistingExchangeContext();
    mpExchangeMgr = nullptr;
    MoveToState(kState_Uninitialized);

exit:
    return;
}

#if CHIP_DETAIL_LOGGING
const char * ReadClient::GetStateStr() const
{
    switch (mState)
    {
    case kState_Uninitialized:
        return "UNINIT";
    case kState_Initialized:
        return "INIT";
    case kState_ReadRequestSending:
        return "READREQUESTSENDING";
    }
    return "N/A";
}
#else  // CHIP_DETAIL_LOGGING
const char * ReadClient::GetStateStr() const
{
    return "N/A";
}
#endif // CHIP_DETAIL_LOGGING

void ReadClient::MoveToState(const ClientState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Client[%u] moving to [%5.5s]", InteractionModelEngine::GetInstance()->GetReadClientId(this),
                  GetStateStr());
}

void ReadClient::ClearState(void)
{
    MoveToState(kState_Uninitialized);
}

void ReadClient::DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam)
{
    IgnoreUnusedVariable(aInParam);
    IgnoreUnusedVariable(aOutParam);

    ChipLogDetail(DataManagement, "%s event: %d", __func__, aEvent);
}

CHIP_ERROR ReadClient::SendReadRequest(NodeId aNodeId)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf = NULL;
    InEventParam inReadParam;
    OutEventParam outReadParam;

    ChipLogDetail(DataManagement, "Client[%u] [%5.5s]", InteractionModelEngine::GetInstance()->GetReadClientId(this),
                  GetStateStr());

    inReadParam.Clear();
    outReadParam.Clear();

    outReadParam.mReadRequestPrepareNeeded.mpEventPathParamsList = NULL;
    mEventCallback(this, kEvent_OnReadRequestPrepareNeeded, inReadParam, outReadParam);

    VerifyOrExit(kState_Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);

    {
        chip::System::PacketBufferTLVWriter writer;
        ReadRequest::Builder request;

        msgBuf = System::PacketBufferHandle::New(System::kMaxPacketBufferSize);
        VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        writer.Init(std::move(msgBuf));

        err = request.Init(&writer);
        SuccessOrExit(err);

        if (outReadParam.mReadRequestPrepareNeeded.mEventPathParamsListSize > 0)
        {
            EventPathList::Builder & eventPathListBuilder = request.CreateEventPathListBuilder();
            EventPath::Builder eventPathBuilder           = eventPathListBuilder.CreateEventPathBuilder();
            for (size_t i = 0; i < outReadParam.mReadRequestPrepareNeeded.mEventPathParamsListSize; ++i)
            {
                EventPathParams eventPath = outReadParam.mReadRequestPrepareNeeded.mpEventPathParamsList[i];

                if (eventPath.mNodeId != 0)
                    eventPathBuilder.NodeId(eventPath.mNodeId);

                if (eventPath.mEventId != 0)
                    eventPathBuilder.EventId(eventPath.mEventId);

                eventPathBuilder.EndpointId(eventPath.mEndpointId).ClusterId(eventPath.mClusterId).EndOfEventPath();
                SuccessOrExit(eventPathBuilder.GetError());
            }

            eventPathListBuilder.EndOfEventPathList();
            SuccessOrExit(eventPathListBuilder.GetError());

            if (outReadParam.mReadRequestPrepareNeeded.mEventNumber != 0)
            {
                request.EventNumber(outReadParam.mReadRequestPrepareNeeded.mEventNumber);
            }
        }

        request.EndOfReadRequest();
        SuccessOrExit(request.GetError());

        err = writer.Finalize(&msgBuf);
        SuccessOrExit(err);

        msgBuf->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);
    }

    ClearExistingExchangeContext();

    // Create a new exchange context.
    // TODO: temporary create a SecureSessionHandle from node id, will be fix in PR 3602
    mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0 }, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(CHIP_REPORT_TIMEOUT);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);
    MoveToState(kState_ReadRequestSending);

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadClient::OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                   const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    // Assert that the exchange context matches the client's current context.
    // This should never fail because even if SendCommandRequest is called
    // back-to-back

    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrDie(apEc == mpExchangeCtx);

    ClearExistingExchangeContext();

    err = ProcessReportData(std::move(aPayload));
    SuccessOrExit(err);

exit:
    Reset();
    return;
}

CHIP_ERROR ReadClient::ClearExistingExchangeContext()
{
    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
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
    InEventParam inParam;
    OutEventParam outParam;
    ReportData::Parser report;

    bool isEventListPresent = false;
    bool suppressResponse   = false;

    chip::System::PacketBufferTLVReader reader;

    {
        inParam.Clear();
        outParam.Clear();
        inParam.mReportDataRequest.mpEC = mpExchangeCtx;
        // inParam.mReportDataRequest.mpMessage = aPayload;
        inParam.mReportDataRequest.mpClient = this;

        // NOTE: state could be changed in the callback to app layer
        mEventCallback(mpAppState, kEvent_OnReportDataRequest, inParam, outParam);
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

        if (isEventListPresent)
        {
            // re-initialize the reader (reuse to save stack depth).
            eventList.GetReader(&reader);
            inParam.mEventStreamReceived.mpReader = &reader;
            inParam.mEventStreamReceived.mpClient = this;

            // Invoke our callback.
            mEventCallback(mpAppState, kEvent_OnEventStreamReceived, inParam, outParam);
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

    inParam.mReportProcessed.mpClient = this;
    // NOTE: state could be changed in the callback to app layer
    mEventCallback(mpAppState, kEvent_OnReportProcessed, inParam, outParam);

exit:
    ChipLogFunctError(err);
    Reset();
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apEc)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive report data from Exchange: %d", apEc->GetExchangeId());
    Reset();
}
}; // namespace app
}; // namespace chip
