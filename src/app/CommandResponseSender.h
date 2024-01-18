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

#pragma once

#include <app/StatusResponse.h>
#include <messaging/ExchangeHolder.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

typedef void (*OnResponseSenderDone)(void * context);
class CommandHandler;

/**
 * Class manages the process of sending `InvokeResponseMessage`(s) back to the initial requester.
 */
class CommandResponseSender : public Messaging::ExchangeDelegate
{
public:
    CommandResponseSender() : mExchangeCtx(*this) {}

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    /**
     * Gets the inner exchange context object, without ownership.
     *
     * WARNING: This is dangerous, since it is directly interacting with the
     *          exchange being managed automatically by mExchangeCtx and
     *          if not done carefully, may end up with use-after-free errors.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    Messaging::ExchangeContext * GetExchangeContext() const { return mExchangeCtx.Get(); }

    /**
     * @brief Flush acks right now, typically done when processing a slow command
     */
    void FlushAcksRightNow()
    {
        VerifyOrReturn(mExchangeCtx);
        auto * msgContext = mExchangeCtx->GetReliableMessageContext();
        VerifyOrReturn(msgContext != nullptr);
        msgContext->FlushAcks();
    }

    /**
     * Gets subject descriptor of the exchange.
     *
     * WARNING: This method should only be called when the caller is certain the
     * session has not been evicted.
     */
    Access::SubjectDescriptor GetSubjectDescriptor() const
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();
    }

    bool HasSessionHandle() { return mExchangeCtx && mExchangeCtx->HasSessionHandle(); }

    FabricIndex GetAccessingFabricIndex() const
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetFabricIndex();
    }

    void SetExchangeContext(Messaging::ExchangeContext * ec) { mExchangeCtx.Grab(ec); }

    void WillSendMessage() { mExchangeCtx->WillSendMessage(); }

    bool IsForGroup()
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->IsGroupExchangeContext();
    }

    bool HasExchangeContext() { return mExchangeCtx.Get() != nullptr; }

    GroupId GetGroupId()
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    }

    /**
     * @brief Initiates the sending of InvokeResponses previously queued using AddInvokeResponseToSend.
     *
     * Upon failure, the caller is responsible for closing the exchange appropriately, potentially
     * by calling `SendStatusResponse`.
     */
    CHIP_ERROR StartSendingCommandResponses();

    void SendStatusResponse(Protocols::InteractionModel::Status aStatus)
    {
        StatusResponse::Send(aStatus, mExchangeCtx.Get(), /*aExpectResponse = */ false);
    }

    bool AwaitingStatusResponse() { return mState == State::AwaitingStatusResponse; }

    void AddInvokeResponseToSend(System::PacketBufferHandle && aPacket)
    {
        VerifyOrDie(mState == State::ReadyForInvokeResponses);
        mChunks.AddToEnd(std::move(aPacket));
    }

    /**
     * @brief Sets final non-path specific StatusResponse error after sending all InvokeResponseMessages
     *
     * Intended to be used for sending a single, non-path specific, error after we send all
     * InvokeRequestMessages.
     *
     * @param aStatus InteractionModel Status. Cannot be success status.
     * @return CHIP_NO_ERROR Successfully set IM failure status to send as final StatusResponse.
     * @return CHIP_ERROR_INVALID_ARGUMENT provided with status that is success.
     * @return CHIP_ERROR_INCORRECT_STATE
     */
    CHIP_ERROR SetFinalStatusResponseFailure(Protocols::InteractionModel::Status aStatus)
    {
        VerifyOrReturnError(aStatus != Protocols::InteractionModel::Status::Success, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(mState != State::AllInvokeResponsesSent, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(!mChunks.IsNull(), CHIP_ERROR_INCORRECT_STATE);
        mFinalFailureStatus.SetValue(aStatus);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Registers a callback to be invoked when CommandResponseSender has finished sending responses.
     */
    void RegisterOnResponseSenderDoneCallback(Callback::Callback<OnResponseSenderDone> * aResponseSenderDoneCallback)
    {
        VerifyOrDie(!mCloseCalled);
        mResponseSenderDoneCallback = aResponseSenderDoneCallback;
    }

private:
    enum class State : uint8_t
    {
        ReadyForInvokeResponses, ///< Accepting InvokeResponses to send back to requester.
        AwaitingStatusResponse,  ///< Awaiting status response from requester, after sending InvokeResponse.
        AllInvokeResponsesSent,  ///< All InvokeResponses have been sent out.
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    CHIP_ERROR SendCommandResponse();
    bool HasMoreToSend() { return !mChunks.IsNull() || mFinalFailureStatus.HasValue(); }
    void Close();

    // A list of InvokeResponseMessages to be sent out by CommandResponseSender.
    System::PacketBufferHandle mChunks;

    // When final status is set, we will send out this status as the final message in the interaction.
    Optional<Protocols::InteractionModel::Status> mFinalFailureStatus;
    chip::Callback::Callback<OnResponseSenderDone> * mResponseSenderDoneCallback = nullptr;
    Messaging::ExchangeHolder mExchangeCtx;
    State mState = State::ReadyForInvokeResponses;

    bool mCloseCalled = false;
};

} // namespace app
} // namespace chip
