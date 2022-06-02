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

#include <app/AppBuildConfig.h>
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

ReadHandler::ReadHandler(ManagementCallback & apCallback, Messaging::ExchangeContext * apExchangeContext,
                         InteractionType aInteractionType) :
    mManagementCallback(apCallback)
{
    mpExchangeCtx                = apExchangeContext;
    mInteractionType             = aInteractionType;
    mLastWrittenEventsBytes      = 0;
    mSubscriptionStartGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    mFlags.ClearAll();
    mFlags.Set(ReadHandlerFlags::PrimingReports, true);
    if (apExchangeContext != nullptr)
    {
        apExchangeContext->SetDelegate(this);
        mSessionHandle.Grab(mpExchangeCtx->GetSessionHandle());
    }
}

void ReadHandler::Abort(bool aCalledFromDestructor)
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

    if (!aCalledFromDestructor)
    {
        Close();
    }
}

ReadHandler::~ReadHandler()
{
    auto * appCallback = mManagementCallback.GetAppCallback();
    if (mFlags.Has(ReadHandlerFlags::ActiveSubscription) && appCallback)
    {
        appCallback->OnSubscriptionTerminated(*this);
    }

    Abort(true);

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
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->SetDelegate(nullptr);
        mpExchangeCtx = nullptr;
    }

    MoveToState(HandlerState::AwaitingDestruction);
    mManagementCallback.OnDone(*this);
}

CHIP_ERROR ReadHandler::OnInitialRequest(System::PacketBufferHandle && aPayload)
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
        Close();
    }
    else
    {
        // Force us to be in a dirty state so we get processed by the reporting
        mFlags.Set(ReadHandlerFlags::ForceDirty);
    }

    return err;
}

CHIP_ERROR ReadHandler::OnStatusResponse(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = StatusResponse::ProcessStatusResponse(std::move(aPayload));
    SuccessOrExit(err);
    switch (mState)
    {
    case HandlerState::AwaitingReportResponse:
        if (IsChunkedReport())
        {
            mpExchangeCtx->WillSendMessage();
        }
        else if (IsType(InteractionType::Subscribe))
        {
            if (IsPriming())
            {
                err = SendSubscribeResponse();

                mpExchangeCtx = nullptr;
                SuccessOrExit(err);

                mFlags.Set(ReadHandlerFlags::ActiveSubscription);

                auto * appCallback = mManagementCallback.GetAppCallback();
                if (appCallback)
                {
                    appCallback->OnSubscriptionEstablished(*this);
                }
            }
            else
            {
                mpExchangeCtx = nullptr;
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
    if (err != CHIP_NO_ERROR)
    {
        Close();
    }

    return err;
}

CHIP_ERROR ReadHandler::SendStatusReport(Protocols::InteractionModel::Status aStatus)
{
    VerifyOrReturnLogError(IsReportable(), CHIP_ERROR_INCORRECT_STATE);
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mpExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
        mpExchangeCtx = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
    }
    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return StatusResponse::Send(aStatus, mpExchangeCtx,
                                /* aExpectResponse = */ false);
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks)
{
    VerifyOrReturnLogError(IsReportable(), CHIP_ERROR_INCORRECT_STATE);
    if (IsPriming() || IsChunkedReport())
    {
        mSessionHandle.Grab(mpExchangeCtx->GetSessionHandle());
    }
    else
    {
        VerifyOrReturnLogError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnLogError(mSessionHandle, CHIP_ERROR_INCORRECT_STATE);
        mpExchangeCtx = InteractionModelEngine::GetInstance()->GetExchangeManager()->NewContext(mSessionHandle.Get().Value(), this);
    }

    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (!IsReporting())
    {
        mCurrentReportsBeginGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    }
    mFlags.Set(ReadHandlerFlags::ChunkedReport, aMoreChunks);
    bool noResponseExpected = IsType(InteractionType::Read) && !aMoreChunks;
    if (!noResponseExpected)
    {
        MoveToState(HandlerState::AwaitingReportResponse);
    }

    mpExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    CHIP_ERROR err =
        mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                   Messaging::SendFlags(noResponseExpected ? Messaging::SendMessageFlags::kNone
                                                                           : Messaging::SendMessageFlags::kExpectResponse));
    if (err == CHIP_NO_ERROR && noResponseExpected)
    {
        mpExchangeCtx = nullptr;
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }

    if (err == CHIP_NO_ERROR)
    {
        if (IsType(InteractionType::Subscribe) && !IsPriming())
        {
            err = RefreshSubscribeSyncTimer();
        }
    }
    if (!aMoreChunks)
    {
        mPreviousReportsBeginGeneration = mCurrentReportsBeginGeneration;
        ClearDirty();
        InteractionModelEngine::GetInstance()->ReleaseDataVersionFilterList(mpDataVersionFilterList);
    }
    return err;
}

CHIP_ERROR ReadHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                          System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        err = OnStatusResponse(apExchangeContext, std::move(aPayload));
    }
    else
    {
        err = OnUnknownMsgType(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
    return err;
}

bool ReadHandler::IsFromSubscriber(Messaging::ExchangeContext & apExchangeContext) const
{
    return (IsType(InteractionType::Subscribe) &&
            GetInitiatorNodeId() == apExchangeContext.GetSessionHandle()->AsSecureSession()->GetPeerNodeId() &&
            GetAccessingFabricIndex() == apExchangeContext.GetSessionHandle()->GetFabricIndex());
}

CHIP_ERROR ReadHandler::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                         System::PacketBufferHandle && aPayload)
{
    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());
    Close();
    return CHIP_ERROR_INVALID_MESSAGE_TYPE;
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
    err = readRequestParser.CheckSchemaValidity();
    if (err != CHIP_NO_ERROR)
    {
        // The actual error we want to return to our consumer is an IM "invalid
        // action" error, not whatever internal error CheckSchemaValidity
        // happens to come up with.
        return CHIP_IM_GLOBAL_STATUS(InvalidAction);
    }
