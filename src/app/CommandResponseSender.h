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

#include <app/CommandHandlerExchangeInterface.h>
#include <app/CommandHandlerImpl.h>
#include <app/StatusResponse.h>
#include <messaging/ExchangeHolder.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

// TODO(#30453): Rename CommandResponseSender to CommandResponder in follow up PR
/**
 * Manages the process of sending InvokeResponseMessage(s) to the requester.
 *
 * Implements the CommandHandlerExchangeInterface. Uses a CommandHandler member to process
 * InvokeCommandRequest. The CommandHandler is provided a reference to this
 * CommandHandlerExchangeInterface implementation to enable sending InvokeResponseMessage(s).
 */
class CommandResponseSender : public Messaging::ExchangeDelegate,
                              public CommandHandlerImpl::Callback,
                              public CommandHandlerExchangeInterface
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
        virtual void OnDone(CommandResponseSender & apResponderObj) = 0;
    };

    CommandResponseSender(Callback * apCallback, CommandHandlerImpl::Callback * apDispatchCallback) :
        mpCallback(apCallback), mpCommandHandlerCallback(apDispatchCallback), mCommandHandler(this), mExchangeCtx(*this)
    {}

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    void OnDone(CommandHandlerImpl & apCommandObj) override;

    void DispatchCommand(CommandHandlerImpl & apCommandObj, const ConcreteCommandPath & aCommandPath,
                         TLV::TLVReader & apPayload) override;

    Protocols::InteractionModel::Status CommandExists(const ConcreteCommandPath & aCommandPath) override;

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

    Optional<GroupId> GetGroupId() const override
    {
        VerifyOrDie(mExchangeCtx);
        auto sessionHandle = mExchangeCtx->GetSessionHandle();
        if (sessionHandle->GetSessionType() != Transport::Session::SessionType::kGroupIncoming)
        {
            return NullOptional;
        }
        return MakeOptional(sessionHandle->AsIncomingGroupSession()->GetGroupId());
    }

    void HandlingSlowCommand() override
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

    void ResponseDropped() override { mReportResponseDropped = true; }

    size_t GetCommandResponseMaxBufferSize() override;

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
                                                        bool isTimedInvoke, CommandHandlerImpl::NlFaultInjectionType faultType);
#endif // CHIP_WITH_NLFAULTINJECTION

private:
    enum class State : uint8_t
    {
        ReadyForInvokeResponses,       ///< Accepting InvokeResponses to send back to requester.
        AwaitingStatusResponse,        ///< Awaiting status response from requester, after sending InvokeResponse.
        AllInvokeResponsesSent,        ///< All InvokeResponses have been sent out.
        ErrorSentDelayCloseUntilOnDone ///< We have sent an early error response, but still need to clean up.
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

    // A list of InvokeResponseMessages to be sent out by CommandResponseSender.
    System::PacketBufferHandle mChunks;

    Callback * mpCallback;
    CommandHandlerImpl::Callback * mpCommandHandlerCallback;
    CommandHandlerImpl mCommandHandler;
    Messaging::ExchangeHolder mExchangeCtx;
    State mState = State::ReadyForInvokeResponses;

    bool mReportResponseDropped = false;
};

} // namespace app
} // namespace chip
