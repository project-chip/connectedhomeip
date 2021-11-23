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

#include "lib/core/CHIPTLVTypes.h"
#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/StatusResponse.h>

namespace chip {
namespace app {

CHIP_ERROR ReadClient::Init(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback, InteractionType aInteractionType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(IsFree(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(apExchangeMgr != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mpExchangeMgr              = apExchangeMgr;
    mpCallback                 = apCallback;
    mState                     = ClientState::Initialized;
    mMinIntervalFloorSeconds   = 0;
    mMaxIntervalCeilingSeconds = 0;
    mSubscriptionId            = 0;
    mInitialReport             = true;
    mInteractionType           = aInteractionType;
    AbortExistingExchangeContext();

exit:
    return err;
}

void ReadClient::Shutdown()
{
    AbortExistingExchangeContext();
    ShutdownInternal(CHIP_NO_ERROR);
}

void ReadClient::ShutdownInternal(CHIP_ERROR aError)
{
    if (mpCallback != nullptr)
    {
        if (aError != CHIP_NO_ERROR)
        {
            mpCallback->OnError(this, aError);
        }
        mpCallback->OnDone(this);
        mpCallback = nullptr;
    }
    if (IsSubscriptionType())
    {
        CancelLivenessCheckTimer();
    }
    mMinIntervalFloorSeconds   = 0;
    mMaxIntervalCeilingSeconds = 0;
    mSubscriptionId            = 0;
    mInteractionType           = InteractionType::Read;
    mpExchangeMgr              = nullptr;
    mpExchangeCtx              = nullptr;
    mInitialReport             = true;
    mPeerNodeId                = kUndefinedNodeId;
    mFabricIndex               = kUndefinedFabricIndex;
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
    case ClientState::AwaitingInitialReport:
        return "AwaitingInitialReport";
    case ClientState::AwaitingSubscribeResponse:
        return "AwaitingSubscribeResponse";
    case ClientState::SubscriptionActive:
        return "SubscriptionActive";
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

CHIP_ERROR ReadClient::SendReadRequest(ReadPrepareParams & aReadPrepareParams)
{
    // TODO: SendRequest parameter is too long, need to have the structure to represent it
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;
    ChipLogDetail(DataManagement, "%s: Client[%u] [%5.5s]", __func__,
                  InteractionModelEngine::GetInstance()->GetReadClientArrayIndex(this), GetStateStr());
    VerifyOrExit(ClientState::Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpCallback != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Discard any existing exchange context. Effectively we can only have one exchange per ReadClient
    // at any one time.
    AbortExistingExchangeContext();

    {
        System::PacketBufferTLVWriter writer;
        ReadRequestMessage::Builder request;

        msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        writer.Init(std::move(msgBuf));

        err = request.Init(&writer);
        SuccessOrExit(err);

        if (aReadPrepareParams.mEventPathParamsListSize != 0 && aReadPrepareParams.mpEventPathParamsList != nullptr)
        {
            EventPaths::Builder & eventPathListBuilder = request.CreateEventRequests();
            SuccessOrExit(err = eventPathListBuilder.GetError());
            err = GenerateEventPaths(eventPathListBuilder, aReadPrepareParams.mpEventPathParamsList,
                                     aReadPrepareParams.mEventPathParamsListSize);
            SuccessOrExit(err);
            if (aReadPrepareParams.mEventNumber != 0)
            {
                // EventFilter is optional
                EventFilters::Builder eventFilters = request.CreateEventFilters();
                SuccessOrExit(err = request.GetError());
                EventFilterIB::Builder eventFilter = eventFilters.CreateEventFilter();
                eventFilter.EventMin(aReadPrepareParams.mEventNumber).EndOfEventFilterIB();
                SuccessOrExit(err = eventFilter.GetError());
                eventFilters.EndOfEventFilters();
                SuccessOrExit(err = eventFilters.GetError());
            }
        }

        if (aReadPrepareParams.mAttributePathParamsListSize != 0 && aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            AttributePathIBs::Builder attributePathListBuilder = request.CreateAttributeRequests();
            SuccessOrExit(err = attributePathListBuilder.GetError());
            err = GenerateAttributePathList(attributePathListBuilder, aReadPrepareParams.mpAttributePathParamsList,
                                            aReadPrepareParams.mAttributePathParamsListSize);
            SuccessOrExit(err);
        }

        request.IsFabricFiltered(false).EndOfReadRequestMessage();
        SuccessOrExit(err = request.GetError());

        err = writer.Finalize(&msgBuf);
        SuccessOrExit(err);
    }

    mpExchangeCtx = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHandle, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(aReadPrepareParams.mTimeout);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    mPeerNodeId  = aReadPrepareParams.mSessionHandle.GetPeerNodeId();
    mFabricIndex = aReadPrepareParams.mSessionHandle.GetFabricIndex();

    MoveToState(ClientState::AwaitingInitialReport);

exit:

    if (err != CHIP_NO_ERROR)
    {
        AbortExistingExchangeContext();
    }

    return err;
}

CHIP_ERROR ReadClient::GenerateEventPaths(EventPaths::Builder & aEventPathsBuilder, EventPathParams * apEventPathParamsList,
                                          size_t aEventPathParamsListSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (size_t eventIndex = 0; eventIndex < aEventPathParamsListSize; ++eventIndex)
    {
        EventPathIB::Builder eventPathBuilder = aEventPathsBuilder.CreatePath();
        EventPathParams eventPath             = apEventPathParamsList[eventIndex];
        eventPathBuilder.Node(eventPath.mNodeId)
            .Event(eventPath.mEventId)
            .Endpoint(eventPath.mEndpointId)
            .Cluster(eventPath.mClusterId)
            .EndOfEventPathIB();
        SuccessOrExit(err = eventPathBuilder.GetError());
    }

    aEventPathsBuilder.EndOfEventPaths();
    SuccessOrExit(err = aEventPathsBuilder.GetError());

exit:
    return err;
}

CHIP_ERROR ReadClient::GenerateAttributePathList(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                                 AttributePathParams * apAttributePathParamsList,
                                                 size_t aAttributePathParamsListSize)
{
    for (size_t index = 0; index < aAttributePathParamsListSize; index++)
    {
        VerifyOrReturnError(apAttributePathParamsList[index].IsValidAttributePath(), CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
        ReturnErrorOnFailure(apAttributePathParamsList[index].BuildAttributePath(aAttributePathIBsBuilder.CreateAttributePath()));
    }
    aAttributePathIBsBuilder.EndOfAttributePathIBs();
    return aAttributePathIBsBuilder.GetError();
}

CHIP_ERROR ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                         System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(!IsFree(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpCallback != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        err = ProcessReportData(std::move(aPayload));
        SuccessOrExit(err);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeResponse))
    {
        VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
        err = ProcessSubscribeResponse(std::move(aPayload));
        // Forget the context as SUBSCRIBE RESPONSE is the last message in SUBSCRIBE transaction and
        // ExchangeContext::HandleMessage automatically closes a context if no other messages need to
        // be sent or received.
        mpExchangeCtx = nullptr;
        SuccessOrExit(err);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        StatusIB status;
        VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
        err = StatusResponse::ProcessStatusResponse(std::move(aPayload), status);
        SuccessOrExit(err);
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    if ((!IsSubscriptionType() && !mPendingMoreChunks) || err != CHIP_NO_ERROR)
    {
        ShutdownInternal(err);
    }
    return err;
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

CHIP_ERROR ReadClient::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                               System::PacketBufferHandle && aPayload)
{
    mpExchangeCtx  = apExchangeContext;
    CHIP_ERROR err = ProcessReportData(std::move(aPayload));
    if (err != CHIP_NO_ERROR)
    {
        ShutdownInternal(err);
    }
    return err;
}

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Parser report;

    bool isEventReportsPresent       = false;
    bool isAttributeReportIBsPresent = false;
    bool suppressResponse            = true;
    uint64_t subscriptionId          = 0;
    EventReports::Parser EventReports;
    AttributeReportIBs::Parser attributeReportIBs;
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
        suppressResponse = false;
        err              = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = report.GetSubscriptionId(&subscriptionId);
    if (CHIP_NO_ERROR == err)
    {
        if (IsInitialReport())
        {
            mSubscriptionId = subscriptionId;
        }
        else if (!IsMatchingClient(subscriptionId))
        {
            err = CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (CHIP_END_OF_TLV == err)
    {
        if (IsSubscriptionType())
        {
            err = CHIP_ERROR_INVALID_ARGUMENT;
        }
        else
        {
            err = CHIP_NO_ERROR;
        }
    }
    SuccessOrExit(err);

    err = report.GetMoreChunkedMessages(&mPendingMoreChunks);
    if (CHIP_END_OF_TLV == err)
    {
        mPendingMoreChunks = false;
        err                = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err                   = report.GetEventReports(&EventReports);
    isEventReportsPresent = (err == CHIP_NO_ERROR);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    if (isEventReportsPresent && nullptr != mpCallback)
    {
        chip::TLV::TLVReader EventReportsReader;
        EventReports.GetReader(&EventReportsReader);
        err = ProcessEventReportIBs(EventReportsReader);
        SuccessOrExit(err);
    }

    err                         = report.GetAttributeReportIBs(&attributeReportIBs);
    isAttributeReportIBsPresent = (err == CHIP_NO_ERROR);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    if (isAttributeReportIBsPresent && nullptr != mpCallback)
    {
        TLV::TLVReader attributeReportIBsReader;
        attributeReportIBs.GetReader(&attributeReportIBsReader);

        if (IsInitialReport())
        {
            mpCallback->OnReportBegin(this);
        }

        err = ProcessAttributeReportIBs(attributeReportIBsReader);
        SuccessOrExit(err);

        if (!mPendingMoreChunks)
        {
            mpCallback->OnReportEnd(this);
        }
    }

exit:
    if (IsSubscriptionType())
    {
        if (IsAwaitingInitialReport())
        {
            MoveToState(ClientState::AwaitingSubscribeResponse);
        }
        else
        {
            RefreshLivenessCheckTimer();
        }
    }

    if (!suppressResponse)
    {
        bool noResponseExpected = IsSubscriptionIdle() && !mPendingMoreChunks;
        err = StatusResponse::SendStatusResponse(err == CHIP_NO_ERROR ? Protocols::InteractionModel::Status::Success
                                                                      : Protocols::InteractionModel::Status::InvalidSubscription,
                                                 mpExchangeCtx, !noResponseExpected);

        if (noResponseExpected || (err != CHIP_NO_ERROR))
        {
            mpExchangeCtx = nullptr;
        }
    }

    mInitialReport = false;
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive report data from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));
    ShutdownInternal(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR ReadClient::ProcessAttributePath(AttributePathIB::Parser & aAttributePath, ClusterInfo & aClusterInfo)
{
    CHIP_ERROR err = aAttributePath.GetNode(&(aClusterInfo.mNodeId));
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    // The ReportData must contain a concrete attribute path
    err = aAttributePath.GetEndpoint(&(aClusterInfo.mEndpointId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
    err = aAttributePath.GetCluster(&(aClusterInfo.mClusterId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
    err = aAttributePath.GetAttribute(&(aClusterInfo.mAttributeId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

    err = aAttributePath.GetListIndex(&(aClusterInfo.mListIndex));
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
    VerifyOrReturnError(aClusterInfo.IsValidAttributePath(), CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::ProcessAttributeReportIBs(TLV::TLVReader & aAttributeReportIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = aAttributeReportIBsReader.Next()))
    {
        TLV::TLVReader dataReader;
        AttributeReportIB::Parser report;
        AttributeDataIB::Parser data;
        AttributeStatusIB::Parser status;
        AttributePathIB::Parser path;
        ClusterInfo clusterInfo;
        StatusIB statusIB;

        TLV::TLVReader reader = aAttributeReportIBsReader;
        ReturnErrorOnFailure(report.Init(reader));

        err = report.GetAttributeStatus(&status);
        if (CHIP_NO_ERROR == err)
        {
            StatusIB::Parser errorStatus;
            ReturnErrorOnFailure(status.GetPath(&path));
            ReturnErrorOnFailure(ProcessAttributePath(path, clusterInfo));
            ReturnErrorOnFailure(status.GetErrorStatus(&errorStatus));
            ReturnErrorOnFailure(errorStatus.DecodeStatusIB(statusIB));
            mpCallback->OnAttributeData(
                this, ConcreteDataAttributePath(clusterInfo.mEndpointId, clusterInfo.mClusterId, clusterInfo.mAttributeId), nullptr,
                statusIB);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            ReturnErrorOnFailure(report.GetAttributeData(&data));
            ReturnErrorOnFailure(data.GetPath(&path));
            ReturnErrorOnFailure(ProcessAttributePath(path, clusterInfo));
            ReturnErrorOnFailure(data.GetData(&dataReader));

            ConcreteDataAttributePath attributePath(clusterInfo.mEndpointId, clusterInfo.mClusterId, clusterInfo.mAttributeId);

            //
            // TODO: Add support for correctly handling appends/updates whenever list chunking support
            // on the server side is added.
            //
            if (dataReader.GetType() == TLV::kTLVType_Array)
            {
                attributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            }

            mpCallback->OnAttributeData(this, attributePath, &dataReader, statusIB);
        }
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ReadClient::ProcessEventReportIBs(TLV::TLVReader & aEventReportIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = aEventReportIBsReader.Next()))
    {
        TLV::TLVReader dataReader;
        EventReportIB::Parser report;
        EventDataIB::Parser data;
        EventHeader header;

        TLV::TLVReader reader = aEventReportIBsReader;
        ReturnErrorOnFailure(report.Init(reader));

        ReturnErrorOnFailure(report.GetEventData(&data));

        header.mTimestamp = mEventTimestamp;
        ReturnErrorOnFailure(data.DecodeEventHeader(header));
        mEventTimestamp = header.mTimestamp;

        ReturnErrorOnFailure(data.GetData(&dataReader));

        mpCallback->OnEventData(this, header, &dataReader, nullptr);
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ReadClient::RefreshLivenessCheckTimer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CancelLivenessCheckTimer();
    VerifyOrReturnError(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    System::Clock::Timeout timeout = System::Clock::Seconds16(mMaxIntervalCeilingSeconds) + mpExchangeCtx->GetAckTimeout();
    // EFR32/MBED/INFINION/K32W's chrono count return long unsinged, but other platform returns unsigned
    ChipLogProgress(DataManagement, "Refresh LivenessCheckTime with %lu milliseconds", static_cast<long unsigned>(timeout.count()));
    err = InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        timeout, OnLivenessTimeoutCallback, this);

    if (err != CHIP_NO_ERROR)
    {
        ShutdownInternal(err);
    }
    return err;
}

void ReadClient::CancelLivenessCheckTimer()
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnLivenessTimeoutCallback, this);
}

void ReadClient::OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadClient * const client = reinterpret_cast<ReadClient *>(apAppState);
    if (client->IsFree())
    {
        ChipLogError(DataManagement,
                     "ReadClient::OnLivenessTimeoutCallback invoked on a free client! This is a bug in CHIP stack!");
        return;
    }

    ChipLogError(DataManagement, "Subscription Liveness timeout with peer node 0x%" PRIx64 ", shutting down ", client->mPeerNodeId);
    client->mpExchangeCtx = nullptr;
    // TODO: add a more specific error here for liveness timeout failure to distinguish between other classes of timeouts (i.e
    // response timeouts).
    client->ShutdownInternal(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR ReadClient::ProcessSubscribeResponse(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));
    ReturnLogErrorOnFailure(reader.Next());

    SubscribeResponseMessage::Parser subscribeResponse;
    ReturnLogErrorOnFailure(subscribeResponse.Init(reader));

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnLogErrorOnFailure(subscribeResponse.CheckSchemaValidity());
#endif

    uint64_t subscriptionId = 0;
    ReturnLogErrorOnFailure(subscribeResponse.GetSubscriptionId(&subscriptionId));
    VerifyOrReturnLogError(IsMatchingClient(subscriptionId), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnLogErrorOnFailure(subscribeResponse.GetMinIntervalFloorSeconds(&mMinIntervalFloorSeconds));
    ReturnLogErrorOnFailure(subscribeResponse.GetMaxIntervalCeilingSeconds(&mMaxIntervalCeilingSeconds));

    if (mpCallback != nullptr)
    {
        mpCallback->OnSubscriptionEstablished(this);
    }

    MoveToState(ClientState::SubscriptionActive);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::SendSubscribeRequest(ReadPrepareParams & aReadPrepareParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    SubscribeRequestMessage::Builder request;
    VerifyOrExit(ClientState::Initialized == mState, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpCallback != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    writer.Init(std::move(msgBuf));

    err = request.Init(&writer);
    SuccessOrExit(err);

    if (aReadPrepareParams.mEventPathParamsListSize != 0 && aReadPrepareParams.mpEventPathParamsList != nullptr)
    {
        EventPaths::Builder & eventPathListBuilder = request.CreateEventRequests();
        SuccessOrExit(err = eventPathListBuilder.GetError());
        err = GenerateEventPaths(eventPathListBuilder, aReadPrepareParams.mpEventPathParamsList,
                                 aReadPrepareParams.mEventPathParamsListSize);
        SuccessOrExit(err);

        if (aReadPrepareParams.mEventNumber != 0)
        {
            // EventNumber is optional
            EventFilters::Builder eventFilters = request.CreateEventFilters();
            SuccessOrExit(err = request.GetError());
            EventFilterIB::Builder eventFilter = eventFilters.CreateEventFilter();
            eventFilter.EventMin(aReadPrepareParams.mEventNumber).EndOfEventFilterIB();
            SuccessOrExit(err = eventFilter.GetError());
            eventFilters.EndOfEventFilters();
            SuccessOrExit(err = eventFilters.GetError());
        }
    }

    if (aReadPrepareParams.mAttributePathParamsListSize != 0 && aReadPrepareParams.mpAttributePathParamsList != nullptr)
    {
        AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
        SuccessOrExit(err = attributePathListBuilder.GetError());
        err = GenerateAttributePathList(attributePathListBuilder, aReadPrepareParams.mpAttributePathParamsList,
                                        aReadPrepareParams.mAttributePathParamsListSize);
        SuccessOrExit(err);
    }

    VerifyOrExit(aReadPrepareParams.mMinIntervalFloorSeconds < aReadPrepareParams.mMaxIntervalCeilingSeconds,
                 err = CHIP_ERROR_INVALID_ARGUMENT);
    request.MinIntervalFloorSeconds(aReadPrepareParams.mMinIntervalFloorSeconds)
        .MaxIntervalCeilingSeconds(aReadPrepareParams.mMaxIntervalCeilingSeconds)
        .KeepSubscriptions(aReadPrepareParams.mKeepSubscriptions)
        .IsFabricFiltered(false)
        .EndOfSubscribeRequestMessage();
    SuccessOrExit(err = request.GetError());

    err = writer.Finalize(&msgBuf);
    SuccessOrExit(err);

    mpExchangeCtx = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHandle, this);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);
    mpExchangeCtx->SetResponseTimeout(kImMessageTimeout);
    if (mpExchangeCtx->IsBLETransport())
    {
        ChipLogError(DataManagement, "IM Subscribe cannot work with BLE");
        SuccessOrExit(err = CHIP_ERROR_INCORRECT_STATE);
    }

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeRequest, std::move(msgBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    mPeerNodeId  = aReadPrepareParams.mSessionHandle.GetPeerNodeId();
    mFabricIndex = aReadPrepareParams.mSessionHandle.GetFabricIndex();
    MoveToState(ClientState::AwaitingInitialReport);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to send subscribe request: %" CHIP_ERROR_FORMAT, err.Format());
        Shutdown();
    }
    return err;
}

}; // namespace app
}; // namespace chip
