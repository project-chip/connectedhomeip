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
#include <lib/core/TLVTypes.h>
#include <lib/support/FibonacciUtils.h>
#include <messaging/ReliableMessageMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/LockTracker.h>
#include <tracing/metric_event.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

namespace chip {
namespace app {

using Status = Protocols::InteractionModel::Status;

ReadClient::ReadClient(InteractionModelEngine * apImEngine, Messaging::ExchangeManager * apExchangeMgr, Callback & apCallback,
                       InteractionType aInteractionType) :
    mExchange(*this),
    mpCallback(apCallback), mOnConnectedCallback(HandleDeviceConnected, this),
    mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{
    assertChipStackLockedByCurrentThread();

    mpExchangeMgr    = apExchangeMgr;
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
    mIsResubscriptionScheduled    = false;

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
    assertChipStackLockedByCurrentThread();

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

    if (mMinimalResubscribeDelay.count() > waitTimeInMsec)
    {
        waitTimeInMsec = mMinimalResubscribeDelay.count();
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

    mForceCaseOnNextResub = aReestablishCASE;
    if (mForceCaseOnNextResub && mReadPrepareParams.mSessionHolder)
    {
        // Mark our existing session defunct, so that we will try to
        // re-establish it when the timer fires (unless something re-establishes
        // before then).
        mReadPrepareParams.mSessionHolder->AsSecureSession()->MarkAsDefunct();
    }

    ReturnErrorOnFailure(
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            System::Clock::Milliseconds32(aTimeTillNextResubscriptionMs), OnResubscribeTimerCallback, this));
    mIsResubscriptionScheduled = true;

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
        if (IsAwaitingInitialReport() || IsAwaitingSubscribeResponse())
        {
            MATTER_LOG_METRIC_END(Tracing::kMetricDeviceSubscriptionSetup, aError);
        }

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
                CHIP_ERROR originalReason = aError;

                aError = mpCallback.OnResubscriptionNeeded(this, aError);
                if (aError == CHIP_NO_ERROR)
                {
                    return;
                }
                if (aError == CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT)
                {
                    VerifyOrDie(originalReason == CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT);
                    ChipLogProgress(DataManagement, "ICD device is inactive mark subscription as InactiveICDSubscription");
                    MoveToState(ClientState::InactiveICDSubscription);
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
    case ClientState::InactiveICDSubscription:
        return "InactiveICDSubscription";
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
        ReturnErrorOnFailure(dataVersionFilterListBuilder.EndOfDataVersionFilterIBs());
    }
    else
    {
        request.Rollback(backup);
    }

    ReturnErrorOnFailure(request.EndOfReadRequestMessage());
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
        VerifyOrReturnError(event.IsValidEventPath(), CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
        EventPathIB::Builder & path = aEventPathsBuilder.CreatePath();
        ReturnErrorOnFailure(aEventPathsBuilder.GetError());
        ReturnErrorOnFailure(path.Encode(event));
    }

    return aEventPathsBuilder.EndOfEventPaths();
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

    return aAttributePathIBsBuilder.EndOfAttributePathIBs();
}

CHIP_ERROR ReadClient::BuildDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                  const Span<AttributePathParams> & aAttributePaths,
                                                  const Span<DataVersionFilter> & aDataVersionFilters,
                                                  bool & aEncodedDataVersionList)
{
#if CHIP_PROGRESS_LOGGING
    size_t encodedFilterCount    = 0;
    size_t irrelevantFilterCount = 0;
    size_t skippedFilterCount    = 0;
#endif
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
#if CHIP_PROGRESS_LOGGING
            ++irrelevantFilterCount;
#endif
            continue;
        }

