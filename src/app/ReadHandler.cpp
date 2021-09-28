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
#include <app/MessageDef/EventPath.h>
#include <app/MessageDef/StatusResponse.h>
#include <app/MessageDef/SubscribeRequest.h>
#include <app/MessageDef/SubscribeResponse.h>
#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>

namespace chip {
namespace app {
CHIP_ERROR ReadHandler::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate,
                             Messaging::ExchangeContext * apExchangeContext, InteractionType aInteractionType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrReturnError(IsFree(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    mpExchangeMgr              = apExchangeMgr;
    mpExchangeCtx              = apExchangeContext;
    mSuppressResponse          = true;
    mpAttributeClusterInfoList = nullptr;
    mpEventClusterInfoList     = nullptr;
    mCurrentPriority           = PriorityLevel::Invalid;
    mInitialReport             = true;
    MoveToState(HandlerState::Initialized);
    mpDelegate       = apDelegate;
    mSubscriptionId  = 0;
    mHoldReport      = false;
    mDirty           = false;
    mInteractionType = aInteractionType;
    if (apExchangeContext != nullptr)
    {
        apExchangeContext->SetDelegate(this);
    }

    return err;
}

void ReadHandler::Shutdown(ShutdownOptions aOptions)
{
    if (IsSubscriptionType())
    {
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            OnRefreshSubscribeTimerSyncCallback, this);
    }
    if (aOptions == ShutdownOptions::AbortCurrentExchange)
    {
        if (mpExchangeCtx != nullptr)
        {
            mpExchangeCtx->Abort();
            mpExchangeCtx = nullptr;
        }
    }

    if (IsAwaitingReportResponse())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }
    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpAttributeClusterInfoList);
    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpEventClusterInfoList);
    mSubscriptionId            = 0;
    mMinIntervalFloorSeconds   = 0;
    mMaxIntervalCeilingSeconds = 0;
    mInteractionType           = InteractionType::Read;
    mpExchangeCtx              = nullptr;
    MoveToState(HandlerState::Uninitialized);
    mpAttributeClusterInfoList = nullptr;
    mpEventClusterInfoList     = nullptr;
    mCurrentPriority           = PriorityLevel::Invalid;
    mInitialReport             = false;
    mpDelegate                 = nullptr;
    mHoldReport                = false;
    mDirty                     = false;
}

CHIP_ERROR ReadHandler::OnReadInitialRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;
    if (IsSubscriptionType())
    {
        err = ProcessSubscribeRequest(std::move(aPayload));
    }
    else
    {
        err = ProcessReadRequest(std::move(aPayload));
    }

    if (err != CHIP_NO_ERROR)
    {
        Shutdown();
    }

    return err;
}

CHIP_ERROR ReadHandler::OnStatusResponse(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status statusCode;
    StatusResponse::Parser response;
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));
    reader.Next();
    err = response.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = response.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = response.GetStatus(statusCode);
    SuccessOrExit(err);

    ChipLogProgress(DataManagement, "In state %s, receive status response, status code is %" PRIu16, GetStateStr(),
                    to_underlying(statusCode));
    VerifyOrExit((statusCode == Protocols::InteractionModel::Status::Success), err = CHIP_ERROR_INVALID_ARGUMENT);
    switch (mState)
    {
    case HandlerState::AwaitingReportResponse:
        if (IsSubscriptionType())
        {
            InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
            if (IsInitialReport())
            {
                err = SendSubscribeResponse();
                SuccessOrExit(err);
            }
            else
            {
                MoveToState(HandlerState::GeneratingReports);
            }
        }
        else
        {
            Shutdown();
        }
        break;
    case HandlerState::GeneratingReports:
    case HandlerState::Initialized:
    case HandlerState::Uninitialized:
    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        Shutdown();
    }
    return err;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload)
{
    VerifyOrReturnLogError(IsReportable(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (IsInitialReport())
    {
        VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mSessionHandle.SetValue(mpExchangeCtx->GetSecureSession());
    }
    else
    {
        mpExchangeCtx = mpExchangeMgr->NewContext(mSessionHandle.Value(), this);
        mpExchangeCtx->SetResponseTimeout(kImMessageTimeoutMsec);
    }
    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    MoveToState(HandlerState::AwaitingReportResponse);
    CHIP_ERROR err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                                Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    if (err == CHIP_NO_ERROR)
    {
        if (IsSubscriptionType() && !IsInitialReport())
        {
            err = RefreshSubscribeSyncTimer();
        }
    }
    ClearDirty();
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

CHIP_ERROR ReadHandler::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                         System::PacketBufferHandle && aPayload)
{
    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());
    Shutdown();
    return CHIP_ERROR_INVALID_MESSAGE_TYPE;
}

void ReadHandler::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive status response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));
    Shutdown();
}

