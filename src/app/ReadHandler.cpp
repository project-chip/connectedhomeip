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

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPath.h>
#include <app/ReadHandler.h>

namespace chip {
namespace app {
CHIP_ERROR ReadHandler::Init(Messaging::ExchangeManager * apExchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mSuppressResponse = true;
    mGetToAllEvents   = true;
    mpExchangeMgr     = apExchangeMgr;
    mpExchangeCtx     = nullptr;
    mCurrentPriority  = reporting::kPriorityLevel_Invalid;
    Reset();

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadHandler::Reset()
{
    ClearExistingExchangeContext();
    MoveToState(kState_Initialized);
}

void ReadHandler::Shutdown()
{
    VerifyOrExit(mState != kState_Uninitialized, );
    ClearExistingExchangeContext();
    MoveToState(kState_Uninitialized);
    mpExchangeMgr = nullptr;

exit:
    return;
}

CHIP_ERROR ReadHandler::ClearExistingExchangeContext()
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

void ReadHandler::OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    mpExchangeCtx = apEc;

    err = ProcessReadRequest(std::move(aPayload));
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mpExchangeCtx != NULL, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
    SuccessOrExit(err);
    MoveToState(kState_ReportDataSending);

exit:
    Shutdown();
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR ReadHandler::ProcessReadRequest(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVReader reader;

    ReadRequest::Parser readRequestParser;
    EventPathList::Parser eventPathListParser;
    chip::TLV::TLVReader eventPathListReader;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = readRequestParser.Init(reader);
    SuccessOrExit(err);

    err = readRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);

    err = readRequestParser.GetEventPathList(&eventPathListParser);
    SuccessOrExit(err);

    eventPathListParser.GetReader(&eventPathListReader);

    while (CHIP_NO_ERROR == (err = eventPathListReader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == eventPathListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        EventPath::Parser eventPath;

        err = eventPath.Init(eventPathListReader);
        SuccessOrExit(err);

        err = eventPath.CheckSchemaValidity();
        SuccessOrExit(err);

        // TODO: Process and offer EventPath for RE to filter interested path
        // err = ProcessEventPath(commandElement);
        // SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    InteractionModelEngine::GetInstance()->GetReportingEngine()->Run();

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadHandler::DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam)
{
    IgnoreUnusedVariable(aInParam);
    IgnoreUnusedVariable(aOutParam);

    ChipLogDetail(DataManagement, "%s event: %d", __func__, aEvent);
}

#if CHIP_DETAIL_LOGGING
const char * ReadHandler::GetStateStr() const
{
    switch (mState)
    {
    case kState_Uninitialized:
        return "Uninitialized";

    case kState_Initialized:
        return "Initialized";

    case kState_ReportDataSending:
        return "ReportDataSending";
    }
    return "N/A";
}
#else  // CHIP_DETAIL_LOGGING
const char * ReadHandler::GetStateStr() const
{
    return "N/A";
}
#endif // CHIP_DETAIL_LOGGING

void ReadHandler::MoveToState(const HandlerState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%10.10s]", GetStateStr());
}

void ReadHandler::ClearState(void)
{
    MoveToState(kState_Uninitialized);
}

bool ReadHandler::CheckEventUpToDate(reporting::LoggingManagement & inLogger)
{
    bool retval = true;

    if (inLogger.IsValid())
    {
        for (size_t i = 0; i < sizeof(mSelfVendedEvents) / sizeof(chip::EventId); i++)
        {
            chip::EventId eid = inLogger.GetLastEventID(static_cast<reporting::PriorityLevel>(i + reporting::kPriorityLevel_First));
            if ((eid != 0) && (eid >= mSelfVendedEvents[i]))
            {
                retval = false;
                break;
            }
        }
    }

    return retval;
}

reporting::PriorityLevel ReadHandler::FindNextPriorityForTransfer(void)
{
    reporting::PriorityLevel retval = reporting::kPriorityLevel_Invalid;

    for (size_t i = 0; i < sizeof(mSelfVendedEvents) / sizeof(chip::EventId); i++)
    {
        if ((mLastScheduledEventId[i] != 0) && mSelfVendedEvents[i] <= mLastScheduledEventId[i])
        {
            retval = static_cast<chip::app::reporting::PriorityLevel>(i + reporting::kPriorityLevel_First);
            break;
        }
    }

    return retval;
}

CHIP_ERROR ReadHandler::SetEventLogEndpoint(reporting::LoggingManagement & aLogger)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(aLogger.IsValid(), err = CHIP_ERROR_INCORRECT_STATE);

    err = aLogger.SetLoggingEndpoint(&(mLastScheduledEventId[0]),
                                     reporting::kPriorityLevel_Last - reporting::kPriorityLevel_First + 1, mBytesOffloaded);

exit:
    return err;
}

void ReadHandler::OnReportProcessingComplete()
{
    ChipLogDetail(DataManagement, "OnReportProcessingComplete");
    Shutdown();
}

} // namespace app
} // namespace chip
