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

#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelHelper.h>
#include <app/ReadClient.h>
#include <app/StatusResponse.h>
#include <assert.h>
#include <lib/core/CHIPTLVTypes.h>
#include <lib/support/FibonacciUtils.h>

namespace chip {
namespace app {

/**
 * @brief The default resubscribe policy will pick a random timeslot
 * with millisecond resolution over an ever increasing window,
 * following a fibonacci sequence up to CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX,
 * Average of the randomized wait time past the CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX
 * will be around one hour.
 * When the retry count resets to 0, the sequence starts from the beginning again.
 */
static void DefaultResubscribePolicy(uint32_t aNumCumulativeRetries, uint32_t & aNextSubscriptionIntervalMsec,
                                     bool & aShouldResubscribe)
{
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec    = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (aNumCumulativeRetries <= CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX)
    {
        maxWaitTimeInMsec = GetFibonacciForIndex(aNumCumulativeRetries) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS;
    }
    else
    {
        maxWaitTimeInMsec = CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS;
    }

    if (maxWaitTimeInMsec != 0)
    {
        minWaitTimeInMsec = (CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP * maxWaitTimeInMsec) / 100;
        waitTimeInMsec    = minWaitTimeInMsec + (Crypto::GetRandU32() % (maxWaitTimeInMsec - minWaitTimeInMsec));
    }

    aNextSubscriptionIntervalMsec = waitTimeInMsec;
    aShouldResubscribe            = true;
    ChipLogProgress(DataManagement,
                    "Computing Resubscribe policy: attempts %" PRIu32 ", max wait time %" PRIu32 " ms, selected wait time %" PRIu32
                    " ms",
                    aNumCumulativeRetries, maxWaitTimeInMsec, waitTimeInMsec);
}

ReadClient::ReadClient(InteractionModelEngine * apImEngine, Messaging::ExchangeManager * apExchangeMgr, Callback & apCallback,
                       InteractionType aInteractionType) :
    mpCallback(apCallback)
{
    // Error if already initialized.
    mpExchangeMgr    = apExchangeMgr;
    mpCallback       = apCallback;
    mInteractionType = aInteractionType;

    mpImEngine = apImEngine;

    if (aInteractionType == InteractionType::Subscribe)
    {
        mpImEngine->AddReadClient(this);
    }
}

void ReadClient::ClearActiveSubscriptionState()
{
    mIsInitialReport           = true;
    mIsPrimingReports          = true;
    mPendingMoreChunks         = false;
    mMinIntervalFloorSeconds   = 0;
    mMaxIntervalCeilingSeconds = 0;
    mSubscriptionId            = 0;
    MoveToState(ClientState::Idle);
}

void ReadClient::StopResubscription()
{
    ClearActiveSubscriptionState();
    CancelLivenessCheckTimer();
    CancelResubscribeTimer();
    mpCallback.OnDeallocatePaths(std::move(mReadPrepareParams));
}

ReadClient::~ReadClient()
{
    Abort();

    if (IsSubscriptionType())
    {
        CancelLivenessCheckTimer();
        //
        // Only remove ourselves from the engine's tracker list if we still continue to have a valid pointer to it.
        // This won't be the case if the engine shut down before this destructor was called (in which case, mpImEngine
        // will point to null)
        //
        if (mpImEngine)
        {
            mpImEngine->RemoveReadClient(this);
        }
    }
}

void ReadClient::Close(CHIP_ERROR aError)
{
    // OnDone below can destroy us before we unwind all the way back into the
    // exchange code and it tries to close itself.  Make sure that it doesn't
    // try to notify us that it's closing, since we will be dead.
    //
    // For more details, see #10344.
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->SetDelegate(nullptr);
    }

    mpExchangeCtx = nullptr;

