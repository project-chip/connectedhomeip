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
#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/TLVUtilities.h>
#include <messaging/ExchangeContext.h>

#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> //nogncheck
#endif

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

uint16_t ReadHandler::GetPublisherSelectedIntervalLimit()
{
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    return std::chrono::duration_cast<System::Clock::Seconds16>(ICDConfigurationData::GetInstance().GetIdleModeDuration()).count();
#else
    return kSubscriptionMaxIntervalPublisherLimit;
#endif
}

ReadHandler::ReadHandler(ManagementCallback & apCallback, Messaging::ExchangeContext * apExchangeContext,
                         InteractionType aInteractionType, Observer * observer) :
    mExchangeCtx(*this),
    mManagementCallback(apCallback)
{
    VerifyOrDie(apExchangeContext != nullptr);

    mExchangeCtx.Grab(apExchangeContext);
#if CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
    // TODO: this should be replaced by a pointer to the InteractionModelEngine that created the ReadHandler
    // once InteractionModelEngine is no longer a singleton (see issue 23625)
    mExchangeMgr = apExchangeContext->GetExchangeMgr();
#endif // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE

    mInteractionType            = aInteractionType;
    mLastWrittenEventsBytes     = 0;
    mTransactionStartGeneration = mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().GetDirtySetGeneration();
    mFlags.ClearAll();
    SetStateFlag(ReadHandlerFlags::PrimingReports);

    mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());

    VerifyOrDie(observer != nullptr);
    mObserver = observer;
}

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
ReadHandler::ReadHandler(ManagementCallback & apCallback, Observer * observer) :
    mExchangeCtx(*this), mManagementCallback(apCallback)
{
    mInteractionType = InteractionType::Subscribe;
    mFlags.ClearAll();

    VerifyOrDie(observer != nullptr);
    mObserver = observer;
}

void ReadHandler::OnSubscriptionResumed(const SessionHandle & sessionHandle,
                                        SubscriptionResumptionSessionEstablisher & resumptionSessionEstablisher)
{
    mSubscriptionId          = resumptionSessionEstablisher.mSubscriptionInfo.mSubscriptionId;
    mMinIntervalFloorSeconds = resumptionSessionEstablisher.mSubscriptionInfo.mMinInterval;
    mMaxInterval             = resumptionSessionEstablisher.mSubscriptionInfo.mMaxInterval;
    SetStateFlag(ReadHandlerFlags::FabricFiltered, resumptionSessionEstablisher.mSubscriptionInfo.mFabricFiltered);

    // Move dynamically allocated attributes and events from the SubscriptionInfo struct into
    // the object pool managed by the IM engine
    for (size_t i = 0; i < resumptionSessionEstablisher.mSubscriptionInfo.mAttributePaths.AllocatedSize(); i++)
    {
        AttributePathParams params = resumptionSessionEstablisher.mSubscriptionInfo.mAttributePaths[i].GetParams();
        CHIP_ERROR err = mManagementCallback.GetInteractionModelEngine()->PushFrontAttributePathList(mpAttributePathList, params);
        if (err != CHIP_NO_ERROR)
        {
            Close();
            return;
        }
    }
    for (size_t i = 0; i < resumptionSessionEstablisher.mSubscriptionInfo.mEventPaths.AllocatedSize(); i++)
    {
        EventPathParams params = resumptionSessionEstablisher.mSubscriptionInfo.mEventPaths[i].GetParams();
        CHIP_ERROR err = mManagementCallback.GetInteractionModelEngine()->PushFrontEventPathParamsList(mpEventPathList, params);
        if (err != CHIP_NO_ERROR)
        {
            Close();
            return;
        }
    }

    mSessionHandle.Grab(sessionHandle);

    SetStateFlag(ReadHandlerFlags::ActiveSubscription);

    auto * appCallback = mManagementCallback.GetAppCallback();
    if (appCallback)
    {
        appCallback->OnSubscriptionEstablished(*this);
    }
    // Notify the observer that a subscription has been resumed
    mObserver->OnSubscriptionEstablished(this);

    MoveToState(HandlerState::CanStartReporting);

    SingleLinkedListNode<AttributePathParams> * attributePath = mpAttributePathList;
    while (attributePath)
    {
        mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().SetDirty(attributePath->mValue);
        attributePath = attributePath->mpNext;
    }
}

