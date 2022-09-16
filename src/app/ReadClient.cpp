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

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelHelper.h>
#include <app/ReadClient.h>
#include <app/StatusResponse.h>
#include <assert.h>
#include <lib/core/CHIPTLVTypes.h>
#include <lib/support/FibonacciUtils.h>
#include <messaging/ReliableMessageMgr.h>

namespace chip {
namespace app {

using Status = Protocols::InteractionModel::Status;

ReadClient::ReadClient(InteractionModelEngine * apImEngine, Messaging::ExchangeManager * apExchangeMgr, Callback & apCallback,
                       InteractionType aInteractionType) :
    mExchange(*this),
    mpCallback(apCallback), mOnConnectedCallback(HandleDeviceConnected, this),
    mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
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
    mIsReporting                  = false;
    mWaitingForFirstPrimingReport = true;
    mPendingMoreChunks            = false;
    mMinIntervalFloorSeconds      = 0;
    mMaxInterval                  = 0;
    mSubscriptionId               = 0;
    MoveToState(ClientState::Idle);
}

void ReadClient::StopResubscription()
{
    CancelLivenessCheckTimer();
    CancelResubscribeTimer();

    // Only deallocate the paths if they are not already deallocated.
    if (mReadPrepareParams.mpAttributePathParamsList != nullptr || mReadPrepareParams.mpEventPathParamsList != nullptr ||
        mReadPrepareParams.mpDataVersionFilterList != nullptr)
    {
        mpCallback.OnDeallocatePaths(std::move(mReadPrepareParams));
        // Make sure we will never try to free those pointers again.
        mReadPrepareParams.mpAttributePathParamsList    = nullptr;
        mReadPrepareParams.mAttributePathParamsListSize = 0;
        mReadPrepareParams.mpEventPathParamsList        = nullptr;
        mReadPrepareParams.mEventPathParamsListSize     = 0;
        mReadPrepareParams.mpDataVersionFilterList      = nullptr;
        mReadPrepareParams.mDataVersionFilterListSize   = 0;
    }
}

ReadClient::~ReadClient()
{
    if (IsSubscriptionType())
    {
        StopResubscription();

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

uint32_t ReadClient::ComputeTimeTillNextSubscription()
{
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec    = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (mNumRetries <= CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX)
    {
        maxWaitTimeInMsec = GetFibonacciForIndex(mNumRetries) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS;
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

    return waitTimeInMsec;
}

CHIP_ERROR ReadClient::ScheduleResubscription(uint32_t aTimeTillNextResubscriptionMs, Optional<SessionHandle> aNewSessionHandle,
                                              bool aReestablishCASE)
{
    VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);

    //
    // If we're establishing CASE, make sure we are not provided a new SessionHandle as well.
    //
    VerifyOrReturnError(!aReestablishCASE || !aNewSessionHandle.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

    if (aNewSessionHandle.HasValue())
    {
        mReadPrepareParams.mSessionHolder.Grab(aNewSessionHandle.Value());
    }

    mDoCaseOnNextResub = aReestablishCASE;

    ReturnErrorOnFailure(
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            System::Clock::Milliseconds32(aTimeTillNextResubscriptionMs), OnResubscribeTimerCallback, this));

    return CHIP_NO_ERROR;
}

void ReadClient::Close(CHIP_ERROR aError, bool allowResubscription)
{
    if (IsReadType())
    {
        if (aError != CHIP_NO_ERROR)
        {
            mpCallback.OnError(aError);
        }
    }
    else
    {
        ClearActiveSubscriptionState();
        if (aError != CHIP_NO_ERROR)
        {
            //
            // We infer that re-subscription was requested by virtue of having a non-zero list of event OR attribute paths present
            // in mReadPrepareParams. This would only be the case if an application called SendAutoResubscribeRequest which
            // populates mReadPrepareParams with the values provided by the application.
            //
            if (allowResubscription &&
                (mReadPrepareParams.mEventPathParamsListSize != 0 || mReadPrepareParams.mAttributePathParamsListSize != 0))
            {
                aError = mpCallback.OnResubscriptionNeeded(this, aError);
                if (aError == CHIP_NO_ERROR)
                {
                    return;
                }
            }

            //
            // Either something bad happened when requesting resubscription or the application has decided to not
            // continue by returning an error. Let's convey the error back up to the application
            // and shut everything down.
            //
            mpCallback.OnError(aError);
        }

        StopResubscription();
    }

    mExchange.Release();

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

    auto exchange = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHolder.Get().Value(), this);
    VerifyOrReturnError(exchange != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mExchange.Grab(exchange);

    if (aReadPrepareParams.mTimeout == System::Clock::kZero)
    {
        mExchange->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }
    else
    {
        mExchange->SetResponseTimeout(aReadPrepareParams.mTimeout);
    }

    ReturnErrorOnFailure(mExchange->SendMessage(Protocols::InteractionModel::MsgType::ReadRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)));

    mPeer = aReadPrepareParams.mSessionHolder->AsSecureSession()->GetPeer();
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
    Status status  = Status::InvalidAction;
    VerifyOrExit(!IsIdle(), err = CHIP_ERROR_INCORRECT_STATE);

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        err = ProcessReportData(std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeResponse))
    {
        ChipLogProgress(DataManagement, "SubscribeResponse is received");
        VerifyOrExit(apExchangeContext == mExchange.Get(), err = CHIP_ERROR_INCORRECT_STATE);
        err = ProcessSubscribeResponse(std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        VerifyOrExit(apExchangeContext == mExchange.Get(), err = CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR statusError = CHIP_NO_ERROR;
        SuccessOrExit(err = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError));
        SuccessOrExit(err = statusError);
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_INVALID_SUBSCRIPTION)
        {
            status = Status::InvalidSubscription;
        }
        StatusResponse::Send(status, apExchangeContext, false /*aExpectResponse*/);
    }

    if ((!IsSubscriptionType() && !mPendingMoreChunks) || err != CHIP_NO_ERROR)
    {
        Close(err);
    }

    return err;
}

void ReadClient::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload)
{
    Status status = Status::Success;
    mExchange.Grab(apExchangeContext);

    //
    // Let's update the session we're tracking in our SessionHolder to that associated with the message that was just received.
    // This CAN be different from the one we were tracking before, since the server is permitted to send exchanges on any valid
    // session to us, of which there could be multiple.
    //
    // Since receipt of a message is proof of a working session on the peer, it's always best to update to that if possible
    // to maximize our chances of success later.
    //
    mReadPrepareParams.mSessionHolder.Grab(mExchange->GetSessionHandle());

    CHIP_ERROR err = ProcessReportData(std::move(aPayload));
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_INVALID_SUBSCRIPTION)
        {
            status = Status::InvalidSubscription;
        }
        else
        {
            status = Status::InvalidAction;
        }

        StatusResponse::Send(status, mExchange.Get(), false /*aExpectResponse*/);
        Close(err);
    }
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
        VerifyOrExit(IsSubscriptionType(), err = CHIP_ERROR_INVALID_ARGUMENT);
        if (mWaitingForFirstPrimingReport)
        {
            mSubscriptionId = subscriptionId;
        }
        else if (!IsMatchingSubscriptionId(subscriptionId))
        {
            err = CHIP_ERROR_INVALID_SUBSCRIPTION;
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
        attributeReportIBs.GetReader(&attributeReportIBsReader);
        err = ProcessAttributeReportIBs(attributeReportIBsReader);
    }
    SuccessOrExit(err);

    if (mIsReporting && !mPendingMoreChunks)
    {
        mpCallback.OnReportEnd();
        mIsReporting = false;
    }

    SuccessOrExit(err = report.ExitContainer());