CHIP_ERROR ReadHandler::ProcessReadRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    ReadRequest::Parser readRequestParser;
    EventPathList::Parser eventPathListParser;
    AttributePathList::Parser attributePathListParser;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = readRequestParser.Init(reader);
    SuccessOrExit(err);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = readRequestParser.GetAttributePathList(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
        err = ProcessAttributePathList(attributePathListParser);
    }
    SuccessOrExit(err);
    err = readRequestParser.GetEventPathList(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
        err = ProcessEventPathList(eventPathListParser);
    }
    SuccessOrExit(err);

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    MoveToState(HandlerState::GeneratingReports);

    err = InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    SuccessOrExit(err);

    // mpExchangeCtx can be null here due to
    // https://github.com/project-chip/connectedhomeip/issues/8031
    if (mpExchangeCtx)
    {
        mpExchangeCtx->WillSendMessage();
    }

    // There must be no code after the WillSendMessage() call that can cause
    // this method to return a failure.

exit:
    return err;
}

CHIP_ERROR ReadHandler::ProcessAttributePathList(AttributePathList::Parser & aAttributePathListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aAttributePathListParser.GetReader(&reader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
        ClusterInfo clusterInfo;
        AttributePath::Parser path;
        err = path.Init(reader);
        SuccessOrExit(err);
        err = path.GetNodeId(&(clusterInfo.mNodeId));
        SuccessOrExit(err);
        err = path.GetEndpointId(&(clusterInfo.mEndpointId));
        SuccessOrExit(err);
        err = path.GetClusterId(&(clusterInfo.mClusterId));
        SuccessOrExit(err);
        err = path.GetFieldId(&(clusterInfo.mFieldId));
        if (CHIP_NO_ERROR == err)
        {
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);

        err = path.GetListIndex(&(clusterInfo.mListIndex));
        if (CHIP_NO_ERROR == err)
        {
            VerifyOrExit(clusterInfo.mFlags.Has(ClusterInfo::Flags::kFieldIdValid), err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);
        err = InteractionModelEngine::GetInstance()->PushFront(mpAttributeClusterInfoList, clusterInfo);
        SuccessOrExit(err);
        mInitialReport = true;
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

CHIP_ERROR ReadHandler::ProcessEventPathList(EventPathList::Parser & aEventPathListParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    aEventPathListParser.GetReader(&reader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
        ClusterInfo clusterInfo;
        EventPath::Parser path;
        err = path.Init(reader);
        SuccessOrExit(err);
        err = path.GetNodeId(&(clusterInfo.mNodeId));
        SuccessOrExit(err);
        err = path.GetEndpointId(&(clusterInfo.mEndpointId));
        SuccessOrExit(err);
        err = path.GetClusterId(&(clusterInfo.mClusterId));
        SuccessOrExit(err);
        err = path.GetEventId(&(clusterInfo.mEventId));
        if (CHIP_NO_ERROR == err)
        {
            clusterInfo.mFlags.Set(ClusterInfo::Flags::kEventIdValid);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);
        err = InteractionModelEngine::GetInstance()->PushFront(mpEventClusterInfoList, clusterInfo);
        SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

const char * ReadHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case HandlerState::Uninitialized:
        return "Uninitialized";

    case HandlerState::Initialized:
        return "Initialized";

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
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());
}

bool ReadHandler::CheckEventClean(EventManagement & aEventManager)
{
    if (mCurrentPriority == PriorityLevel::Invalid)
    {
        // Upload is not in middle, previous mLastScheduledEventNumber is not valid, Check for new events from Critical high
        // priority to Debug low priority, and set a checkpoint when there is dirty events
        for (int index = ArraySize(mSelfProcessedEvents) - 1; index >= 0; index--)
        {
            EventNumber lastEventNumber = aEventManager.GetLastEventNumber(static_cast<PriorityLevel>(index));
            if ((lastEventNumber != 0) && (lastEventNumber >= mSelfProcessedEvents[index]))
            {
                // We have more events. snapshot last event IDs
                aEventManager.SetScheduledEventEndpoint(&(mLastScheduledEventNumber[0]));
                // initialize the next dirty priority level to transfer
                MoveToNextScheduledDirtyPriority();
                return false;
            }
        }
        return true;
    }
    else
    {
        // Upload is in middle, previous mLastScheduledEventNumber is still valid, recheck via MoveToNextScheduledDirtyPriority,
        // if finally mCurrentPriority is invalid, it means no more event
        MoveToNextScheduledDirtyPriority();
        return mCurrentPriority == PriorityLevel::Invalid;
    }
}

void ReadHandler::MoveToNextScheduledDirtyPriority()
{
    for (int i = ArraySize(mSelfProcessedEvents) - 1; i >= 0; i--)
    {
        if ((mLastScheduledEventNumber[i] != 0) && mSelfProcessedEvents[i] <= mLastScheduledEventNumber[i])
        {
            mCurrentPriority = static_cast<PriorityLevel>(i);
            return;
        }
    }

    mCurrentPriority = PriorityLevel::Invalid;
}

CHIP_ERROR ReadHandler::SendSubscribeResponse()
{
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnLogError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(packet));

    SubscribeResponse::Builder response;
    ReturnLogErrorOnFailure(response.Init(&writer));
    response.SubscriptionId(mSubscriptionId)
        .MinIntervalFloorSeconds(mMinIntervalFloorSeconds)
        .MaxIntervalCeilingSeconds(mMaxIntervalCeilingSeconds)
        .EndOfSubscribeResponse();
    ReturnLogErrorOnFailure(response.GetError());

    ReturnLogErrorOnFailure(writer.Finalize(&packet));
    VerifyOrReturnLogError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnLogErrorOnFailure(RefreshSubscribeSyncTimer());
    mInitialReport = false;
    MoveToState(HandlerState::GeneratingReports);
    if (mpDelegate != nullptr)
    {
        mpDelegate->SubscriptionEstablished(this);
    }
    return mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::SubscribeResponse, std::move(packet));
}

CHIP_ERROR ReadHandler::ProcessSubscribeRequest(System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(std::move(aPayload));

    ReturnLogErrorOnFailure(reader.Next());
    SubscribeRequest::Parser subscribeRequestParser;
    ReturnLogErrorOnFailure(subscribeRequestParser.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnLogErrorOnFailure(subscribeRequestParser.CheckSchemaValidity());
#endif

    AttributePathList::Parser attributePathListParser;
    CHIP_ERROR err = subscribeRequestParser.GetAttributePathList(&attributePathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnLogErrorOnFailure(ProcessAttributePathList(attributePathListParser));
    }
    ReturnLogErrorOnFailure(err);

    EventPathList::Parser eventPathListParser;
    err = subscribeRequestParser.GetEventPathList(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        ReturnLogErrorOnFailure(ProcessEventPathList(eventPathListParser));
    }
    ReturnLogErrorOnFailure(err);

    ReturnLogErrorOnFailure(subscribeRequestParser.GetMinIntervalSeconds(&mMinIntervalFloorSeconds));
    ReturnLogErrorOnFailure(subscribeRequestParser.GetMaxIntervalSeconds(&mMaxIntervalCeilingSeconds));

    ReturnLogErrorOnFailure(Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&mSubscriptionId), sizeof(mSubscriptionId)));

    MoveToState(HandlerState::GeneratingReports);

    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    // mpExchangeCtx can be null here due to
    // https://github.com/project-chip/connectedhomeip/issues/8031
    if (mpExchangeCtx)
    {
        mpExchangeCtx->WillSendMessage();
    }
    return CHIP_NO_ERROR;
}

void ReadHandler::OnRefreshSubscribeTimerSyncCallback(System::Layer * apSystemLayer, void * apAppState)
{
    ReadHandler * aReadHandler = static_cast<ReadHandler *>(apAppState);
    aReadHandler->mHoldReport  = false;
    InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
}

CHIP_ERROR ReadHandler::RefreshSubscribeSyncTimer()
{
    ChipLogProgress(DataManagement, "ReadHandler::Refresh Subscribe Sync Timer with %d seconds", mMinIntervalFloorSeconds);
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
        OnRefreshSubscribeTimerSyncCallback, this);
    mHoldReport = true;
    return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
        mMinIntervalFloorSeconds * kMillisecondsPerSecond, OnRefreshSubscribeTimerSyncCallback, this);
}
} // namespace app
} // namespace chip