        TLV::TLVWriter backup;
        aDataVersionFilterIBsBuilder.Checkpoint(backup);
        CHIP_ERROR err = aDataVersionFilterIBsBuilder.EncodeDataVersionFilterIB(filter);
        if (err == CHIP_NO_ERROR)
        {
#if CHIP_PROGRESS_LOGGING
            ++encodedFilterCount;
#endif
            aEncodedDataVersionList = true;
        }
        else if (err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // Packet is full, ignore the rest of the list
            aDataVersionFilterIBsBuilder.Rollback(backup);
#if CHIP_PROGRESS_LOGGING
            ssize_t nonSkippedFilterCount = &filter - aDataVersionFilters.data();
            skippedFilterCount            = aDataVersionFilters.size() - static_cast<size_t>(nonSkippedFilterCount);
#endif // CHIP_PROGRESS_LOGGING
            break;
        }
        else
        {
            return err;
        }
    }

    ChipLogProgress(DataManagement,
                    "%lu data version filters provided, %lu not relevant, %lu encoded, %lu skipped due to lack of space",
                    static_cast<unsigned long>(aDataVersionFilters.size()), static_cast<unsigned long>(irrelevantFilterCount),
                    static_cast<unsigned long>(encodedFilterCount), static_cast<unsigned long>(skippedFilterCount));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::GenerateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                     const Span<AttributePathParams> & aAttributePaths,
                                                     const Span<DataVersionFilter> & aDataVersionFilters,
                                                     bool & aEncodedDataVersionList)
{
    // Give the callback a chance first, otherwise use the list we have, if any.
    ReturnErrorOnFailure(
        mpCallback.OnUpdateDataVersionFilterList(aDataVersionFilterIBsBuilder, aAttributePaths, aEncodedDataVersionList));

    if (!aEncodedDataVersionList)
    {
        ReturnErrorOnFailure(BuildDataVersionFilterList(aDataVersionFilterIBsBuilder, aAttributePaths, aDataVersionFilters,
                                                        aEncodedDataVersionList));
    }

    return CHIP_NO_ERROR;
}

void ReadClient::OnActiveModeNotification()
{
    VerifyOrDie(mpImEngine->InActiveReadClientList(this));
    // When we reach here, the subscription definitely exceeded the liveness timeout. Just continue the unfinished resubscription
    // logic in `OnLivenessTimeoutCallback`.
    if (IsInactiveICDSubscription())
    {
        TriggerResubscriptionForLivenessTimeout(CHIP_ERROR_TIMEOUT);
        return;
    }

    TriggerResubscribeIfScheduled("check-in message");
}

void ReadClient::OnPeerTypeChange(PeerType aType)
{
    VerifyOrDie(mpImEngine->InActiveReadClientList(this));

    mIsPeerLIT = (aType == PeerType::kLITICD);

    ChipLogProgress(DataManagement, "Peer is now %s LIT ICD.", mIsPeerLIT ? "a" : "not a");

    // If the peer is no longer LIT, try to wake up the subscription and do resubscribe when necessary.
    if (!mIsPeerLIT)
    {
        OnActiveModeNotification();
    }
}

