/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines read handler for a CHIP Interaction Data model
 *
 */

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPathIB.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>

#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

ReadHandler::ReadHandler(ManagementCallback & apCallback, Messaging::ExchangeContext * apExchangeContext,
                         InteractionType aInteractionType) :
    mExchangeCtx(*this),
    mManagementCallback(apCallback)
{
    VerifyOrDie(apExchangeContext != nullptr);

    mExchangeCtx.Grab(apExchangeContext);

    mInteractionType            = aInteractionType;
    mLastWrittenEventsBytes     = 0;
    mTransactionStartGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    mFlags.ClearAll();
    SetStateFlag(ReadHandlerFlags::PrimingReports);

    mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());
}

ReadHandler::~ReadHandler()
{
    auto * appCallback = mManagementCallback.GetAppCallback();
    if (mFlags.Has(ReadHandlerFlags::ActiveSubscription) && appCallback)
    {
        appCallback->OnSubscriptionTerminated(*this);
    }

    if (IsType(InteractionType::Subscribe))
    {
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            OnUnblockHoldReportCallback, this);

        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            OnRefreshSubscribeTimerSyncCallback, this);
    }

    if (IsAwaitingReportResponse())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(mpAttributePathList);
    InteractionModelEngine::GetInstance()->ReleaseEventPathList(mpEventPathList);
    InteractionModelEngine::GetInstance()->ReleaseDataVersionFilterList(mpDataVersionFilterList);
}

void ReadHandler::Close()
{
    MoveToState(HandlerState::AwaitingDestruction);
    mManagementCallback.OnDone(*this);
}

void ReadHandler::OnInitialRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    if (IsType(InteractionType::Subscribe))
    {
        err = ProcessSubscribeRequest(std::move(aPayload));
    }
    else
    {
        err = ProcessReadRequest(std::move(aPayload));
    }

    if (err != CHIP_NO_ERROR)
    {
        Status status = Status::InvalidAction;
        if (err.IsIMStatus())
        {
            status = StatusIB(err).mStatus;
        }
        StatusResponse::Send(status, mExchangeCtx.Get(), /* aExpectResponse = */ false);
        Close();
    }
    else
    {
        // Force us to be in a dirty state so we get processed by the reporting
        SetStateFlag(ReadHandlerFlags::ForceDirty);
    }
}

CHIP_ERROR ReadHandler::OnStatusResponse(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                         bool & aSendStatusResponse)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    aSendStatusResponse    = true;
    CHIP_ERROR statusError = CHIP_NO_ERROR;
    SuccessOrExit(err = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError));
    // Since this is a valid Status Response message, we don't have to send a Status Response in reply to it.
    aSendStatusResponse = false;
    SuccessOrExit(err = statusError);
    switch (mState)
    {
    case HandlerState::AwaitingReportResponse:
        if (IsChunkedReport())
        {
            mExchangeCtx->WillSendMessage();
        }
        else if (IsType(InteractionType::Subscribe))
        {
            if (IsPriming())
            {
                err = SendSubscribeResponse();

                SetStateFlag(ReadHandlerFlags::ActiveSubscription);

                auto * appCallback = mManagementCallback.GetAppCallback();
                if (appCallback)
                {
                    appCallback->OnSubscriptionEstablished(*this);
                }
            }
        }
        else
        {
            //
            // We're done processing a read, so let's close out and return.
            //
            Close();
            return CHIP_NO_ERROR;
        }

        MoveToState(HandlerState::GeneratingReports);
        break;

    case HandlerState::GeneratingReports:
    case HandlerState::Idle:
    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    }

exit:
    return err;
}