exit:
    if (IsSubscriptionType())
    {
        if (IsAwaitingInitialReport())
        {
            MoveToState(ClientState::AwaitingSubscribeResponse);
        }
        else if (IsSubscriptionActive() && err == CHIP_NO_ERROR)
        {
            //
            // Only refresh the liveness check timer if we've successfully established
            // a subscription and have a valid value for mMaxInterval which the function
            // relies on.
            //
            err = RefreshLivenessCheckTimer();
        }
    }

    if (!suppressResponse && err == CHIP_NO_ERROR)
    {
        bool noResponseExpected = IsSubscriptionActive() && !mPendingMoreChunks;
        err                     = StatusResponse::Send(Status::Success, mExchange.Get(), !noResponseExpected);
    }

    mWaitingForFirstPrimingReport = false;
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

void ReadClient::NoteReportingData()
{
    if (!mIsReporting)
    {
        mpCallback.OnReportBegin();
        mIsReporting = true;
    }
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
            NoteReportingData();
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

            if (mReadPrepareParams.mpDataVersionFilterList != nullptr)
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

            NoteReportingData();
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

            //
            // Update the event number being tracked in mReadPrepareParams in case
            // we want to send it in the next SubscribeRequest message to convey
            // the event number for which we have already received an event.
            //
            mReadPrepareParams.mEventNumber.SetValue(header.mEventNumber + 1);

            NoteReportingData();
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

            NoteReportingData();
            mpCallback.OnEventData(header, nullptr, &statusIB);
        }
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