#endif

    err = readRequestParser.GetAttributeRequests(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(ProcessAttributePathList(attributePathListParser));
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

    // Ensure the read transaction doesn't exceed the resources dedicated to
    // read transactions.
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->CanEstablishReadTransaction(this));

    bool isFabricFiltered;
    ReturnErrorOnFailure(readRequestParser.GetIsFabricFiltered(&isFabricFiltered));
    mFlags.Set(ReadHandlerFlags::FabricFiltered, isFabricFiltered);
    ReturnErrorOnFailure(readRequestParser.ExitContainer());
    MoveToState(HandlerState::GeneratingReports);

    mpExchangeCtx->WillSendMessage();

    // There must be no code after the WillSendMessage() call that can cause
    // this method to return a failure.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadHandler::ProcessAttributePathList(AttributePathIBs::Parser & aAttributePathListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aAttributePathListParser.GetReader(&reader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag() == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        AttributePathParams attribute;
        AttributePathIB::Parser path;
        err = path.Init(reader);
        SuccessOrExit(err);

        err = path.GetEndpoint(&(attribute.mEndpointId));
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrExit(!attribute.HasWildcardEndpointId(), err = CHIP_IM_GLOBAL_STATUS(InvalidAction));
        }
        else if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        ClusterId clusterId = kInvalidClusterId;
        err                 = path.GetCluster(&clusterId);
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrExit(IsValidClusterId(clusterId), err = CHIP_IM_GLOBAL_STATUS(InvalidAction));
            attribute.mClusterId = clusterId;
        }
        else if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        AttributeId attributeId = kInvalidAttributeId;
        err                     = path.GetAttribute(&attributeId);
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            VerifyOrExit(IsValidAttributeId(attributeId), err = CHIP_IM_GLOBAL_STATUS(InvalidAction));
            attribute.mAttributeId = attributeId;
        }
        SuccessOrExit(err);

        // A wildcard cluster requires that the attribute path either be
        // wildcard or a global attribute.
        VerifyOrExit(!attribute.HasWildcardClusterId() || attribute.HasWildcardAttributeId() ||
                         IsGlobalAttribute(attribute.mAttributeId),
                     err = CHIP_IM_GLOBAL_STATUS(InvalidAction));

        err = path.GetListIndex(&(attribute.mListIndex));
        if (CHIP_NO_ERROR == err)
        {
            VerifyOrExit(!attribute.HasWildcardAttributeId() && !attribute.HasWildcardListIndex(),
                         err = CHIP_IM_GLOBAL_STATUS(InvalidAction));
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);
        err = InteractionModelEngine::GetInstance()->PushFrontAttributePathList(mpAttributePathList, attribute);
        SuccessOrExit(err);
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(mpAttributePathList);
        mAttributePathExpandIterator = AttributePathExpandIterator(mpAttributePathList);
        err                          = CHIP_NO_ERROR;
    }