CHIP_ERROR ReadHandler::SendStatusReport(Protocols::InteractionModel::Status aStatus)
{
    VerifyOrReturnLogError(IsReportable(), CHIP_ERROR_INCORRECT_STATE);
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
        auto exchange = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
        VerifyOrReturnLogError(exchange != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mExchangeCtx.Grab(exchange);
    }

    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
    return StatusResponse::Send(aStatus, mExchangeCtx.Get(), /* aExpectResponse = */ false);
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks)
{
    VerifyOrReturnLogError(IsReportable(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrDie(!IsAwaitingReportResponse()); // Should not be reportable!
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
        auto exchange = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
        VerifyOrReturnLogError(exchange != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mExchangeCtx.Grab(exchange);
    }

    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);

    if (!IsReporting())
    {
        mCurrentReportsBeginGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    }
    SetStateFlag(ReadHandlerFlags::ChunkedReport, aMoreChunks);
    bool responseExpected = IsType(InteractionType::Subscribe) || aMoreChunks;

    mExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    CHIP_ERROR err = mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                               responseExpected ? Messaging::SendMessageFlags::kExpectResponse
                                                                : Messaging::SendMessageFlags::kNone);
    if (err == CHIP_NO_ERROR)
    {
        if (responseExpected)
        {
            MoveToState(HandlerState::AwaitingReportResponse);
        }
        else
        {
            // Make sure we're not treated as an in-flight report waiting for a
            // response by the reporting engine.
            InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
        }

        if (IsType(InteractionType::Subscribe) && !IsPriming())
        {
            // Ignore the error from RefreshSubscribeSyncTimer.  If we've
            // successfully sent the message, we need to return success from
            // this method.
            RefreshSubscribeSyncTimer();
        }
    }
    if (!aMoreChunks)
    {
        mPreviousReportsBeginGeneration = mCurrentReportsBeginGeneration;
        ClearForceDirtyFlag();
        InteractionModelEngine::GetInstance()->ReleaseDataVersionFilterList(mpDataVersionFilterList);
    }

    return err;
}

CHIP_ERROR ReadHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                          System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool sendStatusResponse = true;
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        err = OnStatusResponse(apExchangeContext, std::move(aPayload), sendStatusResponse);
    }
    else
    {
        ChipLogDetail(DataManagement, "ReadHandler:: Msg type %d not supported", aPayloadHeader.GetMessageType());
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    if (sendStatusResponse)
    {
        StatusResponse::Send(Status::InvalidAction, apExchangeContext, false /*aExpectResponse*/);
    }

    if (err != CHIP_NO_ERROR)
    {
        Close();
    }
    return err;
}

bool ReadHandler::IsFromSubscriber(Messaging::ExchangeContext & apExchangeContext) const
{
    return (IsType(InteractionType::Subscribe) &&
            GetInitiatorNodeId() == apExchangeContext.GetSessionHandle()->AsSecureSession()->GetPeerNodeId() &&
            GetAccessingFabricIndex() == apExchangeContext.GetSessionHandle()->GetFabricIndex());
}

void ReadHandler::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogError(DataManagement, "Time out! failed to receive status response from Exchange: " ChipLogFormatExchange,
                 ChipLogValueExchange(apExchangeContext));
    Close();
}

