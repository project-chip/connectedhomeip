#include "CommandHandlerDispatcher.h"
#include "InteractionModelEngine.h"
#include "messaging/ExchangeContext.h"

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

CHIP_ERROR CommandHandlerDispatcher::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    bool sendStatusResponseWithInvalidAction = false;

    if (mState == State::AwaitingResponse && aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
    {
        CHIP_ERROR statusError = CHIP_NO_ERROR;
        err                    = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError);
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithInvalidAction = true);
        statusError = err;
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithInvalidAction = true);

        err = SendCommandResponse();
        VerifyOrExit(err == CHIP_NO_ERROR, sendStatusResponseWithInvalidAction = true);

        if (mState != State::AwaitingResponse)
        {
            // We are sending out the last message and no longer are expecting any responses. As a results we are
            // fulfilling out responsibility to call close by calling ExitNow().
            ExitNow();
        }
        return CHIP_NO_ERROR;
    }
    ChipLogDetail(DataManagement, "CommandHandler: Unexpected message type %d", aPayloadHeader.GetMessageType());
    StatusResponse::Send(Status::InvalidAction, mExchangeCtx.Get(), false /*aExpectResponse*/);
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    if (mState == State::AwaitingResponse)
    {
        // We were waiting on a response and we got something we did not expect, so we will no longer continue sending
        // messages, nor do we expect to get any more responses. As a result we are simply fulfilling our
        // responsibility to call Close() by calling ExitNow().
        ExitNow();
    }
    return err;
exit:
    if (sendStatusResponseWithInvalidAction)
    {
        StatusResponse::Send(Status::InvalidAction, mExchangeCtx.Get(), false /*aExpectResponse*/);
    }
    mpCommandHandler->Close();
    return err;
}

void CommandHandlerDispatcher::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogDetail(DataManagement, "CommandHandlerDispatcher: Timed out waiting for response from requester mState=[%10.10s]",
                  GetStateStr());
    mpCommandHandler->Close();
}

CHIP_ERROR CommandHandlerDispatcher::SendCommandResponse()
{
    VerifyOrReturnError(!mChunks.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    System::PacketBufferHandle commandResponsePayload = mChunks.PopHead();

    bool moreToSend               = !mChunks.IsNull();
    Messaging::SendFlags sendFlag = Messaging::SendMessageFlags::kNone;
    if (moreToSend)
    {
        sendFlag = Messaging::SendMessageFlags::kExpectResponse;
    }

    bool sendingFirstResponse = mState == State::Idle;

    // If this is the first response we are sending, and we are waiting on response to send out another
    // InvokeResponseMessages we setup the response timer.
    if (sendingFirstResponse && moreToSend)
    {
        mExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    }

    ReturnErrorOnFailure(mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse,
                                                   std::move(commandResponsePayload), sendFlag));
    if (moreToSend)
    {
        MoveToState(State::AwaitingResponse);
        if (sendingFirstResponse)
        {
            mExchangeCtx->SetDelegate(this);
        }
    }
    else
    {
        MoveToState(State::AllCommandsSent);
    }
    return CHIP_NO_ERROR;
}

const char * CommandHandlerDispatcher::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::AwaitingResponse:
        return "AwaitingResponse";

    case State::AllCommandsSent:
        return "AllCommandsSent";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void CommandHandlerDispatcher::MoveToState(const State aTargetState)
{
    if (mState == aTargetState)
    {
        return;
    }
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Command handler dispatcher moving to [%10.10s]", GetStateStr());
}

} // namespace app
} // namespace chip
