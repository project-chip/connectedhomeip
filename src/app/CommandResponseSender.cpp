/*
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Status> failureStatusToSend;

    if (mState == State::AwaitingStatusResponse &&
        aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        CHIP_ERROR statusError = CHIP_NO_ERROR;
        err                    = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError);
        VerifyOrExit(err == CHIP_NO_ERROR, failureStatusToSend.SetValue(Status::InvalidAction));
        err = statusError;
        VerifyOrExit(err == CHIP_NO_ERROR, failureStatusToSend.SetValue(Status::InvalidAction));

        // If SendCommandResponse() fails, we are responsible for closing the exchange,
        // as stipulated by the API contract. We fulfill this obligation by not sending
        // a message expecting a response on the exchange. Since we are in the middle
        // of processing an incoming message, the exchange will close itself once we are
        // done processing it, if there is no response to wait for at that point.
        err = SendCommandResponse();
        VerifyOrExit(err == CHIP_NO_ERROR, failureStatusToSend.SetValue(Status::Failure));

        bool moreToSend = !mChunks.IsNull();
        if (!moreToSend)
        {
            // We are sending the final message and do not anticipate any further responses. We are
            // calling ExitNow() to immediately execute Close() and subsequently return from this function.
            ExitNow();
        }
        return CHIP_NO_ERROR;
    }

    ChipLogDetail(DataManagement, "CommandResponseSender: Unexpected message type %d", aPayloadHeader.GetMessageType());
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    if (mState != State::AllInvokeResponsesSent)
    {
        failureStatusToSend.SetValue(Status::Failure);
        ExitNow();
    }
    StatusResponse::Send(Status::InvalidAction, mExchangeCtx.Get(), false /*aExpectResponse*/);
    return err;
exit:
    if (failureStatusToSend.HasValue())
    {
        StatusResponse::Send(failureStatusToSend.Value(), mExchangeCtx.Get(), false /*aExpectResponse*/);
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

CHIP_ERROR CommandResponseSender::StartSendingCommandResponses()
{
    VerifyOrReturnError(mState == State::ReadyForInvokeResponses, CHIP_ERROR_INCORRECT_STATE);
    // If SendCommandResponse() fails, we are obligated to close the exchange as per the API
    // contract. However, this method's contract also stipulates that in the event of our
    // failure, the caller bears the responsibility of closing the exchange.
    ReturnErrorOnFailure(SendCommandResponse());

    if (HasMoreToSend())
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
    VerifyOrReturnError(HasMoreToSend(), CHIP_ERROR_INCORRECT_STATE);
    if (mChunks.IsNull())
    {
        VerifyOrReturnError(mReportResponseDropped, CHIP_ERROR_INCORRECT_STATE);
        SendStatusResponse(Status::ResourceExhausted);
        mReportResponseDropped = false;
        return CHIP_NO_ERROR;
    }
    System::PacketBufferHandle commandResponsePayload = mChunks.PopHead();

    Messaging::SendFlags sendFlag = Messaging::SendMessageFlags::kNone;
    if (HasMoreToSend())
    {
        sendFlag = Messaging::SendMessageFlags::kExpectResponse;
        mExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }

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