CHIP_ERROR ReadClient::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                         System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status  = Status::InvalidAction;
    VerifyOrExit(!IsIdle() && !IsInactiveICDSubscription(), err = CHIP_ERROR_INCORRECT_STATE);

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        err = ProcessReportData(std::move(aPayload), ReportType::kContinuingTransaction);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeResponse))
    {
        ChipLogProgress(DataManagement, "SubscribeResponse is received");
        VerifyOrExit(apExchangeContext == mExchange.Get(), err = CHIP_ERROR_INCORRECT_STATE);
        err = ProcessSubscribeResponse(std::move(aPayload));
        MATTER_LOG_METRIC_END(Tracing::kMetricDeviceSubscriptionSetup, err);
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

    CHIP_ERROR err = ProcessReportData(std::move(aPayload), ReportType::kUnsolicited);
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

CHIP_ERROR ReadClient::ProcessReportData(System::PacketBufferHandle && aPayload, ReportType aReportType)
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

#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (aReportType != ReportType::kUnsolicited)
    {
        report.PrettyPrint();
    }
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
            mpCallback.NotifySubscriptionStillActive(*this);
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

CHIP_ERROR ReadClient::ReadICDOperatingModeFromAttributeDataIB(TLV::TLVReader && aReader, PeerType & aType)
{
    Clusters::IcdManagement::Attributes::OperatingMode::TypeInfo::DecodableType operatingMode;

    CHIP_ERROR err = DataModel::Decode(aReader, operatingMode);
    ReturnErrorOnFailure(err);

    switch (operatingMode)
    {
    case Clusters::IcdManagement::OperatingModeEnum::kSit:
        aType = PeerType::kNormal;
        break;
    case Clusters::IcdManagement::OperatingModeEnum::kLit:
        aType = PeerType::kLITICD;
        break;
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    return err;
}

CHIP_ERROR ReadClient::ProcessAttributePath(AttributePathIB::Parser & aAttributePathParser,
                                            ConcreteDataAttributePath & aAttributePath)
{
    // The ReportData must contain a concrete attribute path.  Don't validate ID
    // ranges here, so we can tell apart "malformed data" and "out of range
    // IDs".
    CHIP_ERROR err = CHIP_NO_ERROR;
    // The ReportData must contain a concrete attribute path
    err = aAttributePathParser.GetConcreteAttributePath(aAttributePath, AttributePathIB::ValidateIdRanges::kNo);
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
            if (!attributePath.IsValid())
            {
                // Don't fail the entire read or subscription when there is an
                // out-of-range ID.  Just skip that one AttributeReportIB.
                ChipLogError(DataManagement,
                             "Skipping AttributeStatusIB with out-of-range IDs: (%d, " ChipLogFormatMEI ", " ChipLogFormatMEI ") ",
                             attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId),
                             ChipLogValueMEI(attributePath.mAttributeId));
                continue;
            }

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
            if (!attributePath.IsValid())
            {
                // Don't fail the entire read or subscription when there is an
                // out-of-range ID.  Just skip that one AttributeReportIB.
                ChipLogError(DataManagement,
                             "Skipping AttributeDataIB  with out-of-range IDs: (%d, " ChipLogFormatMEI ", " ChipLogFormatMEI ") ",
                             attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId),
                             ChipLogValueMEI(attributePath.mAttributeId));
                continue;
            }

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

            if (attributePath.MatchesConcreteAttributePath(ConcreteAttributePath(
                    kRootEndpointId, Clusters::IcdManagement::Id, Clusters::IcdManagement::Attributes::OperatingMode::Id)))
            {
                PeerType peerType;
                TLV::TLVReader operatingModeTlvReader;
                operatingModeTlvReader.Init(dataReader);
                if (CHIP_NO_ERROR == ReadICDOperatingModeFromAttributeDataIB(std::move(operatingModeTlvReader), peerType))
                {
                    // It is safe to call `OnPeerTypeChange` since we are in the middle of parsing the attribute data, And
                    // the subscription should be active so `OnActiveModeNotification` is a no-op in this case.
                    InteractionModelEngine::GetInstance()->OnPeerTypeChange(mPeer, peerType);
                }
                else
                {
                    ChipLogError(DataManagement, "Failed to get ICD state from attribute data with error'%" CHIP_ERROR_FORMAT "'",
                                 err.Format());
                }
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

    VerifyOrReturnError(IsSubscriptionActive(), CHIP_ERROR_INCORRECT_STATE);

    CancelLivenessCheckTimer();

    System::Clock::Timeout timeout;
    ReturnErrorOnFailure(ComputeLivenessCheckTimerTimeout(&timeout));

    // EFR32/MBED/INFINION/K32W's chrono count return long unsigned, but other platform returns unsigned
    ChipLogProgress(
        DataManagement,
        "Refresh LivenessCheckTime for %lu milliseconds with SubscriptionId = 0x%08" PRIx32 " Peer = %02x:" ChipLogFormatX64,
        static_cast<long unsigned>(timeout.count()), mSubscriptionId, GetFabricIndex(), ChipLogValueX64(GetPeerNodeId()));
    err = InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        timeout, OnLivenessTimeoutCallback, this);

    return err;
}

CHIP_ERROR ReadClient::ComputeLivenessCheckTimerTimeout(System::Clock::Timeout * aTimeout)
{
    if (mLivenessTimeoutOverride != System::Clock::kZero)
    {
        *aTimeout = mLivenessTimeoutOverride;
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mReadPrepareParams.mSessionHolder, CHIP_ERROR_INCORRECT_STATE);

    //
    // To calculate the duration we're willing to wait for a report to come to us, we take into account the maximum interval of
    // the subscription AND the time it takes for the report to make it to us in the worst case.
    //
    // We have no way to estimate what the network latency will be, but we do know the other side will time out its ReportData
    // after its computed round-trip timeout plus the processing time it gives us (app::kExpectedIMProcessingTime).  Once it
    // times out, assuming it sent the report at all, there's no point in us thinking we still have a subscription.
    //
    // We can't use ComputeRoundTripTimeout() on the session for two reasons: we want the roundtrip timeout from the point of
    // view of the peer, not us, and we want to start off with the assumption the peer will likely have, which is that we are
    // idle, whereas ComputeRoundTripTimeout() uses the current activity state of the peer.
    //
    // So recompute the round-trip timeout directly.  Assume MRP, since in practice that is likely what is happening.
    auto & peerMRPConfig = mReadPrepareParams.mSessionHolder->GetRemoteMRPConfig();
    // Peer will assume we are idle (hence we pass kZero to GetMessageReceiptTimeout()), but will assume we treat it as active
    // for the response, so to match the retransmission timeout computation for the message back to the peeer, we should treat
    // it as active.
    auto roundTripTimeout = mReadPrepareParams.mSessionHolder->GetMessageReceiptTimeout(System::Clock::kZero) +
        kExpectedIMProcessingTime +
        GetRetransmissionTimeout(peerMRPConfig.mActiveRetransTimeout, peerMRPConfig.mIdleRetransTimeout,
                                 System::SystemClock().GetMonotonicTimestamp(), peerMRPConfig.mActiveThresholdTime);
    *aTimeout = System::Clock::Seconds16(mMaxInterval) + roundTripTimeout;
    return CHIP_NO_ERROR;
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
    mIsResubscriptionScheduled = false;
}