#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

ReadHandler::~ReadHandler()
{
    mObserver->OnReadHandlerDestroyed(this);

    auto * appCallback = mManagementCallback.GetAppCallback();
    if (mFlags.Has(ReadHandlerFlags::ActiveSubscription) && appCallback)
    {
        appCallback->OnSubscriptionTerminated(*this);
    }

    if (IsAwaitingReportResponse())
    {
        mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().OnReportConfirm();
    }
    mManagementCallback.GetInteractionModelEngine()->ReleaseAttributePathList(mpAttributePathList);
    mManagementCallback.GetInteractionModelEngine()->ReleaseEventPathList(mpEventPathList);
    mManagementCallback.GetInteractionModelEngine()->ReleaseDataVersionFilterList(mpDataVersionFilterList);
}

void ReadHandler::Close(CloseOptions options)
{
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    if (IsType(InteractionType::Subscribe) && options == CloseOptions::kDropPersistedSubscription)
    {
        auto * subscriptionResumptionStorage = mManagementCallback.GetInteractionModelEngine()->GetSubscriptionResumptionStorage();
        if (subscriptionResumptionStorage)
        {
            subscriptionResumptionStorage->Delete(GetInitiatorNodeId(), GetAccessingFabricIndex(), mSubscriptionId);
        }
    }
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
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
        // At this point we can't have a persisted subscription, since that
        // happens only when ProcessSubscribeRequest returns success. And our
        // subscription id is almost certainly not actually useful at this
        // point, either.  So don't try to mess with persisted subscriptions in
        // Close().
        Close(CloseOptions::kKeepPersistedSubscription);
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
                mObserver->OnSubscriptionEstablished(this);
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

        MoveToState(HandlerState::CanStartReporting);
        break;

    case HandlerState::CanStartReporting:
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
    VerifyOrReturnLogError(mState == HandlerState::CanStartReporting, CHIP_ERROR_INCORRECT_STATE);
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
#if CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        auto exchange = mExchangeMgr->NewContext(mSessionHandle.Get().Value(), this);
#else  // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        auto exchange =
            mManagementCallback.GetInteractionModelEngine()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
#endif // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        VerifyOrReturnLogError(exchange != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mExchangeCtx.Grab(exchange);
    }

    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
    return StatusResponse::Send(aStatus, mExchangeCtx.Get(), /* aExpectResponse = */ false);
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks)
{
    VerifyOrReturnLogError(mState == HandlerState::CanStartReporting, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrDie(!IsAwaitingReportResponse()); // Should not be reportable!
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
#if CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        auto exchange = mExchangeMgr->NewContext(mSessionHandle.Get().Value(), this);
#else  // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        auto exchange =
            mManagementCallback.GetInteractionModelEngine()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
#endif // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
        VerifyOrReturnLogError(exchange != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mExchangeCtx.Grab(exchange);
    }

    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);

    if (!IsReporting())
    {
        mCurrentReportsBeginGeneration =
            mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().GetDirtySetGeneration();
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
            mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().OnReportConfirm();
        }

        // If we just finished a non-priming subscription report, notify our observers.
        // Priming reports are handled when we send a SubscribeResponse.
        if (IsType(InteractionType::Subscribe) && !IsPriming() && !IsChunkedReport())
        {
            mObserver->OnSubscriptionReportSent(this);
        }
    }
    if (!aMoreChunks)
    {
        mPreviousReportsBeginGeneration = mCurrentReportsBeginGeneration;
        ClearForceDirtyFlag();
        mManagementCallback.GetInteractionModelEngine()->ReleaseDataVersionFilterList(mpDataVersionFilterList);
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
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    Close(CloseOptions::kKeepPersistedSubscription);
#else
    Close();
#endif
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

    // No need to pretty-print here.  We pretty-print read requests in the read
    // case of InteractionModelEngine::OnReadInitialRequest, so we do it even if
    // we reject a read request.

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
    MoveToState(HandlerState::CanStartReporting);

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
        ReturnErrorOnFailure(
            mManagementCallback.GetInteractionModelEngine()->PushFrontAttributePathList(mpAttributePathList, attribute));
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        mManagementCallback.GetInteractionModelEngine()->RemoveDuplicateConcreteAttributePath(mpAttributePathList);
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
        ReturnErrorOnFailure(mManagementCallback.GetInteractionModelEngine()->PushFrontDataVersionFilterList(
            mpDataVersionFilterList, versionFilter));
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
        ReturnErrorOnFailure(mManagementCallback.GetInteractionModelEngine()->PushFrontEventPathParamsList(mpEventPathList, event));
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
    case HandlerState::CanStartReporting:
        return "CanStartReporting";

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
        mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().OnReportConfirm();
    }

    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());

    //
    // If we just unblocked sending reports, let's go ahead and schedule the reporting
    // engine to run to kick that off.
    //
    if (aTargetState == HandlerState::CanStartReporting)
    {
        if (ShouldReportUnscheduled())
        {
            mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().ScheduleRun();
        }
        else
        {
            // If we became reportable, the scheduler will schedule a run as soon as allowed
            mObserver->OnBecameReportable(this);
        }
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
    ReturnErrorOnFailure(response.SubscriptionId(mSubscriptionId).MaxInterval(mMaxInterval).EndOfSubscribeResponseMessage());

    ReturnErrorOnFailure(writer.Finalize(&packet));
    VerifyOrReturnLogError(mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);

    ClearStateFlag(ReadHandlerFlags::PrimingReports);
    return mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeResponse, std::move(packet));
}