exit:
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

        err = path.GetEndpoint(&(event.mEndpointId));
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(!event.HasWildcardEndpointId(), err = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
        }
        else if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

        err = path.GetCluster(&(event.mClusterId));
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(!event.HasWildcardClusterId(), err = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
        }
        else if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

        err = path.GetEvent(&(event.mEventId));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        else if (err == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(!event.HasWildcardEventId(), err = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
        }
        ReturnErrorOnFailure(err);

        err = path.GetIsUrgent(&(event.mIsUrgentEvent));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

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

    //
    // If we just unblocked sending reports, let's go ahead and schedule the reporting
    // engine to run to kick that off.
    //
    if (aTargetState == HandlerState::GeneratingReports)
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }

    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());
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
    response.SubscriptionId(mSubscriptionId)
        .MinIntervalFloorSeconds(mMinIntervalFloorSeconds)
        .MaxIntervalCeilingSeconds(mMaxIntervalCeilingSeconds)
        .EndOfSubscribeResponseMessage();
    ReturnErrorOnFailure(response.GetError());

    ReturnErrorOnFailure(writer.Finalize(&packet));
    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(RefreshSubscribeSyncTimer());

    mFlags.Set(ReadHandlerFlags::PrimingReports, false);
    return mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeResponse, std::move(packet));
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
        ReturnErrorOnFailure(ProcessAttributePathList(attributePathListParser));
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
    ReturnErrorOnFailure(subscribeRequestParser.GetMaxIntervalCeilingSeconds(&mMaxIntervalCeilingSeconds));
    VerifyOrReturnError(mMinIntervalFloorSeconds <= mMaxIntervalCeilingSeconds, CHIP_ERROR_INVALID_ARGUMENT);

    //
    // Notify the application (if requested) of the impending subscription and check whether we should still proceed to set it up.
    // This also provides the application an opportunity to modify the negotiated min/max intervals set above.
    //
    auto * appCallback = mManagementCallback.GetAppCallback();
    if (appCallback)
    {
        if (appCallback->OnSubscriptionRequested(*this, *mpExchangeCtx->GetSessionHandle()->AsSecureSession()) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_TRANSACTION_CANCELED;
        }
    }

    ChipLogProgress(DataManagement, "Final negotiated min/max parameters: Min = %ds, Max = %ds", mMinIntervalFloorSeconds,
                    mMaxIntervalCeilingSeconds);

    bool isFabricFiltered;
    ReturnErrorOnFailure(subscribeRequestParser.GetIsFabricFiltered(&isFabricFiltered));
    mFlags.Set(ReadHandlerFlags::FabricFiltered, isFabricFiltered);
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&mSubscriptionId), sizeof(mSubscriptionId)));
    ReturnErrorOnFailure(subscribeRequestParser.ExitContainer());
    MoveToState(HandlerState::GeneratingReports);

    mpExchangeCtx->WillSendMessage();

    return CHIP_NO_ERROR;
}

void ReadHandler::OnUnblockHoldReportCallback(System::Layer * apSystemLayer, void * apAppState)
{
    VerifyOrReturn(apAppState != nullptr);
    ReadHandler * readHandler = static_cast<ReadHandler *>(apAppState);
    ChipLogDetail(DataManagement, "Unblock report hold after min %d seconds", readHandler->mMinIntervalFloorSeconds);
    readHandler->mFlags.Set(ReadHandlerFlags::HoldReport, false);
    if (readHandler->IsDirty())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        System::Clock::Seconds16(readHandler->mMaxIntervalCeilingSeconds - readHandler->mMinIntervalFloorSeconds),
        OnRefreshSubscribeTimerSyncCallback, readHandler);
}

void ReadHandler::OnRefreshSubscribeTimerSyncCallback(System::Layer * apSystemLayer, void * apAppState)
{
    VerifyOrReturn(apAppState != nullptr);
    ReadHandler * readHandler = static_cast<ReadHandler *>(apAppState);
    readHandler->mFlags.Set(ReadHandlerFlags::HoldSync, false);
    ChipLogDetail(DataManagement, "Refresh subscribe timer sync after %d seconds",
                  readHandler->mMaxIntervalCeilingSeconds - readHandler->mMinIntervalFloorSeconds);
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
}

CHIP_ERROR ReadHandler::RefreshSubscribeSyncTimer()
{
    ChipLogDetail(DataManagement, "Refresh Subscribe Sync Timer with max %d seconds", mMaxIntervalCeilingSeconds);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnUnblockHoldReportCallback, this);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnRefreshSubscribeTimerSyncCallback, this);

    mFlags.Set(ReadHandlerFlags::HoldReport);
    mFlags.Set(ReadHandlerFlags::HoldSync);

    ReturnErrorOnFailure(
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            System::Clock::Seconds16(mMinIntervalFloorSeconds), OnUnblockHoldReportCallback, this));

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
}

Transport::SecureSession * ReadHandler::GetSession() const
{
    if (!mSessionHandle)
    {
        return nullptr;
    }
    return mSessionHandle->AsSecureSession();
}
} // namespace app
} // namespace chip
