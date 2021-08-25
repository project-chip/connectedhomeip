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
#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>
#include <protocols/secure_channel/StatusReport.h>

namespace chip {
namespace app {
CHIP_ERROR ReadHandler::Init(InteractionModelDelegate * apDelegate, Messaging::ExchangeContext * apExchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrReturnError(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    mpExchangeCtx              = apExchangeContext;
    mSuppressResponse          = true;
    mpAttributeClusterInfoList = nullptr;
    mpEventClusterInfoList     = nullptr;
    mCurrentPriority           = PriorityLevel::Invalid;
    MoveToState(HandlerState::Initialized);
    mpDelegate = apDelegate;
    if (apExchangeContext != nullptr)
    {
        apExchangeContext->SetDelegate(this);
    }

    return err;
}

void ReadHandler::Shutdown()
{
    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpAttributeClusterInfoList);
    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpEventClusterInfoList);
    mpExchangeCtx = nullptr;
    MoveToState(HandlerState::Uninitialized);
    mpAttributeClusterInfoList = nullptr;
    mpEventClusterInfoList     = nullptr;
    mCurrentPriority           = PriorityLevel::Invalid;
    mpDelegate                 = nullptr;
}

CHIP_ERROR ReadHandler::OnReadRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    err = ProcessReadRequest(std::move(aPayload));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogFunctError(err);
        Shutdown();
    }

    return err;
}

CHIP_ERROR ReadHandler::OnStatusReport(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Protocols::SecureChannel::StatusReport statusReport;
    err = statusReport.Parse(std::move(aPayload));
    SuccessOrExit(err);
    ChipLogProgress(DataManagement, "Receive Status Report, protocol id is %" PRIu32 ", protocol code is %" PRIu16,
                    statusReport.GetProtocolId(), statusReport.GetProtocolCode());
    if ((statusReport.GetProtocolId() == Protocols::InteractionModel::Id.ToFullyQualifiedSpecForm()) &&
        (statusReport.GetProtocolCode() == to_underlying(Protocols::InteractionModel::ProtocolCode::Success)))
    {
        switch (mState)
        {
        case HandlerState::Reporting:
            InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
            break;
        case HandlerState::Reportable:
        case HandlerState::Initialized:
        case HandlerState::Uninitialized:
        default:
            err = CHIP_ERROR_INCORRECT_STATE;
            break;
        }
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

exit:
    Shutdown();
    return err;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(IsReportable(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    if (err == CHIP_NO_ERROR)
    {
        MoveToState(HandlerState::Reporting);
    }

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR)
    {
        Shutdown();
    }
    return err;
}

CHIP_ERROR ReadHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                          const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPayloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        err = OnStatusReport(apExchangeContext, std::move(aPayload));
    }
    else
    {
        err = OnUnknownMsgType(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    return err;
}

CHIP_ERROR ReadHandler::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                         const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());
    if (IsReporting())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }
    Shutdown();
    return CHIP_ERROR_INVALID_MESSAGE_TYPE;
}

void ReadHandler::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive status response from Exchange: %d",
                    apExchangeContext->GetExchangeId());
    if (IsReporting())
    {
        InteractionModelEngine::GetInstance()->GetReportingEngine().OnReportConfirm();
    }
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

    MoveToState(HandlerState::Reportable);

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
    ChipLogFunctError(err);
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
        mpAttributeClusterInfoList->SetDirty();
    }
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);
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
    ChipLogFunctError(err);
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

    case HandlerState::Reportable:
        return "Reportable";

    case HandlerState::Reporting:
        return "Reporting";
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
} // namespace app
} // namespace chip