void ReadClient::OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadClient * const _this = reinterpret_cast<ReadClient *>(apAppState);

    // TODO: add a more specific error here for liveness timeout failure to distinguish between other classes of timeouts (i.e
    // response timeouts).
    CHIP_ERROR subscriptionTerminationCause = CHIP_ERROR_TIMEOUT;

    //
    // Might as well try to see if this instance exists in the tracked list in the IM.
    // This might blow-up if either the client has since been free'ed (use-after-free), or if the engine has since
    // been shutdown at which point the client wouldn't exist in the active read client list.
    //
    VerifyOrDie(_this->mpImEngine->InActiveReadClientList(_this));

    ChipLogError(DataManagement,
                 "Subscription Liveness timeout with SubscriptionID = 0x%08" PRIx32 ", Peer = %02x:" ChipLogFormatX64,
                 _this->mSubscriptionId, _this->GetFabricIndex(), ChipLogValueX64(_this->GetPeerNodeId()));

    if (_this->mIsPeerLIT)
    {
        subscriptionTerminationCause = CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT;
    }

    _this->TriggerResubscriptionForLivenessTimeout(subscriptionTerminationCause);
}

void ReadClient::TriggerResubscriptionForLivenessTimeout(CHIP_ERROR aReason)
{
    // We didn't get a message from the server on time; it's possible that it no
    // longer has a useful CASE session to us.  Mark defunct all sessions that
    // have not seen peer activity in at least as long as our session.
    const auto & holder = mReadPrepareParams.mSessionHolder;
    if (holder)
    {
        System::Clock::Timestamp lastPeerActivity = holder->AsSecureSession()->GetLastPeerActivityTime();
        mpImEngine->GetExchangeManager()->GetSessionManager()->ForEachMatchingSession(mPeer, [&lastPeerActivity](auto * session) {
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

    Close(aReason);
}

CHIP_ERROR ReadClient::ProcessSubscribeResponse(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));

    SubscribeResponseMessage::Parser subscribeResponse;
    ReturnErrorOnFailure(subscribeResponse.Init(reader));

#if CHIP_CONFIG_IM_PRETTY_PRINT
    subscribeResponse.PrettyPrint();
#endif

    SubscriptionId subscriptionId = 0;
    VerifyOrReturnError(subscribeResponse.GetSubscriptionId(&subscriptionId) == CHIP_NO_ERROR, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsMatchingSubscriptionId(subscriptionId), CHIP_ERROR_INVALID_SUBSCRIPTION);
    ReturnErrorOnFailure(subscribeResponse.GetMaxInterval(&mMaxInterval));

#if CHIP_PROGRESS_LOGGING
    auto duration = System::Clock::Milliseconds32(System::SystemClock().GetMonotonicTimestamp() - mSubscribeRequestTime);
#endif
    ChipLogProgress(DataManagement,
                    "Subscription established in %" PRIu32 "ms with SubscriptionID = 0x%08" PRIx32 " MinInterval = %u"
                    "s MaxInterval = %us Peer = %02x:" ChipLogFormatX64,
                    duration.count(), mSubscriptionId, mMinIntervalFloorSeconds, mMaxInterval, GetFabricIndex(),
                    ChipLogValueX64(GetPeerNodeId()));

    ReturnErrorOnFailure(subscribeResponse.ExitContainer());

    MoveToState(ClientState::SubscriptionActive);

    mpCallback.OnSubscriptionEstablished(subscriptionId);

    mNumRetries = 0;

    ReturnErrorOnFailure(RefreshLivenessCheckTimer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadClient::SendAutoResubscribeRequest(ReadPrepareParams && aReadPrepareParams)
{
    // Make sure we don't use minimal resubscribe delays from previous attempts
    // for this one.
    mMinimalResubscribeDelay = System::Clock::kZero;

    mReadPrepareParams = std::move(aReadPrepareParams);
    CHIP_ERROR err     = SendSubscribeRequest(mReadPrepareParams);
    if (err != CHIP_NO_ERROR)
    {
        StopResubscription();
    }
    return err;
}

CHIP_ERROR ReadClient::SendAutoResubscribeRequest(const ScopedNodeId & aPublisherId, ReadPrepareParams && aReadPrepareParams)
{
    mPeer              = aPublisherId;
    mReadPrepareParams = std::move(aReadPrepareParams);
    CHIP_ERROR err     = EstablishSessionToPeer();
    if (err != CHIP_NO_ERROR)
    {
        // Make sure we call our callback's OnDeallocatePaths.
        StopResubscription();
    }
    return err;
}

CHIP_ERROR ReadClient::SendSubscribeRequest(const ReadPrepareParams & aReadPrepareParams)
{
    VerifyOrReturnError(aReadPrepareParams.mMinIntervalFloorSeconds <= aReadPrepareParams.mMaxIntervalCeilingSeconds,
                        CHIP_ERROR_INVALID_ARGUMENT);

    auto err = SendSubscribeRequestImpl(aReadPrepareParams);
    if (CHIP_NO_ERROR != err)
    {
        MATTER_LOG_METRIC_END(Tracing::kMetricDeviceSubscriptionSetup, err);
    }
    return err;
}

CHIP_ERROR ReadClient::SendSubscribeRequestImpl(const ReadPrepareParams & aReadPrepareParams)
{
    MATTER_LOG_METRIC_BEGIN(Tracing::kMetricDeviceSubscriptionSetup);

#if CHIP_PROGRESS_LOGGING
    mSubscribeRequestTime = System::SystemClock().GetMonotonicTimestamp();
#endif

    VerifyOrReturnError(ClientState::Idle == mState, CHIP_ERROR_INCORRECT_STATE);

    if (&aReadPrepareParams != &mReadPrepareParams)
    {
        mReadPrepareParams.mSessionHolder = aReadPrepareParams.mSessionHolder;
    }

    mIsPeerLIT = aReadPrepareParams.mIsPeerLIT;

    mMinIntervalFloorSeconds = aReadPrepareParams.mMinIntervalFloorSeconds;

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
        ReturnErrorOnFailure(dataVersionFilterListBuilder.EndOfDataVersionFilterIBs());
    }
    else
    {
        request.Rollback(backup);
    }

    ReturnErrorOnFailure(request.EndOfSubscribeRequestMessage());
    ReturnErrorOnFailure(writer.Finalize(&msgBuf));

    VerifyOrReturnError(aReadPrepareParams.mSessionHolder, CHIP_ERROR_MISSING_SECURE_SESSION);

    auto exchange = mpExchangeMgr->NewContext(aReadPrepareParams.mSessionHolder.Get().Value(), this);
    if (exchange == nullptr)
    {
        if (aReadPrepareParams.mSessionHolder->AsSecureSession()->IsActiveSession())
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        // Trying to subscribe with a defunct session somehow.
        return CHIP_ERROR_INCORRECT_STATE;
    }

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
    if (aTerminationCause == CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT)
    {
        ChipLogProgress(DataManagement, "ICD device is inactive, skipping scheduling resubscribe within DefaultResubscribePolicy");
        return CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT;
    }

    VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);

    auto timeTillNextResubscription = ComputeTimeTillNextSubscription();
    ChipLogProgress(DataManagement,
                    "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
                    "ms due to error %" CHIP_ERROR_FORMAT,
                    GetFabricIndex(), ChipLogValueX64(GetPeerNodeId()), mNumRetries, timeTillNextResubscription,
                    aTerminationCause.Format());
    return ScheduleResubscription(timeTillNextResubscription, NullOptional, aTerminationCause == CHIP_ERROR_TIMEOUT);
}

void ReadClient::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                       const SessionHandle & sessionHandle)
{
    ReadClient * const _this = static_cast<ReadClient *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogProgress(DataManagement, "HandleDeviceConnected");
    _this->mReadPrepareParams.mSessionHolder.Grab(sessionHandle);
    _this->mpExchangeMgr = &exchangeMgr;

    _this->mpCallback.OnCASESessionEstablished(sessionHandle, _this->mReadPrepareParams);

    auto err = _this->SendSubscribeRequest(_this->mReadPrepareParams);
    if (err != CHIP_NO_ERROR)
    {
        _this->Close(err);
    }
}

