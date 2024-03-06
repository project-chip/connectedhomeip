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

#include "CommandResponder.h"
#include "InteractionModelEngine.h"
#include "messaging/ExchangeContext.h"

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

CHIP_ERROR CommandResponder::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                               System::PacketBufferHandle && aPayload)
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

        err = SendCommandResponse();
        // If SendCommandResponse() fails, we must still close the exchange. Since sending an
        // InvokeResponseMessage seems problematic, we'll send a StatusResponse with 'Failure'
        // instead.
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

    ChipLogDetail(DataManagement, "CommandResponder: Unexpected message type %d", aPayloadHeader.GetMessageType());
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

void CommandResponder::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogDetail(DataManagement, "CommandResponder: Timed out waiting for response from requester mState=[%10.10s]",
                  GetStateStr());
    // We don't need to consider remaining async CommandHandlers here. We become the exchange
    // delegate only after CommandHandler::OnDone is called.
    Close();
}

void CommandResponder::StartSendingCommandResponses()
{
    VerifyOrDie(mState == State::ReadyForInvokeResponses);
    CHIP_ERROR err = SendCommandResponse();
    if (err != CHIP_NO_ERROR)
    {
        // TODO(#30453): It should be our responsibility to send a Failure StatusResponse to the requestor
        // if there is a SessionHandle, but legacy unit tests explicitly check the behavior where
        // we do not send any message. Changing this behavior should be done in a standalone
        // PR where only that specific change is made. Here is a possible solution that should
        // be done that fulfills our responsibility to send a Failure StatusResponse, but this causes unit
        // tests to start failing.
        //   ```
        //   if (HasSessionHandle())
        //   {
        //       SendStatusResponse(Status::Failure);
        //   }
        //   Close();
        //   return;
        //   ```
    }

    if (HasMoreToSend())
    {
        MoveToState(State::AwaitingStatusResponse);
        mExchangeCtx->SetDelegate(this);
    }
    else
    {
        Close();
    }
}

CHIP_ERROR CommandResponder::SendCommandResponse()
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

const char * CommandResponder::GetStateStr() const
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

void CommandResponder::MoveToState(const State aTargetState)
{
    if (mState == aTargetState)
    {
        return;
    }
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Command response sender moving to [%10.10s]", GetStateStr());
}

void CommandResponder::Close()
{
    MoveToState(State::AllInvokeResponsesSent);
    mpCallback->OnDone(*this);
}

void CommandResponder::OnInvokeCommandRequest(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload,
                                              bool isTimedInvoke)
{
    VerifyOrDieWithMsg(ec != nullptr, DataManagement, "Incoming exchange context should not be null");
    VerifyOrDieWithMsg(mState == State::ReadyForInvokeResponses, DataManagement, "state should be ReadyForInvokeResponses");

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.
    mExchangeCtx.Grab(ec);
    mExchangeCtx->WillSendMessage();

    // Grabbing Handle to prevent OnDone from being called before OnInvokeCommandRequest returns.
    // This allows us to send a StatusResponse error instead of the queued up InvokeResponseMessages.
    CommandHandler::Handle workHandle(&mCommandHandler);
    Status status = mCommandHandler.OnInvokeCommandRequest(*this, std::move(payload), isTimedInvoke);
    if (status != Status::Success)
    {
        VerifyOrDie(mState == State::ReadyForInvokeResponses);
        SendStatusResponse(status);
        // We can't safely call Close() yet, even though we won't be sending more data on this exchange.
        // It must be deferred until OnDone is called.
        mDelayCallingCloseUntilOnDone = true;
    }
}

#if CHIP_WITH_NLFAULTINJECTION

void CommandResponder::TestOnlyInvokeCommandRequestWithFaultsInjected(Messaging::ExchangeContext * ec,
                                                                      System::PacketBufferHandle && payload, bool isTimedInvoke,
                                                                      CommandHandler::NlFaultInjectionType faultType)
{
    VerifyOrDieWithMsg(ec != nullptr, DataManagement, "TH Failure: Incoming exchange context should not be null");
    VerifyOrDieWithMsg(mState == State::ReadyForInvokeResponses, DataManagement,
                       "TH Failure: state should be ReadyForInvokeResponses, issue with TH");

    mExchangeCtx.Grab(ec);
    mExchangeCtx->WillSendMessage();

    mCommandHandler.TestOnlyInvokeCommandRequestWithFaultsInjected(*this, std::move(payload), isTimedInvoke, faultType);
}
#endif // CHIP_WITH_NLFAULTINJECTION

} // namespace app
} // namespace chip
