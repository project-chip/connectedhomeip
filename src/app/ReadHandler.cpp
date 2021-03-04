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
CHIP_ERROR ReadHandler::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(mpExchangeMgr == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr     = apExchangeMgr;
    mpExchangeCtx     = nullptr;
    mpDelegate        = apDelegate;
    mCurrentPriority  = reporting::PriorityLevel::Invalid;
    mSuppressResponse = true;
    mGetToAllEvents   = true;
    mState            = HandlerState::Initialized;

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadHandler::Shutdown()
{
    ClearExistingExchangeContext();
    MoveToState(HandlerState::Uninitialized);
    mpExchangeMgr = nullptr;
    mpDelegate    = nullptr;
}

CHIP_ERROR ReadHandler::ClearExistingExchangeContext()
{
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

void ReadHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    mpExchangeCtx = apExchangeContext;

    err = ProcessReadRequest(std::move(aPayload));
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
    SuccessOrExit(err);
    MoveToState(HandlerState::ReportDataSending);

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

        // TODO: Process and offer EventPath for RE to filter interested path
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    InteractionModelEngine::GetInstance()->GetReportingEngine()->ScheduleRun();

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

    case HandlerState::ReportDataSending:
        return "ReportDataSending";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadHandler::MoveToState(const HandlerState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%10.10s]", GetStateStr());
}

void ReadHandler::ClearState(void)
{
    MoveToState(HandlerState::Uninitialized);
}

bool ReadHandler::CheckEventUpToDate(reporting::LoggingManagement & inLogger)
{
    bool retval = true;

    if (inLogger.IsValid())
    {
        for (uint8_t i = 0; i < ArraySize(mSelfVendedEvents); i++)
        {
            chip::EventNumber eid = inLogger.GetLastEventNumber(
                static_cast<reporting::PriorityLevel>(i + static_cast<uint8_t>(reporting::PriorityLevel::First)));
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
    reporting::PriorityLevel retval = reporting::PriorityLevel::Invalid;

    for (uint8_t i = 0; i < ArraySize(mSelfVendedEvents); i++)
    {
        if ((mLastScheduledEventNumber[i] != 0) && mSelfVendedEvents[i] <= mLastScheduledEventNumber[i])
        {
            retval = static_cast<chip::app::reporting::PriorityLevel>(
                static_cast<reporting::PriorityLevel>(i + static_cast<uint8_t>(reporting::PriorityLevel::First)));
            break;
        }
    }

    return retval;
}

CHIP_ERROR ReadHandler::SetEventLogEndpoint(reporting::LoggingManagement & aLogger)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(aLogger.IsValid(), err = CHIP_ERROR_INCORRECT_STATE);

    err = aLogger.SetLoggingEndpoint(&(mLastScheduledEventNumber[0]), CHIP_NUM_PRIORITY_LEVEL);

exit:
    return err;
}

void ReadHandler::OnReportProcessingComplete()
{
    ChipLogDetail(DataManagement, "OnReportProcessingComplete");
    mCurrentPriority = reporting::PriorityLevel::Invalid;
    Shutdown();
}

} // namespace app
} // namespace chip