CHIP_ERROR ReadHandler::ProcessReadRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    ReadRequestMessage::Parser readRequestParser;
    EventPathIBs::Parser eventPathListParser;
    EventFilterIBs::Parser eventFilterIBsParser;
    AttributePathIBs::Parser attributePathListParser;

    reader.Init(std::move(aPayload));

    ReturnErrorOnFailure(readRequestParser.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(readRequestParser.CheckSchemaValidity());
#endif

    err = readRequestParser.GetAttributeRequests(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(ProcessAttributePaths(attributePathListParser));
        DataVersionFilterIBs::Parser dataVersionFilterListParser;
        err = readRequestParser.GetDataVersionFilters(&dataVersionFilterListParser);
        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(ProcessDataVersionFilterList(dataVersionFilterListParser));
        }
    }
    ReturnErrorOnFailure(err);
    err = readRequestParser.GetEventRequests(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(err);
        ReturnErrorOnFailure(ProcessEventPaths(eventPathListParser));
        err = readRequestParser.GetEventFilters(&eventFilterIBsParser);
        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(ProcessEventFilters(eventFilterIBsParser));
        }
    }
    ReturnErrorOnFailure(err);

    bool isFabricFiltered;
    ReturnErrorOnFailure(readRequestParser.GetIsFabricFiltered(&isFabricFiltered));
    SetStateFlag(ReadHandlerFlags::FabricFiltered, isFabricFiltered);
    ReturnErrorOnFailure(readRequestParser.ExitContainer());
    MoveToState(HandlerState::GeneratingReports);

    mExchangeCtx->WillSendMessage();

    // There must be no code after the WillSendMessage() call that can cause
    // this method to return a failure.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadHandler::ProcessAttributePaths(AttributePathIBs::Parser & aAttributePathListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aAttributePathListParser.GetReader(&reader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        AttributePathParams attribute;
        AttributePathIB::Parser path;
        ReturnErrorOnFailure(path.Init(reader));
        ReturnErrorOnFailure(path.ParsePath(attribute));
        ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->PushFrontAttributePathList(mpAttributePathList, attribute));
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(mpAttributePathList);
        mAttributePathExpandIterator = AttributePathExpandIterator(mpAttributePathList);
        err                          = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ReadHandler::ProcessDataVersionFilterList(DataVersionFilterIBs::Parser & aDataVersionFilterListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    aDataVersionFilterListParser.GetReader(&reader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        DataVersionFilter versionFilter;
        ClusterPathIB::Parser path;
        DataVersionFilterIB::Parser filter;
        ReturnErrorOnFailure(filter.Init(reader));
        DataVersion version = 0;
        ReturnErrorOnFailure(filter.GetDataVersion(&version));
        versionFilter.mDataVersion.SetValue(version);
        ReturnErrorOnFailure(filter.GetPath(&path));
        ReturnErrorOnFailure(path.GetEndpoint(&(versionFilter.mEndpointId)));
        ReturnErrorOnFailure(path.GetCluster(&(versionFilter.mClusterId)));
        VerifyOrReturnError(versionFilter.IsValidDataVersionFilter(), CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB);
        ReturnErrorOnFailure(
            InteractionModelEngine::GetInstance()->PushFrontDataVersionFilterList(mpDataVersionFilterList, versionFilter));
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ReadHandler::ProcessEventPaths(EventPathIBs::Parser & aEventPathsParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aEventPathsParser.GetReader(&reader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        EventPathParams event;
        EventPathIB::Parser path;
        ReturnErrorOnFailure(path.Init(reader));
        ReturnErrorOnFailure(path.ParsePath(event));
        ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->PushFrontEventPathParamsList(mpEventPathList, event));
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ReadHandler::ProcessEventFilters(EventFilterIBs::Parser & aEventFiltersParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aEventFiltersParser.GetReader(&reader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        EventFilterIB::Parser filter;
        ReturnErrorOnFailure(filter.Init(reader));
        // this is for current node, and would have only one event filter.
        ReturnErrorOnFailure(filter.GetEventMin(&(mEventMin)));
    }
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

const char * ReadHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case HandlerState::Idle:
        return "Idle";
    case HandlerState::AwaitingDestruction:
        return "AwaitingDestruction";
    case HandlerState::GeneratingReports:
        return "GeneratingReports";

    case HandlerState::AwaitingReportResponse:
        return "AwaitingReportResponse";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadHandler::MoveToState(const HandlerState aTargetState)
{
    if (aTargetState == mState)
    {
        return;
    }

    if (IsAwaitingReportResponse() && aTargetState != HandlerState::AwaitingReportResponse)
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }

    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());

    //
    // If we just unblocked sending reports, let's go ahead and schedule the reporting
    // engine to run to kick that off.
    //
    if (aTargetState == HandlerState::GeneratingReports && IsReportable())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
}

bool ReadHandler::CheckEventClean(EventManagement & aEventManager)
{
    if (mFlags.Has(ReadHandlerFlags::ChunkedReport))
    {
        if ((mLastScheduledEventNumber != 0) && (mEventMin <= mLastScheduledEventNumber))
        {
            return false;
        }
    }
    else
    {
        EventNumber lastEventNumber = aEventManager.GetLastEventNumber();
        if ((lastEventNumber != 0) && (mEventMin <= lastEventNumber))
        {
            // We have more events. snapshot last event number
            aEventManager.SetScheduledEventInfo(mLastScheduledEventNumber, mLastWrittenEventsBytes);
            return false;
        }
    }
    return true;
}

CHIP_ERROR ReadHandler::SendSubscribeResponse()
{
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnLogError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(packet));

    SubscribeResponseMessage::Builder response;
    ReturnErrorOnFailure(response.Init(&writer));
    response.SubscriptionId(mSubscriptionId).MaxInterval(mMaxInterval).EndOfSubscribeResponseMessage();
    ReturnErrorOnFailure(response.GetError());

    ReturnErrorOnFailure(writer.Finalize(&packet));
    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(RefreshSubscribeSyncTimer());

    ClearStateFlag(ReadHandlerFlags::PrimingReports);
    return mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeResponse, std::move(packet));
}