void ReadClient::OverrideLivenessTimeout(System::Clock::Timeout aLivenessTimeout)
{
    mLivenessTimeoutOverride = aLivenessTimeout;
    auto err                 = RefreshLivenessCheckTimer();
    if (err != CHIP_NO_ERROR)
    {
        Close(err);
    }
}

CHIP_ERROR ReadClient::RefreshLivenessCheckTimer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == ClientState::SubscriptionActive, CHIP_ERROR_INCORRECT_STATE);

    CancelLivenessCheckTimer();

    System::Clock::Timeout timeout;

    if (mLivenessTimeoutOverride != System::Clock::kZero)
    {
        timeout = mLivenessTimeoutOverride;
    }
    else
    {
        VerifyOrReturnError(mReadPrepareParams.mSessionHolder, CHIP_ERROR_INCORRECT_STATE);

        //
        // To calculate the duration we're willing to wait for a report to come to us, we take into account the maximum interval of
        // the subscription AND the time it takes for the report to make it to us in the worst case. This latter bit involves
        // computing the Ack timeout from the publisher for the ReportData message being sent to us using our IDLE interval as the
        // basis for that computation.
        //
        // TODO: We need to find a good home for this logic that will correctly compute this based on transport. For now, this will
        // suffice since we don't use TCP as a transport currently and subscriptions over BLE aren't really a thing.
        //
        auto publisherTransmissionTimeout =
            GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()).mIdleRetransTimeout * (CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1);
        timeout = System::Clock::Seconds16(mMaxInterval) + publisherTransmissionTimeout;
    }

    // EFR32/MBED/INFINION/K32W's chrono count return long unsinged, but other platform returns unsigned
    ChipLogProgress(
        DataManagement,
        "Refresh LivenessCheckTime for %lu milliseconds with SubscriptionId = 0x%08" PRIx32 " Peer = %02x:" ChipLogFormatX64,
        static_cast<long unsigned>(timeout.count()), mSubscriptionId, GetFabricIndex(), ChipLogValueX64(GetPeerNodeId()));
    err = InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        timeout, OnLivenessTimeoutCallback, this);

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
                 _this->mSubscriptionId, _this->GetFabricIndex(), ChipLogValueX64(_this->GetPeerNodeId()));

    // We didn't get a message from the server on time; it's possible that it no
    // longer has a useful CASE session to us.  Mark defunct all sessions that
    // have not seen peer activity in at least as long as our session.
    const auto & holder = _this->mReadPrepareParams.mSessionHolder;
    if (holder)
    {
        System::Clock::Timestamp lastPeerActivity = holder->AsSecureSession()->GetLastPeerActivityTime();
        _this->mpImEngine->GetExchangeManager()->GetSessionManager()->ForEachMatchingSession(
            _this->mPeer, [&lastPeerActivity](auto * session) {
                if (!session->IsCASESession())
                {
                    return;
                }

                if (session->GetLastPeerActivityTime() > lastPeerActivity)
                {
                    return;
                }

                session->MarkAsDefunct();
            });
    }

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
    VerifyOrReturnError(subscribeResponse.GetSubscriptionId(&subscriptionId) == CHIP_NO_ERROR, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsMatchingSubscriptionId(subscriptionId), CHIP_ERROR_INVALID_SUBSCRIPTION);
    ReturnErrorOnFailure(subscribeResponse.GetMaxInterval(&mMaxInterval));

    ChipLogProgress(DataManagement,
                    "Subscription established with SubscriptionID = 0x%08" PRIx32 " MinInterval = %u"
                    "s MaxInterval = %us Peer = %02x:" ChipLogFormatX64,
                    mSubscriptionId, mMinIntervalFloorSeconds, mMaxInterval, GetFabricIndex(), ChipLogValueX64(GetPeerNodeId()));

    ReturnErrorOnFailure(subscribeResponse.ExitContainer());

    MoveToState(ClientState::SubscriptionActive);

    mpCallback.OnSubscriptionEstablished(subscriptionId);

    mNumRetries = 0;

    ReturnErrorOnFailure(RefreshLivenessCheckTimer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::SendAutoResubscribeRequest(ReadPrepareParams && aReadPrepareParams)
{
    mReadPrepareParams = std::move(aReadPrepareParams);
    CHIP_ERROR err     = SendSubscribeRequest(mReadPrepareParams);
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

    if (&aReadPrepareParams != &mReadPrepareParams)
    {
        mReadPrepareParams.mSessionHolder = aReadPrepareParams.mSessionHolder;
    }

    mMinIntervalFloorSeconds = aReadPrepareParams.mMinIntervalFloorSeconds;

    // Todo: Remove the below, Update span in ReadPrepareParams
    Span<AttributePathParams> attributePaths(aReadPrepareParams.mpAttributePathParamsList,
                                             aReadPrepareParams.mAttributePathParamsListSize);
    Span<EventPathParams> eventPaths(aReadPrepareParams.mpEventPathParamsList, aReadPrepareParams.mEventPathParamsListSize);
    Span<DataVersionFilter> dataVersionFilters(aReadPrepareParams.mpDataVersionFilterList,
                                               aReadPrepareParams.mDataVersionFilterListSize);

    VerifyOrReturnError(aReadPrepareParams.mAttributePathParamsListSize != 0 || aReadPrepareParams.mEventPathParamsListSize != 0,
                        CHIP_ERROR_INVALID_ARGUMENT);

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

    auto exchange = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHolder.Get().Value(), this);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    mExchange.Grab(exchange);

    if (aReadPrepareParams.mTimeout == System::Clock::kZero)
    {
        mExchange->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }
    else
    {
        mExchange->SetResponseTimeout(aReadPrepareParams.mTimeout);
    }

    ReturnErrorOnFailure(mExchange->SendMessage(Protocols::InteractionModel::MsgType::SubscribeRequest, std::move(msgBuf),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse)));

    mPeer = aReadPrepareParams.mSessionHolder->AsSecureSession()->GetPeer();
    MoveToState(ClientState::AwaitingInitialReport);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::DefaultResubscribePolicy(CHIP_ERROR aTerminationCause)
{
    VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);

    auto timeTillNextResubscription = ComputeTimeTillNextSubscription();
    ChipLogProgress(DataManagement,
                    "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
                    "ms due to error %" CHIP_ERROR_FORMAT,
                    GetFabricIndex(), ChipLogValueX64(GetPeerNodeId()), mNumRetries, timeTillNextResubscription,
                    aTerminationCause.Format());
    ReturnErrorOnFailure(ScheduleResubscription(timeTillNextResubscription, NullOptional, aTerminationCause == CHIP_ERROR_TIMEOUT));
    return CHIP_NO_ERROR;
}