void ReadClient::HandleDeviceConnectionFailure(void * context, const OperationalSessionSetup::ConnectionFailureInfo & failureInfo)
{
    ReadClient * const _this = static_cast<ReadClient *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(DataManagement, "Failed to establish CASE for re-subscription with error '%" CHIP_ERROR_FORMAT "'",
                 failureInfo.error.Format());

#if CHIP_CONFIG_ENABLE_BUSY_HANDLING_FOR_OPERATIONAL_SESSION_SETUP
#if CHIP_DETAIL_LOGGING
    if (failureInfo.requestedBusyDelay.HasValue())
    {
        ChipLogDetail(DataManagement, "Will delay resubscription by %u ms due to BUSY response",
                      failureInfo.requestedBusyDelay.Value().count());
    }
#endif // CHIP_DETAIL_LOGGING
    _this->mMinimalResubscribeDelay = failureInfo.requestedBusyDelay.ValueOr(System::Clock::kZero);
#else
    _this->mMinimalResubscribeDelay = System::Clock::kZero;
#endif // CHIP_CONFIG_ENABLE_BUSY_HANDLING_FOR_OPERATIONAL_SESSION_SETUP

    _this->Close(failureInfo.error);
}

void ReadClient::OnResubscribeTimerCallback(System::Layer * /* If this starts being used, fix callers that pass nullptr */,
                                            void * apAppState)
{
    ReadClient * const _this = static_cast<ReadClient *>(apAppState);
    VerifyOrDie(_this != nullptr);

    _this->mIsResubscriptionScheduled = false;

    CHIP_ERROR err;

    ChipLogProgress(DataManagement, "OnResubscribeTimerCallback: ForceCASE = %d", _this->mForceCaseOnNextResub);
    _this->mNumRetries++;

    bool allowResubscribeOnError = true;
    if (!_this->mReadPrepareParams.mSessionHolder ||
        !_this->mReadPrepareParams.mSessionHolder->AsSecureSession()->IsActiveSession())
    {
        // We don't have an active CASE session.  We need to go ahead and set
        // one up, if we can.
        if (_this->EstablishSessionToPeer() == CHIP_NO_ERROR)
        {
            return;
        }

        if (_this->mForceCaseOnNextResub)
        {
            // Caller asked us to force CASE but we have no way to do CASE.
            // Just stop trying.
            allowResubscribeOnError = false;
        }

        // No way to send our subscribe request.
        err = CHIP_ERROR_INCORRECT_STATE;
        ExitNow();
    }

    err = _this->SendSubscribeRequest(_this->mReadPrepareParams);

exit:
    if (err != CHIP_NO_ERROR)
    {
        //
        // Call Close (which should trigger re-subscription again) EXCEPT if we got here because we didn't have a valid
        // CASESessionManager pointer when mForceCaseOnNextResub was true.
        //
        // In that case, don't permit re-subscription to occur.
        //
        _this->Close(err, allowResubscribeOnError);
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
        ReturnErrorOnFailure(mpCallback.GetHighestReceivedEventNumber(aEventMin));
        if (aEventMin.HasValue())
        {
            // We want to start with the first event _after_ the last one we received.
            aEventMin.SetValue(aEventMin.Value() + 1);
        }
    }
    return CHIP_NO_ERROR;
}

void ReadClient::TriggerResubscribeIfScheduled(const char * reason)
{
    if (!mIsResubscriptionScheduled)
    {
        return;
    }

    ChipLogDetail(DataManagement, "ReadClient[%p] triggering resubscribe, reason: %s", this, reason);
    CancelResubscribeTimer();
    OnResubscribeTimerCallback(nullptr, this);
}

Optional<System::Clock::Timeout> ReadClient::GetSubscriptionTimeout()
{
    if (!IsSubscriptionType() || !IsSubscriptionActive())
    {
        return NullOptional;
    }

    System::Clock::Timeout timeout;
    CHIP_ERROR err = ComputeLivenessCheckTimerTimeout(&timeout);
    if (err != CHIP_NO_ERROR)
    {
        return NullOptional;
    }

    return MakeOptional(timeout);
}

CHIP_ERROR ReadClient::EstablishSessionToPeer()
{
    ChipLogProgress(DataManagement, "Trying to establish a CASE session for subscription");
    auto * caseSessionManager = InteractionModelEngine::GetInstance()->GetCASESessionManager();
    VerifyOrReturnError(caseSessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    caseSessionManager->FindOrEstablishSession(mPeer, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