CHIP_ERROR ReadHandler::ProcessSubscribeRequest(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));

    SubscribeRequestMessage::Parser subscribeRequestParser;
    ReturnErrorOnFailure(subscribeRequestParser.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(subscribeRequestParser.CheckSchemaValidity());
#endif

    AttributePathIBs::Parser attributePathListParser;
    CHIP_ERROR err = subscribeRequestParser.GetAttributeRequests(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(ProcessAttributePaths(attributePathListParser));
        DataVersionFilterIBs::Parser dataVersionFilterListParser;
        err = subscribeRequestParser.GetDataVersionFilters(&dataVersionFilterListParser);
        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(ProcessDataVersionFilterList(dataVersionFilterListParser));
        }
    }
    ReturnErrorOnFailure(err);

    EventPathIBs::Parser eventPathListParser;
    err = subscribeRequestParser.GetEventRequests(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(ProcessEventPaths(eventPathListParser));
        EventFilterIBs::Parser eventFilterIBsParser;
        err = subscribeRequestParser.GetEventFilters(&eventFilterIBsParser);
        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(ProcessEventFilters(eventFilterIBsParser));
        }
    }
    ReturnErrorOnFailure(err);

    ReturnErrorOnFailure(subscribeRequestParser.GetMinIntervalFloorSeconds(&mMinIntervalFloorSeconds));
    ReturnErrorOnFailure(subscribeRequestParser.GetMaxIntervalCeilingSeconds(&mMaxInterval));
    VerifyOrReturnError(mMinIntervalFloorSeconds <= mMaxInterval, CHIP_ERROR_INVALID_ARGUMENT);

    //
    // Notify the application (if requested) of the impending subscription and check whether we should still proceed to set it up.
    // This also provides the application an opportunity to modify the negotiated min/max intervals set above.
    //
    auto * appCallback = mManagementCallback.GetAppCallback();
    if (appCallback)
    {
        if (appCallback->OnSubscriptionRequested(*this, *mExchangeCtx->GetSessionHandle()->AsSecureSession()) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_TRANSACTION_CANCELED;
        }
    }

    ChipLogProgress(DataManagement, "Final negotiated min/max parameters: Min = %ds, Max = %ds", mMinIntervalFloorSeconds,
                    mMaxInterval);

    bool isFabricFiltered;
    ReturnErrorOnFailure(subscribeRequestParser.GetIsFabricFiltered(&isFabricFiltered));
    SetStateFlag(ReadHandlerFlags::FabricFiltered, isFabricFiltered);
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&mSubscriptionId), sizeof(mSubscriptionId)));
    ReturnErrorOnFailure(subscribeRequestParser.ExitContainer());
    MoveToState(HandlerState::GeneratingReports);

    mExchangeCtx->WillSendMessage();

    return CHIP_NO_ERROR;
}

void ReadHandler::OnUnblockHoldReportCallback(System::Layer * apSystemLayer, void * apAppState)
{
    VerifyOrReturn(apAppState != nullptr);
    ReadHandler * readHandler = static_cast<ReadHandler *>(apAppState);
    ChipLogDetail(DataManagement, "Unblock report hold after min %d seconds", readHandler->mMinIntervalFloorSeconds);
    readHandler->ClearStateFlag(ReadHandlerFlags::HoldReport);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        System::Clock::Seconds16(readHandler->mMaxInterval - readHandler->mMinIntervalFloorSeconds),
        OnRefreshSubscribeTimerSyncCallback, readHandler);
}