    if (IsReadType())
    {
        if (aError != CHIP_NO_ERROR)
        {
            mpCallback.OnError(aError);
        }
    }
    else
    {
        if (aError != CHIP_NO_ERROR)
        {
            uint32_t nextResubscribeMsec = 0;

            if (ResubscribeIfNeeded(nextResubscribeMsec))
            {
                ChipLogProgress(DataManagement,
                                "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
                                "ms due to error %" CHIP_ERROR_FORMAT,
                                mFabricIndex, ChipLogValueX64(mPeerNodeId), mNumRetries, nextResubscribeMsec, aError.Format());
                mpCallback.OnResubscriptionAttempt(aError, nextResubscribeMsec);
                ClearActiveSubscriptionState();
                return;
            }
            mpCallback.OnError(aError);
        }
        StopResubscription();
    }

    mpCallback.OnDone(this);
}

const char * ReadClient::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case ClientState::Idle:
        return "Idle";
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
    ChipLogDetail(DataManagement, "%s ReadClient[%p]: Moving to [%10.10s]", __func__, this, GetStateStr());
}

CHIP_ERROR ReadClient::SendRequest(ReadPrepareParams & aReadPrepareParams)
{
    if (mInteractionType == InteractionType::Read)
    {
        return SendReadRequest(aReadPrepareParams);
    }

    if (mInteractionType == InteractionType::Subscribe)
    {
        return SendSubscribeRequest(aReadPrepareParams);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ReadClient::SendReadRequest(ReadPrepareParams & aReadPrepareParams)
{
    // TODO: SendRequest parameter is too long, need to have the structure to represent it
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "%s ReadClient[%p]: Sending Read Request", __func__, this);

    VerifyOrReturnError(ClientState::Idle == mState, err = CHIP_ERROR_INCORRECT_STATE);

    Span<AttributePathParams> attributePaths(aReadPrepareParams.mpAttributePathParamsList,
                                             aReadPrepareParams.mAttributePathParamsListSize);
    Span<EventPathParams> eventPaths(aReadPrepareParams.mpEventPathParamsList, aReadPrepareParams.mEventPathParamsListSize);
    Span<DataVersionFilter> dataVersionFilters(aReadPrepareParams.mpDataVersionFilterList,
                                               aReadPrepareParams.mDataVersionFilterListSize);

    System::PacketBufferHandle msgBuf;
    ReadRequestMessage::Builder request;
    System::PacketBufferTLVWriter writer;

    InitWriterWithSpaceReserved(writer, kReservedSizeForTLVEncodingOverhead);
    ReturnErrorOnFailure(request.Init(&writer));

    if (!attributePaths.empty())
    {
        AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
        ReturnErrorOnFailure(err = request.GetError());
        ReturnErrorOnFailure(GenerateAttributePaths(attributePathListBuilder, attributePaths));
    }

    if (!eventPaths.empty())
    {
        EventPathIBs::Builder & eventPathListBuilder = request.CreateEventRequests();
        ReturnErrorOnFailure(err = request.GetError());

        ReturnErrorOnFailure(GenerateEventPaths(eventPathListBuilder, eventPaths));

        Optional<EventNumber> eventMin;
        ReturnErrorOnFailure(GetMinEventNumber(aReadPrepareParams, eventMin));
        if (eventMin.HasValue())
        {
            EventFilterIBs::Builder & eventFilters = request.CreateEventFilters();
            ReturnErrorOnFailure(err = request.GetError());
            ReturnErrorOnFailure(eventFilters.GenerateEventFilter(eventMin.Value()));
        }
    }

    ReturnErrorOnFailure(request.IsFabricFiltered(aReadPrepareParams.mIsFabricFiltered).GetError());

    bool encodedDataVersionList = false;
    TLV::TLVWriter backup;
    request.Checkpoint(backup);
    DataVersionFilterIBs::Builder & dataVersionFilterListBuilder = request.CreateDataVersionFilters();
    ReturnErrorOnFailure(request.GetError());
    if (!attributePaths.empty())
    {
        ReturnErrorOnFailure(GenerateDataVersionFilterList(dataVersionFilterListBuilder, attributePaths, dataVersionFilters,
                                                           encodedDataVersionList));
    }
    ReturnErrorOnFailure(dataVersionFilterListBuilder.GetWriter()->UnreserveBuffer(kReservedSizeForTLVEncodingOverhead));
    if (encodedDataVersionList)
    {
        ReturnErrorOnFailure(dataVersionFilterListBuilder.EndOfDataVersionFilterIBs().GetError());
    }
    else
    {
        request.Rollback(backup);
    }

    ReturnErrorOnFailure(request.EndOfReadRequestMessage().GetError());
    ReturnErrorOnFailure(writer.Finalize(&msgBuf));

    VerifyOrReturnError(aReadPrepareParams.mSessionHolder, CHIP_ERROR_MISSING_SECURE_SESSION);

    mpExchangeCtx = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHolder.Get().Value(), this);
    VerifyOrReturnError(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (aReadPrepareParams.mTimeout == System::Clock::kZero)
    {
        mpExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }
    else
    {
        mpExchangeCtx->SetResponseTimeout(aReadPrepareParams.mTimeout);
    }

    ReturnErrorOnFailure(mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)));

    mPeerNodeId  = aReadPrepareParams.mSessionHolder->AsSecureSession()->GetPeerNodeId();
    mFabricIndex = aReadPrepareParams.mSessionHolder->GetFabricIndex();

    MoveToState(ClientState::AwaitingInitialReport);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::GenerateEventPaths(EventPathIBs::Builder & aEventPathsBuilder, const Span<EventPathParams> & aEventPaths)
{
    for (auto & event : aEventPaths)
    {
        VerifyOrReturnError(event.IsValidEventPath(), CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
        EventPathIB::Builder & path = aEventPathsBuilder.CreatePath();
        ReturnErrorOnFailure(aEventPathsBuilder.GetError());
        ReturnErrorOnFailure(path.Encode(event));
    }

    aEventPathsBuilder.EndOfEventPaths();
    return aEventPathsBuilder.GetError();
}

CHIP_ERROR ReadClient::GenerateAttributePaths(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                              const Span<AttributePathParams> & aAttributePaths)
{
    for (auto & attribute : aAttributePaths)
    {
        VerifyOrReturnError(attribute.IsValidAttributePath(), CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
        AttributePathIB::Builder & path = aAttributePathIBsBuilder.CreatePath();
        ReturnErrorOnFailure(aAttributePathIBsBuilder.GetError());
        ReturnErrorOnFailure(path.Encode(attribute));
    }

    aAttributePathIBsBuilder.EndOfAttributePathIBs();
    return aAttributePathIBsBuilder.GetError();
}

CHIP_ERROR ReadClient::BuildDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                  const Span<AttributePathParams> & aAttributePaths,
                                                  const Span<DataVersionFilter> & aDataVersionFilters,
                                                  bool & aEncodedDataVersionList)
{
    for (auto & filter : aDataVersionFilters)
    {
        VerifyOrReturnError(filter.IsValidDataVersionFilter(), CHIP_ERROR_INVALID_ARGUMENT);

        // If data version filter is for some cluster none of whose attributes are included in our paths, discard this filter.
        bool intersected = false;
        for (auto & path : aAttributePaths)
        {
            if (path.IncludesAttributesInCluster(filter))
            {
                intersected = true;
                break;
            }
        }
        if (!intersected)
        {
            continue;
        }

        DataVersionFilterIB::Builder & filterIB = aDataVersionFilterIBsBuilder.CreateDataVersionFilter();
        ReturnErrorOnFailure(aDataVersionFilterIBsBuilder.GetError());
        ClusterPathIB::Builder & path = filterIB.CreatePath();
        ReturnErrorOnFailure(filterIB.GetError());
        ReturnErrorOnFailure(path.Endpoint(filter.mEndpointId).Cluster(filter.mClusterId).EndOfClusterPathIB().GetError());
        VerifyOrReturnError(filter.mDataVersion.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(filterIB.DataVersion(filter.mDataVersion.Value()).EndOfDataVersionFilterIB().GetError());
        aEncodedDataVersionList = true;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::GenerateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                     const Span<AttributePathParams> & aAttributePaths,
                                                     const Span<DataVersionFilter> & aDataVersionFilters,
                                                     bool & aEncodedDataVersionList)
{
    if (!aDataVersionFilters.empty())
    {
        ReturnErrorOnFailure(BuildDataVersionFilterList(aDataVersionFilterIBsBuilder, aAttributePaths, aDataVersionFilters,
                                                        aEncodedDataVersionList));
    }
    else
    {
        ReturnErrorOnFailure(
            mpCallback.OnUpdateDataVersionFilterList(aDataVersionFilterIBsBuilder, aAttributePaths, aEncodedDataVersionList));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                         System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!IsIdle(), err = CHIP_ERROR_INCORRECT_STATE);

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
        VerifyOrExit(apExchangeContext == mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
        err = StatusResponse::ProcessStatusResponse(std::move(aPayload));
        SuccessOrExit(err);
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    if ((!IsSubscriptionType() && !mPendingMoreChunks) || err != CHIP_NO_ERROR)
    {
        Close(err);
    }

    return err;
}

void ReadClient::Abort()
{
    //
    // If the exchange context hasn't already been gracefully closed
    // (signaled by setting it to null), then we need to forcibly
    // tear it down.
    //
    if (mpExchangeCtx != nullptr)
    {
        // We might be a delegate for this exchange, and we don't want the
        // OnExchangeClosing notification in that case.  Null out the delegate
        // to avoid that.
        //
        // TODO: This makes all sorts of assumptions about what the delegate is
        // (notice the "might" above!) that might not hold in practice.  We
        // really need a better solution here....
        mpExchangeCtx->SetDelegate(nullptr);
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
}

CHIP_ERROR ReadClient::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                               System::PacketBufferHandle && aPayload)
{
    mpExchangeCtx = apExchangeContext;

    //
    // Let's take over further message processing on this exchange from the IM.
    // This is only relevant for reports during post-subscription.
    //
    mpExchangeCtx->SetDelegate(this);

    CHIP_ERROR err = ProcessReportData(std::move(aPayload));
    if (err != CHIP_NO_ERROR)
    {
        Close(err);
    }

    return err;
}

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Parser report;

    bool suppressResponse         = true;
    SubscriptionId subscriptionId = 0;
    EventReportIBs::Parser eventReportIBs;
    AttributeReportIBs::Parser attributeReportIBs;
    System::PacketBufferTLVReader reader;

    reader.Init(std::move(aPayload));
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
        if (mIsPrimingReports)
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

    err = report.GetEventReports(&eventReportIBs);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        chip::TLV::TLVReader EventReportsReader;
        eventReportIBs.GetReader(&EventReportsReader);
        err = ProcessEventReportIBs(EventReportsReader);
    }
    SuccessOrExit(err);

    err = report.GetAttributeReportIBs(&attributeReportIBs);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        TLV::TLVReader attributeReportIBsReader;
        mSawAttributeReportsInCurrentReport = true;
        attributeReportIBs.GetReader(&attributeReportIBsReader);

        if (mIsInitialReport)
        {
            mpCallback.OnReportBegin();
            mIsInitialReport = false;
        }

        err = ProcessAttributeReportIBs(attributeReportIBsReader);
    }
    SuccessOrExit(err);

    if (mSawAttributeReportsInCurrentReport && !mPendingMoreChunks)
    {
        mpCallback.OnReportEnd();
        mIsInitialReport                    = true;
        mSawAttributeReportsInCurrentReport = false;
    }

    SuccessOrExit(err = report.ExitContainer());

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
        bool noResponseExpected = IsSubscriptionActive() && !mPendingMoreChunks;
        err                     = StatusResponse::Send(err == CHIP_NO_ERROR ? Protocols::InteractionModel::Status::Success
                                                        : Protocols::InteractionModel::Status::InvalidSubscription,
                                   mpExchangeCtx, !noResponseExpected);

        if (noResponseExpected || (err != CHIP_NO_ERROR))
        {
            mpExchangeCtx = nullptr;
        }
    }

    mIsPrimingReports = false;
    return err;
}

