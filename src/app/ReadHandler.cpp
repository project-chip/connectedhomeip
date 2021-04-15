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
#include <app/reporting/Engine.h>

namespace chip {
namespace app {
CHIP_ERROR ReadHandler::Init(InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Error if already initialized.
    VerifyOrExit(apDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mpExchangeCtx == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    mpExchangeCtx     = nullptr;
    mpDelegate        = apDelegate;
    mSuppressResponse = true;
    mGetToAllEvents   = true;
    MoveToState(HandlerState::Initialized);

exit:
    ChipLogFunctError(err);
    return err;
}

void ReadHandler::Shutdown()
{
    ClearExistingExchangeContext();
    MoveToState(HandlerState::Uninitialized);
    mpDelegate = nullptr;
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

CHIP_ERROR ReadHandler::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    mpExchangeCtx = apExchangeContext;

    err = ProcessReadRequest(std::move(aPayload));
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogFunctError(err);
        Shutdown();
    }

    return err;
}

CHIP_ERROR ReadHandler::SendReportData(System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::ReportData, std::move(aPayload),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
exit:
    return err;
}

CHIP_ERROR ReadHandler::ProcessReadRequest(System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    ReadRequest::Parser readRequestParser;
    EventPathList::Parser eventPathListParser;
    TLV::TLVReader eventPathListReader;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = readRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = readRequestParser.GetEventPathList(&eventPathListParser);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
        eventPathListParser.GetReader(&eventPathListReader);

        while (CHIP_NO_ERROR == (err = eventPathListReader.Next()))
        {
            VerifyOrExit(TLV::AnonymousTag == eventPathListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

            EventPath::Parser eventPath;

            err = eventPath.Init(eventPathListReader);
            SuccessOrExit(err);
            // TODO: Pass event path to report engine to generate report with interested events
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    MoveToState(HandlerState::Reportable);

    err = InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();

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
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void ReadHandler::MoveToState(const HandlerState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM RH moving to [%s]", GetStateStr());
}

} // namespace app
} // namespace chip