void ReadHandler::OnRefreshSubscribeTimerSyncCallback(System::Layer * apSystemLayer, void * apAppState)
{
    VerifyOrReturn(apAppState != nullptr);
    ReadHandler * readHandler = static_cast<ReadHandler *>(apAppState);
    readHandler->ClearStateFlag(ReadHandlerFlags::HoldSync);
    ChipLogProgress(DataManagement, "Refresh subscribe timer sync after %d seconds",
                    readHandler->mMaxInterval - readHandler->mMinIntervalFloorSeconds);
}

CHIP_ERROR ReadHandler::RefreshSubscribeSyncTimer()
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnUnblockHoldReportCallback, this);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnRefreshSubscribeTimerSyncCallback, this);

    if (!IsChunkedReport())
    {
        ChipLogProgress(DataManagement, "Refresh Subscribe Sync Timer with min %d seconds and max %d seconds",
                        mMinIntervalFloorSeconds, mMaxInterval);
        SetStateFlag(ReadHandlerFlags::HoldReport);
        SetStateFlag(ReadHandlerFlags::HoldSync);
        ReturnErrorOnFailure(
            InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
                System::Clock::Seconds16(mMinIntervalFloorSeconds), OnUnblockHoldReportCallback, this));
    }

    return CHIP_NO_ERROR;
}

void ReadHandler::ResetPathIterator()
{
    mAttributePathExpandIterator = AttributePathExpandIterator(mpAttributePathList);
    mAttributeEncoderState       = AttributeValueEncoder::AttributeEncodeState();
}

void ReadHandler::SetDirty(const AttributePathParams & aAttributeChanged)
{
    ConcreteAttributePath path;

    mDirtyGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();

    // We won't reset the path iterator for every SetDirty call to reduce the number of full data reports.
    // The iterator will be reset after finishing each report session.
    //
    // Here we just reset the iterator to the beginning of the current cluster, if the dirty path affects it.
    // This will ensure the reports are consistent within a single cluster generated from a single path in the request.

    // TODO (#16699): Currently we can only gurentee the reports generated from a single path in the request are consistent. The
    // data might be inconsistent if the user send a request with two paths from the same cluster. We need to clearify the behavior
    // or make it consistent.
    if (mAttributePathExpandIterator.Get(path) &&
        (aAttributeChanged.HasWildcardEndpointId() || aAttributeChanged.mEndpointId == path.mEndpointId) &&
        (aAttributeChanged.HasWildcardClusterId() || aAttributeChanged.mClusterId == path.mClusterId))
    {
        ChipLogDetail(DataManagement,
                      "The dirty path intersects the cluster we are currently reporting; reset the iterator to the beginning of "
                      "that cluster");
        // If we're currently in the middle of generating reports for a given cluster and that in turn is marked dirty, let's reset
        // our iterator to point back to the beginning of that cluster. This ensures that the receiver will get a coherent view of
        // the state of the cluster as present on the server
        mAttributePathExpandIterator.ResetCurrentCluster();
        mAttributeEncoderState = AttributeValueEncoder::AttributeEncodeState();
    }

    if (IsReportable())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
}

Transport::SecureSession * ReadHandler::GetSession() const
{
    if (!mSessionHandle)
    {
        return nullptr;
    }
    return mSessionHandle->AsSecureSession();
}

void ReadHandler::UnblockUrgentEventDelivery()
{
    SetStateFlag(ReadHandlerFlags::ForceDirty);
}

void ReadHandler::SetStateFlag(ReadHandlerFlags aFlag, bool aValue)
{
    bool oldReportable = IsReportable();
    mFlags.Set(aFlag, aValue);
    // If we became reportable, schedule a reporting run.
    if (!oldReportable && IsReportable())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
}

void ReadHandler::ClearStateFlag(ReadHandlerFlags aFlag)
{
    SetStateFlag(aFlag, false);
}

} // namespace app
} // namespace chip
