#pragma once

#include <app/StatusResponse.h>
#include <messaging/ExchangeHolder.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

class CommandHandler;

class CommandHandlerDispatcher : public Messaging::ExchangeDelegate
{
public:
    CommandHandlerDispatcher(CommandHandler * commandHandler) : mpCommandHandler(commandHandler), mExchangeCtx(*this) {}

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
     * Gets subject descriptor of the exchange
     *
     * WARNING: Caller is expected to call this only when it knows session
     * cannot have been evicted.
     */
    Access::SubjectDescriptor GetSubjectDescriptor() const
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();
    }

    bool ValidateExchangeHasSessionHandle()
    {
        return mExchangeCtx && mExchangeCtx->HasSessionHandle();
    }

    FabricIndex GetAccessingFabricIndex() const
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->GetFabricIndex();
    }

    void SetExchangeContext(Messaging::ExchangeContext * ec)
    {
        mExchangeCtx.Grab(ec);
    }

    void WillSendMessage()
    {
        mExchangeCtx->WillSendMessage();
    }

    bool IsGroupExchangeContext()
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->IsGroupExchangeContext();
    }

    bool HasExchangeContext()
    {
        return mExchangeCtx.Get() != nullptr;
    }

    GroupId GetGroupId()
    {
        VerifyOrDie(mExchangeCtx);
        return mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    }

    CHIP_ERROR SendCommandResponse();

    void SendStatusResponse(Protocols::InteractionModel::Status aStatus)
    {
        // TODO Should this move state to the end?
        StatusResponse::Send(aStatus, mExchangeCtx.Get(), /*aExpectResponse = */ false);
    }

    bool AwaitingResponse()
    {
        return mState == State::AwaitingResponse;
    }

    void AddPacketToSend(System::PacketBufferHandle && aPacket)
    {
        VerifyOrDie(mState == State::Idle);
        mChunks.AddToEnd(std::move(aPacket));
    }

private:
    friend class TestCommandInteraction;

    enum class State : uint8_t
    {
        Idle,                ///< Default state that the object starts out in, where no commands have been dispatched
        AwaitingResponse,    ///< Awaiting response from requester, after sending response.
        AllCommandsSent,
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    // A list of InvokeResponseMessages to be sent out by CommandHandler.
    System::PacketBufferHandle mChunks;

    CommandHandler * mpCommandHandler = nullptr;
    Messaging::ExchangeHolder mExchangeCtx;
    State mState = State::Idle;
};

} // namespace app
} // namespace chip