CHIP_ERROR ReadHandler::ProcessSubscribeRequest(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));

    SubscribeRequestMessage::Parser subscribeRequestParser;
    ReturnErrorOnFailure(subscribeRequestParser.Init(reader));

    // No need to pretty-print here.  We pretty-print subscribe requests in the
    // subscribe case of InteractionModelEngine::OnReadInitialRequest, so we do
    // it even if we reject a subscribe request.

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

#if CHIP_CONFIG_ENABLE_ICD_SERVER

    // Default behavior for ICDs where the wanted MaxInterval for a subscription is the IdleModeDuration
    // defined in the ICD Management Cluster.
    // Behavior can be changed with the OnSubscriptionRequested function defined in the application callbacks

    // Default Behavior Steps :
    // If MinInterval > IdleModeDuration, try to set the MaxInterval to the first interval of IdleModeDurations above the
    // MinInterval.
    // If the next interval is greater than the MaxIntervalCeiling, use the MaxIntervalCeiling.
    // Otherwise, use IdleModeDuration as MaxInterval

    // GetPublisherSelectedIntervalLimit() returns the IdleModeDuration if the device is an ICD
    uint32_t decidedMaxInterval = GetPublisherSelectedIntervalLimit();

    // Check if the PublisherSelectedIntervalLimit is 0. If so, set decidedMaxInterval to MaxIntervalCeiling
    if (decidedMaxInterval == 0)
    {
        decidedMaxInterval = mMaxInterval;
    }

    // If requestedMinInterval is greater than the IdleTimeInterval, select next active up time as max interval
    if (mMinIntervalFloorSeconds > decidedMaxInterval)
    {
        uint16_t ratio = mMinIntervalFloorSeconds / static_cast<uint16_t>(decidedMaxInterval);
        if (mMinIntervalFloorSeconds % decidedMaxInterval)
        {
            ratio++;
        }

        decidedMaxInterval *= ratio;
    }

    // Verify that decidedMaxInterval is an acceptable value (overflow)
    if (decidedMaxInterval > System::Clock::Seconds16::max().count())
    {
        decidedMaxInterval = System::Clock::Seconds16::max().count();
    }

    // Verify that the decidedMaxInterval respects MAX(GetPublisherSelectedIntervalLimit(), MaxIntervalCeiling)
    uint16_t maximumMaxInterval = std::max(GetPublisherSelectedIntervalLimit(), mMaxInterval);
    if (decidedMaxInterval > maximumMaxInterval)
    {
        decidedMaxInterval = maximumMaxInterval;
    }

    // Set max interval of the subscription
    mMaxInterval = static_cast<uint16_t>(decidedMaxInterval);

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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
    MoveToState(HandlerState::CanStartReporting);

    mExchangeCtx->WillSendMessage();

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    PersistSubscription();
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    return CHIP_NO_ERROR;
}

