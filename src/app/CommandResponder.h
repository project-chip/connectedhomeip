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

#include <app/CommandHandler.h>
#include <app/CommandResponderInterface.h>
#include <app/StatusResponse.h>
#include <messaging/ExchangeHolder.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

/**
 * Manages the process of sending InvokeResponseMessage(s) to the requester.
 *
 * Implements the CommandResponderInterface. Uses a CommandHandler member to process
 * InvokeRequestCommand. The CommandHandler is provided a reference to this
 * CommandResponderInterface implementation to enable sending InvokeResponseMessage(s).
 */
class CommandResponder : public Messaging::ExchangeDelegate, public CommandHandler::Callback, public CommandResponderInterface
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;
        /*
         * Signals registered callback that this object has finished its work and can now be
         * safely destroyed/released.
         */
        virtual void OnDone(CommandResponder & apResponderObj) = 0;
    };

    CommandResponder(Callback * apCallback, CommandHandler::Callback * apDispatchCallback) :
        mpCallback(apCallback), mpCommandHandlerCallback(apDispatchCallback), mCommandHandler(this), mExchangeCtx(*this)
    {}

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    void OnDone(CommandHandler & apCommandObj) override
    {
        if (mDelayCallingCloseUntilOnDone)
        {
            // We have already sent a message to the client indicating that we are not expecting
            // a response.
            Close();
            return;
        }
        StartSendingCommandResponses();
    }

    void DispatchCommand(CommandHandler & apCommandObj, const ConcreteCommandPath & aCommandPath,
                         TLV::TLVReader & apPayload) override
    {
        VerifyOrReturn(mpCommandHandlerCallback);
        mpCommandHandlerCallback->DispatchCommand(apCommandObj, aCommandPath, apPayload);
    }

    Protocols::InteractionModel::Status CommandExists(const ConcreteCommandPath & aCommandPath) override
    {
        VerifyOrReturnValue(mpCommandHandlerCallback, Protocols::InteractionModel::Status::UnsupportedCommand);
        return mpCommandHandlerCallback->CommandExists(aCommandPath);
    }

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
    Messaging::ExchangeContext * GetExchangeContext() const override { return mExchangeCtx.Get(); }

    /**
     * Gets subject descriptor of the exchange.
     *
     * WARNING: This method should only be called when the caller is certain the
     * session has not been evicted.
     */
    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();
    }

    FabricIndex GetAccessingFabricIndex() const override
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetFabricIndex();
    }

    bool IsForGroup() const override
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->IsGroupExchangeContext();
    }

    GroupId GetGroupId() const override
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    }

    /**
     * @brief Flush acks right now, typically done when processing a slow command
     */
    void FlushAcksRightNow() override
    {
        VerifyOrReturn(mExchangeCtx);
        auto * msgContext = mExchangeCtx->GetReliableMessageContext();
        VerifyOrReturn(msgContext != nullptr);
        msgContext->FlushAcks();
    }

    void AddInvokeResponseToSend(System::PacketBufferHandle && aPacket) override
    {
        VerifyOrDie(mState == State::ReadyForInvokeResponses);
        mChunks.AddToEnd(std::move(aPacket));
    }

    /**
     * @brief Called to indicate that response was dropped.
     */
    void ResponseDropped() override { mReportResponseDropped = true; }

    /*
     * Main entrypoint for this class to handle an invoke request.
     *
     * isTimedInvoke is true if and only if this is part of a Timed Invoke
     * transaction (i.e. was preceded by a Timed Request).  If we reach here,
     * the timer verification has already been done.
     */
    void OnInvokeCommandRequest(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload, bool isTimedInvoke);

#if CHIP_WITH_NLFAULTINJECTION
    /**
     * @brief Sends InvokeResponseMessages with injected faults for certification testing.
     *
     * The Test Harness (TH) uses this to simulate various server response behaviors,
     * ensuring the Device Under Test (DUT) handles responses per specification.
     *
     * This function strictly validates the DUT's InvokeRequestMessage against the test plan.
     * If deviations occur, the TH terminates with a detailed error message.
     *
     * @param ec Exchange context for sending InvokeResponseMessages to the client.
     * @param payload Payload of the incoming InvokeRequestMessage from the client.
     * @param isTimedInvoke Indicates whether the interaction is timed.
     * @param faultType The specific type of fault to inject into the response.
     */
    void TestOnlyInvokeCommandRequestWithFaultsInjected(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload,
                                                        bool isTimedInvoke, CommandHandler::NlFaultInjectionType faultType);
#endif // CHIP_WITH_NLFAULTINJECTION

private:
    enum class State : uint8_t
    {
        ReadyForInvokeResponses, ///< Accepting InvokeResponses to send back to requester.
        AwaitingStatusResponse,  ///< Awaiting status response from requester, after sending InvokeResponse.
        AllInvokeResponsesSent,  ///< All InvokeResponses have been sent out.
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    /**
     * @brief Initiates the sending of InvokeResponses previously queued using AddInvokeResponseToSend.
     */
    void StartSendingCommandResponses();

    void SendStatusResponse(Protocols::InteractionModel::Status aStatus)
    {
        StatusResponse::Send(aStatus, mExchangeCtx.Get(), /*aExpectResponse = */ false);
    }

    CHIP_ERROR SendCommandResponse();
    bool HasMoreToSend() { return !mChunks.IsNull() || mReportResponseDropped; }
    void Close();

    // A list of InvokeResponseMessages to be sent out by CommandResponder.
    System::PacketBufferHandle mChunks;

    Callback * mpCallback;
    CommandHandler::Callback * mpCommandHandlerCallback;
    CommandHandler mCommandHandler;
    Messaging::ExchangeHolder mExchangeCtx;
    State mState = State::ReadyForInvokeResponses;

    bool mDelayCallingCloseUntilOnDone = false;
    bool mReportResponseDropped        = false;
};

} // namespace app
} // namespace chip