void ReadClient::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    ReadClient * const _this = static_cast<ReadClient *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogProgress(DataManagement, "HandleDeviceConnected");
    _this->mReadPrepareParams.mSessionHolder.Grab(sessionHandle);

    auto err = _this->SendSubscribeRequest(_this->mReadPrepareParams);
    if (err != CHIP_NO_ERROR)
    {
        _this->Close(err);
    }
}

void ReadClient::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ReadClient * const _this = static_cast<ReadClient *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(DataManagement, "Failed to establish CASE for re-subscription with error '%" CHIP_ERROR_FORMAT "'", err.Format());

    _this->Close(err);
}

void ReadClient::OnResubscribeTimerCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadClient * const _this = static_cast<ReadClient *>(apAppState);
    VerifyOrDie(_this != nullptr);

    CHIP_ERROR err;

    ChipLogProgress(DataManagement, "OnResubscribeTimerCallback: DoCASE = %d", _this->mDoCaseOnNextResub);
    _this->mNumRetries++;

    if (_this->mDoCaseOnNextResub)
    {
        auto * caseSessionManager = InteractionModelEngine::GetInstance()->GetCASESessionManager();
        VerifyOrExit(caseSessionManager != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        //
        // We need to mark our session as defunct explicitly since the assessment of a liveness failure
        // is usually triggered by the absence of any exchange activity that would have otherwise
        // automatically marked the session as defunct on a response timeout.
        //
        // Doing so will ensure that the subsequent call to FindOrEstablishSession will not bind to
        // an existing established session but rather trigger establishing a new one.
        //
        if (_this->mReadPrepareParams.mSessionHolder)
        {
            _this->mReadPrepareParams.mSessionHolder->AsSecureSession()->MarkAsDefunct();
        }

        caseSessionManager->FindOrEstablishSession(_this->mPeer, &_this->mOnConnectedCallback,
                                                   &_this->mOnConnectionFailureCallback);
        return;
    }

    err = _this->SendSubscribeRequest(_this->mReadPrepareParams);

exit:
    if (err != CHIP_NO_ERROR)
    {
        //
        // Call Close (which should trigger re-subscription again) EXCEPT if we got here because we didn't have a valid
        // CASESessionManager pointer when mDoCaseOnNextResub was true.
        //
        // In that case, don't permit re-subscription to occur.
        //
        _this->Close(err, err != CHIP_ERROR_INCORRECT_STATE);
    }
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