void ReadClient::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogError(DataManagement, "Time out! failed to receive report data from Exchange: " ChipLogFormatExchange,
                 ChipLogValueExchange(apExchangeContext));
    Close(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR ReadClient::ProcessAttributePath(AttributePathIB::Parser & aAttributePathParser,
                                            ConcreteDataAttributePath & aAttributePath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // The ReportData must contain a concrete attribute path
    err = aAttributePathParser.GetEndpoint(&(aAttributePath.mEndpointId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
    err = aAttributePathParser.GetCluster(&(aAttributePath.mClusterId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
    err = aAttributePathParser.GetAttribute(&(aAttributePath.mAttributeId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
    err = aAttributePathParser.GetListIndex(aAttributePath);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
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
        ConcreteDataAttributePath attributePath;
        StatusIB statusIB;

        TLV::TLVReader reader = aAttributeReportIBsReader;
        ReturnErrorOnFailure(report.Init(reader));

        err = report.GetAttributeStatus(&status);
        if (CHIP_NO_ERROR == err)
        {
            StatusIB::Parser errorStatus;
            ReturnErrorOnFailure(status.GetPath(&path));
            ReturnErrorOnFailure(ProcessAttributePath(path, attributePath));
            ReturnErrorOnFailure(status.GetErrorStatus(&errorStatus));
            ReturnErrorOnFailure(errorStatus.DecodeStatusIB(statusIB));
            mpCallback.OnAttributeData(attributePath, nullptr, statusIB);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            ReturnErrorOnFailure(report.GetAttributeData(&data));
            ReturnErrorOnFailure(data.GetPath(&path));
            ReturnErrorOnFailure(ProcessAttributePath(path, attributePath));
            DataVersion version = 0;
            ReturnErrorOnFailure(data.GetDataVersion(&version));
            attributePath.mDataVersion.SetValue(version);
            if (mReadPrepareParams.mResubscribePolicy != nullptr)
            {
                UpdateDataVersionFilters(attributePath);
            }

            ReturnErrorOnFailure(data.GetData(&dataReader));

            // The element in an array may be another array -- so we should only set the list operation when we are handling the
            // whole list.
            if (!attributePath.IsListOperation() && dataReader.GetType() == TLV::kTLVType_Array)
            {
                attributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            }

            mpCallback.OnAttributeData(attributePath, &dataReader, statusIB);
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
        StatusIB statusIB; // Default value for statusIB is success.

        TLV::TLVReader reader = aEventReportIBsReader;
        ReturnErrorOnFailure(report.Init(reader));

        err = report.GetEventData(&data);

        if (err == CHIP_NO_ERROR)
        {
            header.mTimestamp = mEventTimestamp;
            ReturnErrorOnFailure(data.DecodeEventHeader(header));
            mEventTimestamp = header.mTimestamp;

            ReturnErrorOnFailure(data.GetData(&dataReader));

            if (mReadPrepareParams.mResubscribePolicy != nullptr)
            {
                mReadPrepareParams.mEventNumber.SetValue(header.mEventNumber + 1);
            }

            mpCallback.OnEventData(header, &dataReader, nullptr);
        }
        else if (err == CHIP_END_OF_TLV)
        {
            EventStatusIB::Parser status;
            EventPathIB::Parser pathIB;
            StatusIB::Parser statusIBParser;
            ReturnErrorOnFailure(report.GetEventStatus(&status));
            ReturnErrorOnFailure(status.GetPath(&pathIB));
            ReturnErrorOnFailure(pathIB.GetEventPath(&header.mPath));
            ReturnErrorOnFailure(status.GetErrorStatus(&statusIBParser));
            ReturnErrorOnFailure(statusIBParser.DecodeStatusIB(statusIB));

            mpCallback.OnEventData(header, nullptr, &statusIB);
        }
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
    VerifyOrReturnError(mpExchangeCtx->HasSessionHandle(), err = CHIP_ERROR_INCORRECT_STATE);

    System::Clock::Timeout timeout =
        System::Clock::Seconds16(mMaxIntervalCeilingSeconds) + mpExchangeCtx->GetSessionHandle()->GetAckTimeout();
    // EFR32/MBED/INFINION/K32W's chrono count return long unsinged, but other platform returns unsigned
    ChipLogProgress(DataManagement,
                    "Refresh LivenessCheckTime for %lu milliseconds with SubscriptionId = 0x%08" PRIx32
                    " Peer = %02x:" ChipLogFormatX64,
                    static_cast<long unsigned>(timeout.count()), mSubscriptionId, mFabricIndex, ChipLogValueX64(mPeerNodeId));
    err = InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        timeout, OnLivenessTimeoutCallback, this);

    if (err != CHIP_NO_ERROR)
    {
        Close(err);
    }

    return err;
}

void ReadClient::CancelLivenessCheckTimer()
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnLivenessTimeoutCallback, this);
}

void ReadClient::CancelResubscribeTimer()
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnResubscribeTimerCallback, this);
}

void ReadClient::OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadClient * const _this = reinterpret_cast<ReadClient *>(apAppState);

    //
    // Might as well try to see if this instance exists in the tracked list in the IM.
    // This might blow-up if either the client has since been free'ed (use-after-free), or if the engine has since
    // been shutdown at which point the client wouldn't exist in the active read client list.
    //
    VerifyOrDie(_this->mpImEngine->InActiveReadClientList(_this));

    ChipLogError(DataManagement,
                 "Subscription Liveness timeout with SubscriptionID = 0x%08" PRIx32 ", Peer = %02x:" ChipLogFormatX64,
                 _this->mSubscriptionId, _this->mFabricIndex, ChipLogValueX64(_this->mPeerNodeId));

    // TODO: add a more specific error here for liveness timeout failure to distinguish between other classes of timeouts (i.e
    // response timeouts).
    _this->Close(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR ReadClient::ProcessSubscribeResponse(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));

    SubscribeResponseMessage::Parser subscribeResponse;
    ReturnErrorOnFailure(subscribeResponse.Init(reader));

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(subscribeResponse.CheckSchemaValidity());
#endif

    SubscriptionId subscriptionId = 0;
    ReturnErrorOnFailure(subscribeResponse.GetSubscriptionId(&subscriptionId));
    VerifyOrReturnError(IsMatchingClient(subscriptionId), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(subscribeResponse.GetMinIntervalFloorSeconds(&mMinIntervalFloorSeconds));
    ReturnErrorOnFailure(subscribeResponse.GetMaxIntervalCeilingSeconds(&mMaxIntervalCeilingSeconds));

    ChipLogProgress(DataManagement,
                    "Subscription established with SubscriptionID = 0x%08" PRIx32 " MinInterval = %u"
                    "s MaxInterval = %us Peer = %02x:" ChipLogFormatX64,
                    mSubscriptionId, mMinIntervalFloorSeconds, mMaxIntervalCeilingSeconds, mFabricIndex,
                    ChipLogValueX64(mPeerNodeId));

    ReturnErrorOnFailure(subscribeResponse.ExitContainer());

    MoveToState(ClientState::SubscriptionActive);

    mpCallback.OnSubscriptionEstablished(subscriptionId);

    if (mReadPrepareParams.mResubscribePolicy != nullptr)
    {
        mNumRetries = 0;
    }

    RefreshLivenessCheckTimer();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::SendAutoResubscribeRequest(ReadPrepareParams && aReadPrepareParams)
{
    mReadPrepareParams = std::move(aReadPrepareParams);
    if (mReadPrepareParams.mResubscribePolicy == nullptr)
    {
        mReadPrepareParams.mResubscribePolicy = DefaultResubscribePolicy;
    }

    CHIP_ERROR err = SendSubscribeRequest(mReadPrepareParams);
    if (err != CHIP_NO_ERROR)
    {
        StopResubscription();
    }
    return err;
}

CHIP_ERROR ReadClient::SendSubscribeRequest(const ReadPrepareParams & aReadPrepareParams)
{
    VerifyOrReturnError(aReadPrepareParams.mMinIntervalFloorSeconds <= aReadPrepareParams.mMaxIntervalCeilingSeconds,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return SendSubscribeRequestImpl(aReadPrepareParams);
}

CHIP_ERROR ReadClient::SendSubscribeRequestImpl(const ReadPrepareParams & aReadPrepareParams)
{
    VerifyOrReturnError(ClientState::Idle == mState, CHIP_ERROR_INCORRECT_STATE);

    // Todo: Remove the below, Update span in ReadPrepareParams
    Span<AttributePathParams> attributePaths(aReadPrepareParams.mpAttributePathParamsList,
                                             aReadPrepareParams.mAttributePathParamsListSize);
    Span<EventPathParams> eventPaths(aReadPrepareParams.mpEventPathParamsList, aReadPrepareParams.mEventPathParamsListSize);
    Span<DataVersionFilter> dataVersionFilters(aReadPrepareParams.mpDataVersionFilterList,
                                               aReadPrepareParams.mDataVersionFilterListSize);

    System::PacketBufferHandle msgBuf;
    System::PacketBufferTLVWriter writer;
    SubscribeRequestMessage::Builder request;
    InitWriterWithSpaceReserved(writer, kReservedSizeForTLVEncodingOverhead);

    ReturnErrorOnFailure(request.Init(&writer));

    request.KeepSubscriptions(aReadPrepareParams.mKeepSubscriptions)
        .MinIntervalFloorSeconds(aReadPrepareParams.mMinIntervalFloorSeconds)
        .MaxIntervalCeilingSeconds(aReadPrepareParams.mMaxIntervalCeilingSeconds);

    if (!attributePaths.empty())
    {
        AttributePathIBs::Builder & attributePathListBuilder = request.CreateAttributeRequests();
        ReturnErrorOnFailure(attributePathListBuilder.GetError());
        ReturnErrorOnFailure(GenerateAttributePaths(attributePathListBuilder, attributePaths));
    }

    if (!eventPaths.empty())
    {
        EventPathIBs::Builder & eventPathListBuilder = request.CreateEventRequests();
        ReturnErrorOnFailure(eventPathListBuilder.GetError());
        ReturnErrorOnFailure(GenerateEventPaths(eventPathListBuilder, eventPaths));

        Optional<EventNumber> eventMin;
        ReturnErrorOnFailure(GetMinEventNumber(aReadPrepareParams, eventMin));
        if (eventMin.HasValue())
        {
            EventFilterIBs::Builder & eventFilters = request.CreateEventFilters();
            ReturnErrorOnFailure(request.GetError());
            ReturnErrorOnFailure(eventFilters.GenerateEventFilter(eventMin.Value()));
        }
    }

    ReturnErrorOnFailure(request.IsFabricFiltered(aReadPrepareParams.mIsFabricFiltered).GetError());

    bool encodedDataVersionList = false;
    TLV::TLVWriter backup;
    request.Checkpoint(backup);
    DataVersionFilterIBs::Builder & dataVersionFilterListBuilder = request.CreateDataVersionFilters();
    ReturnErrorOnFailure(request.GetError());
    if (!attributePaths.empty())
    {
        ReturnErrorOnFailure(GenerateDataVersionFilterList(dataVersionFilterListBuilder, attributePaths, dataVersionFilters,
                                                           encodedDataVersionList));
    }
    ReturnErrorOnFailure(dataVersionFilterListBuilder.GetWriter()->UnreserveBuffer(kReservedSizeForTLVEncodingOverhead));
    if (encodedDataVersionList)
    {
        ReturnErrorOnFailure(dataVersionFilterListBuilder.EndOfDataVersionFilterIBs().GetError());
    }
    else
    {
        request.Rollback(backup);
    }

    ReturnErrorOnFailure(request.EndOfSubscribeRequestMessage().GetError());
    ReturnErrorOnFailure(writer.Finalize(&msgBuf));

    VerifyOrReturnError(aReadPrepareParams.mSessionHolder, CHIP_ERROR_MISSING_SECURE_SESSION);

    mpExchangeCtx = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHolder.Get().Value(), this);
    VerifyOrReturnError(mpExchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    if (aReadPrepareParams.mTimeout == System::Clock::kZero)
    {
        mpExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }
    else
    {
        mpExchangeCtx->SetResponseTimeout(aReadPrepareParams.mTimeout);
    }

    ReturnErrorOnFailure(mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeRequest, std::move(msgBuf),
                                                    Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)));

    mPeerNodeId  = aReadPrepareParams.mSessionHolder->AsSecureSession()->GetPeerNodeId();
    mFabricIndex = aReadPrepareParams.mSessionHolder->GetFabricIndex();

    MoveToState(ClientState::AwaitingInitialReport);

    return CHIP_NO_ERROR;
}

void ReadClient::OnResubscribeTimerCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadClient * const _this = reinterpret_cast<ReadClient *>(apAppState);
    assert(_this != nullptr);
    _this->SendSubscribeRequest(_this->mReadPrepareParams);
    _this->mNumRetries++;
}

bool ReadClient::ResubscribeIfNeeded(uint32_t & aNextResubscribeIntervalMsec)
{
    bool shouldResubscribe       = true;
    uint32_t intervalMsec        = 0;
    aNextResubscribeIntervalMsec = 0;
    if (mReadPrepareParams.mResubscribePolicy == nullptr)
    {
        ChipLogDetail(DataManagement, "mResubscribePolicy is null");
        return false;
    }
    mReadPrepareParams.mResubscribePolicy(mNumRetries, intervalMsec, shouldResubscribe);
    if (!shouldResubscribe)
    {
        ChipLogProgress(DataManagement, "Resubscribe has been stopped");
        return false;
    }
    CHIP_ERROR err = InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        System::Clock::Milliseconds32(intervalMsec), OnResubscribeTimerCallback, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Fail to resubscribe with error %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    aNextResubscribeIntervalMsec = intervalMsec;
    return true;
}

void ReadClient::UpdateDataVersionFilters(const ConcreteDataAttributePath & aPath)
{
    for (size_t index = 0; index < mReadPrepareParams.mDataVersionFilterListSize; index++)
    {
        if (mReadPrepareParams.mpDataVersionFilterList[index].mEndpointId == aPath.mEndpointId &&
            mReadPrepareParams.mpDataVersionFilterList[index].mClusterId == aPath.mClusterId)
        {
            // Now we know the current version for this cluster is aPath.mDataVersion.
            mReadPrepareParams.mpDataVersionFilterList[index].mDataVersion = aPath.mDataVersion;
        }
    }
}

CHIP_ERROR ReadClient::GetMinEventNumber(const ReadPrepareParams & aReadPrepareParams, Optional<EventNumber> & aEventMin)
{
    if (aReadPrepareParams.mEventNumber.HasValue())
    {
        aEventMin = aReadPrepareParams.mEventNumber;
    }
    else
    {
        return mpCallback.GetHighestReceivedEventNumber(aEventMin);
    }
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip
