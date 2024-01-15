/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "CommandResponseSender.h"
#include "InteractionModelEngine.h"
#include "messaging/ExchangeContext.h"

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

CHIP_ERROR CommandResponseSender::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    bool sendStatusResponseWithFailure = false;

    if (mState == State::AwaitingStatusResponse && aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        CHIP_ERROR statusError = CHIP_NO_ERROR;
        err                    = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError);
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithFailure = true);
        err = statusError;
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithFailure = true);

        err = SendCommandResponse();
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithFailure = true);

        if (mState != State::AwaitingStatusResponse)
        {
            // We are sending out the last message and no longer are expecting any responses. As a result we are
            // fulfilling our responsibility to call Close() on CommandHandler by calling ExitNow().
            ExitNow();
        }
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(DataManagement, "CommandResponseSender: Unexpected message type %d", aPayloadHeader.GetMessageType());
    StatusResponse::Send(Status::InvalidAction, mExchangeCtx.Get(), false /*aExpectResponse*/);
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    if (mState == State::AwaitingStatusResponse)
    {
        // We were waiting for a StatusResponse and received something unexpected. As a result,
        // we've sent a StatusResponse indicating an issue, and we do not anticipate any further
        // responses. Consequently, we are simply fulfilling our responsibility to call Close()
        // on CommandHandler by calling ExitNow().
        ExitNow();
    }
    return err;
exit:
    if (sendStatusResponseWithFailure)
    {
        StatusResponse::Send(Status::Failure, mExchangeCtx.Get(), false /*aExpectResponse*/);
    }
    Close();
    return err;
}

void CommandResponseSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogDetail(DataManagement, "CommandResponseSender: Timed out waiting for response from requester mState=[%10.10s]",
                  GetStateStr());
    Close();
}

CHIP_ERROR CommandResponseSender::StartSendingCommandResponse()
{
    ReturnErrorOnFailure(SendCommandResponse());

    bool moreToSend = !mChunks.IsNull();
    if (moreToSend)
    {
        MoveToState(State::AwaitingStatusResponse);
        mExchangeCtx->SetDelegate(this);
    }
    else
    {
        Close();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandResponseSender::SendCommandResponse()
{
    VerifyOrReturnError(!mChunks.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    System::PacketBufferHandle commandResponsePayload = mChunks.PopHead();

    bool moreToSend               = !mChunks.IsNull();
    Messaging::SendFlags sendFlag = Messaging::SendMessageFlags::kNone;
    if (moreToSend)
    {
        sendFlag = Messaging::SendMessageFlags::kExpectResponse;
    }

    if (moreToSend)
    {
        mExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }

    // TODO still need to figure out what should be responsible for closing the exchange if command
    // below fails.
    ReturnErrorOnFailure(mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse,
                                                   std::move(commandResponsePayload), sendFlag));

    return CHIP_NO_ERROR;
}

const char * CommandResponseSender::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::ReadyForInvokeResponses:
        return "ReadyForInvokeResponses";

    case State::AwaitingStatusResponse:
        return "AwaitingStatusResponse";

    case State::AllInvokeResponsesSent:
        return "AllInvokeResponsesSent";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void CommandResponseSender::MoveToState(const State aTargetState)
{
    if (mState == aTargetState)
    {
        return;
    }
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Command response sender moving to [%10.10s]", GetStateStr());
}

void CommandResponseSender::Close()
{
    MoveToState(State::AllInvokeResponsesSent);
    mCloseCalled = true;
    if (mResponseSenderDoneCallback)
    {
        mResponseSenderDoneCallback->mCall(mResponseSenderDoneCallback->mContext);
    }
}

} // namespace app
} // namespace chip