void ReadHandler::PersistSubscription()
{
    auto * subscriptionResumptionStorage = mManagementCallback.GetInteractionModelEngine()->GetSubscriptionResumptionStorage();
    VerifyOrReturn(subscriptionResumptionStorage != nullptr);

    SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo = { .mNodeId         = GetInitiatorNodeId(),
                                                                         .mFabricIndex    = GetAccessingFabricIndex(),
                                                                         .mSubscriptionId = mSubscriptionId,
                                                                         .mMinInterval    = mMinIntervalFloorSeconds,
                                                                         .mMaxInterval    = mMaxInterval,
                                                                         .mFabricFiltered = IsFabricFiltered() };
    VerifyOrReturn(subscriptionInfo.SetAttributePaths(mpAttributePathList) == CHIP_NO_ERROR);
    VerifyOrReturn(subscriptionInfo.SetEventPaths(mpEventPathList) == CHIP_NO_ERROR);

    CHIP_ERROR err = subscriptionResumptionStorage->Save(subscriptionInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to save subscription info error: '%" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ReadHandler::ResetPathIterator()
{
    mAttributePathExpandIterator = AttributePathExpandIterator(mpAttributePathList);
    mAttributeEncoderState       = AttributeValueEncoder::AttributeEncodeState();
}

void ReadHandler::AttributePathIsDirty(const AttributePathParams & aAttributeChanged)
{
    ConcreteAttributePath path;

    mDirtyGeneration = mManagementCallback.GetInteractionModelEngine()->GetReportingEngine().GetDirtySetGeneration();

    // We won't reset the path iterator for every AttributePathIsDirty call to reduce the number of full data reports.
    // The iterator will be reset after finishing each report session.
    //
    // Here we just reset the iterator to the beginning of the current cluster, if the dirty path affects it.
    // This will ensure the reports are consistent within a single cluster generated from a single path in the request.

    // TODO (#16699): Currently we can only guarantee the reports generated from a single path in the request are consistent. The
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

    // ReportScheduler will take care of verifying the reportability of the handler and schedule the run
    mObserver->OnBecameReportable(this);
}

Transport::SecureSession * ReadHandler::GetSession() const
{
    if (!mSessionHandle)
    {
        return nullptr;
    }
    return mSessionHandle->AsSecureSession();
}

void ReadHandler::ForceDirtyState()
{
    SetStateFlag(ReadHandlerFlags::ForceDirty);
}

void ReadHandler::SetStateFlag(ReadHandlerFlags aFlag, bool aValue)
{
    bool oldReportable = ShouldStartReporting();
    mFlags.Set(aFlag, aValue);

    // If we became reportable, schedule a reporting run.
    if (!oldReportable && ShouldStartReporting())
    {
        // If we became reportable, the scheduler will schedule a run as soon as allowed
        mObserver->OnBecameReportable(this);
    }
}

void ReadHandler::ClearStateFlag(ReadHandlerFlags aFlag)
{
    SetStateFlag(aFlag, false);
}

} // namespace app
} // namespace chip
