/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>

using namespace chip::Encoding;
using namespace chip::Inet;
using namespace chip::System;

namespace chip {
namespace Messaging {

static void DefaultOnMessageReceived(ExchangeContext * ec, Protocols::Id protocolId, uint8_t msgType, uint32_t messageCounter,
                                     PacketBufferHandle && payload)
{
    ChipLogError(ExchangeManager,
                 "Dropping unexpected message of type " ChipLogFormatMessageType " with protocolId " ChipLogFormatProtocolId
                 " and MessageCounter:" ChipLogFormatMessageCounter " on exchange " ChipLogFormatExchange,
                 msgType, ChipLogValueProtocolId(protocolId), messageCounter, ChipLogValueExchange(ec));
}

bool ExchangeContext::IsInitiator() const
{
    return mFlags.Has(Flags::kFlagInitiator);
}

void ExchangeContext::SetResponseTimeout(Timeout timeout)
{
    mResponseTimeout = timeout;
}

void ExchangeContext::WillSendMessage()
{
    VerifyOrDie(mState == State::Initiator || mState == State::Active || mState == State::Background);
    if (mState == State::Active)
    {
        Retain(); // Life-cycle Upper-layer case
        mState = State::Background;
    }
}

CHIP_ERROR ExchangeContext::WillHandleMoreMessage(Timeout idleTimeout)
{
    VerifyOrDie(mState == State::SentNoExpectResponse);
    mState = State::Sleep;
    ReturnErrorOnFailure(StartTimer(idleTimeout));
    return CHIP_NO_ERROR;
}

bool ExchangeContext::CanHandleMessage()
{
    return mState == State::Responder || mState == State::SentExpectResponse || mState == State::SentNoExpectResponse ||
        mState == State::Sleep;
}

CHIP_ERROR ExchangeContext::SendMessage(Protocols::Id protocolId, uint8_t msgType, PacketBufferHandle && msgBuf,
                                        const SendFlags & sendFlags)
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    bool isStandaloneAck =
        (protocolId == Protocols::SecureChannel::Id) && msgType == to_underlying(Protocols::SecureChannel::MsgType::StandaloneAck);

    VerifyOrReturnError(isStandaloneAck || mState == State::Initiator || mState == State::Active || mState == State::Background,
                        CHIP_ERROR_INCORRECT_STATE);

    bool isResponseExpected = sendFlags.Has(SendMessageFlags::kExpectResponse);
    if (isResponseExpected)
    {
        VerifyOrDie(mResponseTimeout > 0);
        ReturnErrorOnFailure(StartTimer(mResponseTimeout));
    }

    State originalState = mState;
    if (mState == State::Initiator || mState == State::Background)
    {
        Release(); // Life-cycle Upper-layer case
        mState = State::Active;
    }

    // If sending via UDP and NoAutoRequestAck send flag is not specificed,
    // request reliable transmission.
    const Transport::PeerAddress * peerAddress = GetSecureSession().GetPeerAddress(mExchangeMgr->GetSessionManager());
    // Treat unknown peer address as "not UDP", because we have no idea whether
    // it's safe to do MRP there.
    bool isUDPTransport                = peerAddress && peerAddress->GetTransportType() == Transport::Type::kUdp;
    bool reliableTransmissionRequested = isUDPTransport && !sendFlags.Has(SendMessageFlags::kNoAutoRequestAck);

    CHIP_ERROR err = mDispatch->SendMessage(mSecureSession.Value(), mExchangeId, IsInitiator(), GetReliableMessageContext(),
                                            reliableTransmissionRequested, protocolId, msgType, std::move(msgBuf));
    if (err == CHIP_NO_ERROR)
    {
        mState = isResponseExpected ? State::SentExpectResponse : State::SentNoExpectResponse;
    }
    else
    {
        if (err != CHIP_NO_ERROR && isResponseExpected)
        {
            CancelTimer();
        }

        // Rollback to originalState when send error
        if (originalState == State::Initiator || originalState == State::Background)
        {
            Retain();
            mState = originalState;
        }
    }

    return err;
}

void ExchangeContext::CleanUp(State originalState)
{
    if (mState == State::Initiator || mState == State::Background)
    {
        Release(); // Life-cycle Upper-layer case
    }
    else if (mState == State::SentExpectResponse)
    {
        ExchangeDelegate * delegate = GetDelegate();
        if (delegate != nullptr)
        {
            delegate->OnResponseTimeout(this);
        }
    }
    else if (mState == State::SentNoExpectResponse || mState == State::Sleep)
    {
        // TODO: Add idle timeout callback.
    }
}

void ExchangeContext::DoClose(State originalState, bool clearRetransTable)
{
    // Closure of an exchange context is based on ref counting. The Protocol, when it calls DoClose(), indicates that
    // it is done with the exchange context and the message layer sets all callbacks to NULL and does not send anything
    // received on the exchange context up to higher layers.  At this point, the message layer needs to handle the
    // remaining work to be done on that exchange, (e.g. send all pending acks) before truly cleaning it up.
    FlushAcks();

    // In case the protocol wants a harder release of the EC right away, such as calling Abort(), exchange
    // needs to clear the MRP retransmission table immediately.
    if (clearRetransTable)
    {
        mExchangeMgr->GetReliableMessageMgr()->ClearRetransTable(static_cast<ReliableMessageContext *>(this));
    }

    CleanUp(originalState);

    // Clear protocol callbacks
    if (mDelegate != nullptr)
    {
        mDelegate->OnExchangeClosing(this);
    }
    mDelegate = nullptr;
}

/**
 *  Gracefully close an exchange context. This call decrements the
 *  reference count and releases the exchange when the reference
 *  count goes to zero.
 *
 */
void ExchangeContext::Close()
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    if (mState == State::Closed || mState == State::Error)
    {
        return;
    }

    State originalState = mState;
    mState              = State::Closed;
    DoClose(originalState, false);
}

/**
 *  Abort the Exchange context immediately and release all
 *  references to it.
 *
 */
void ExchangeContext::Abort()
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    if (mState == State::Closed || mState == State::Error)
    {
        return;
    }

    State originalState = mState;
    mState              = State::Error;
    DoClose(originalState, true);
}

void ExchangeContextDeletor::Release(ExchangeContext * ec)
{
    ec->mExchangeMgr->ReleaseContext(ec);
}

ExchangeContext::ExchangeContext(ExchangeManager * em, uint16_t ExchangeId, SessionHandle session, bool Initiator,
                                 ExchangeDelegate * delegate)
{
    VerifyOrDie(mExchangeMgr == nullptr);

    mExchangeMgr = em;
    mExchangeId  = ExchangeId;
    mSecureSession.SetValue(session);
    mFlags.Set(Flags::kFlagInitiator, Initiator);
    if (Initiator)
    {
        mState = State::Initiator;
    }
    else
    {
        mState = State::Responder;
    }
    mDelegate = delegate;

    ExchangeMessageDispatch * dispatch = nullptr;
    if (delegate != nullptr)
    {
        dispatch = delegate->GetMessageDispatch(em->GetReliableMessageMgr(), em->GetSessionManager());
        if (dispatch == nullptr)
        {
            dispatch = &em->mDefaultExchangeDispatch;
        }
    }
    else
    {
        dispatch = &em->mDefaultExchangeDispatch;
    }
    VerifyOrDie(dispatch != nullptr);
    mDispatch = dispatch->Retain();

    SetDropAckDebug(false);
    SetAckPending(false);

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

ExchangeContext::~ExchangeContext()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() == 0);
    VerifyOrDie(!IsAckPending());

    // Ideally, in this scenario, the retransmit table should
    // be clear of any outstanding messages for this context and
    // the boolean parameter passed to DoClose() should not matter.

    State originalState = mState;
    mState              = State::Released;
    DoClose(originalState, false);
    mExchangeMgr = nullptr;

    if (mExchangeACL != nullptr)
    {
        chip::Platform::Delete(mExchangeACL);
        mExchangeACL = nullptr;
    }

    if (mDispatch != nullptr)
    {
        mDispatch->Release();
        mDispatch = nullptr;
    }

    SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

bool ExchangeContext::MatchExchange(SessionHandle session, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader)
{
    // A given message is part of a particular exchange if...
    return

        // The exchange identifier of the message matches the exchange identifier of the context.
        (mExchangeId == payloadHeader.GetExchangeID())

        // AND The Session ID associated with the incoming message matches the Session ID associated with the exchange.
        && (mSecureSession.HasValue() && mSecureSession.Value().MatchIncomingSession(session))

        // TODO: This check should be already implied by the equality of session check,
        // It should be removed after we have implemented the temporary node id for PASE and CASE sessions
        && (IsEncryptionRequired() == packetHeader.GetFlags().Has(Header::FlagValues::kEncryptedMessage))

        // AND The message was sent by an initiator and the exchange context is a responder (IsInitiator==false)
        //    OR The message was sent by a responder and the exchange context is an initiator (IsInitiator==true) (for the broadcast
        //    case, the initiator is ill defined)

        && (payloadHeader.IsInitiator() != IsInitiator());
}

void ExchangeContext::OnConnectionExpired()
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    if (mState == State::Closed || mState == State::Error)
    {
        return;
    }

    // Reset our mSecureSession to a default-initialized (hence not matching any
    // connection state) value, because it's still referencing the now-expired
    // connection.  This will mean that no more messages can be sent via this
    // exchange, which seems fine given the semantics of connection expiration.
    mSecureSession.ClearValue();

    State originalState = mState;
    mState              = State::Error;
    CleanUp(originalState);
}

CHIP_ERROR ExchangeContext::StartTimer(Timeout timeout)
{
    VerifyOrReturnError(!mFlags.Has(Flags::kTimerScheduled), CHIP_ERROR_INCORRECT_STATE);

    System::Layer * lSystemLayer = mExchangeMgr->GetSessionManager()->SystemLayer();
    CHIP_ERROR err               = lSystemLayer->StartTimer(timeout, HandleTimeout, this);
    if (err == CHIP_NO_ERROR)
    {
        mFlags.Set(Flags::kTimerScheduled, true);
        Retain(); // Life-cycle Timer case
    }
    return err;
}

void ExchangeContext::CancelTimer()
{
    if (!mFlags.Has(Flags::kTimerScheduled))
    {
        return;
    }

    System::Layer * lSystemLayer = mExchangeMgr->GetSessionManager()->SystemLayer();
    lSystemLayer->CancelTimer(HandleTimeout, this);
    mFlags.Set(Flags::kTimerScheduled, false);
    Release(); // Life-cycle Timer case
}

void ExchangeContext::HandleTimeout(System::Layer * aSystemLayer, void * aAppState)
{
    reinterpret_cast<ExchangeContext *>(aAppState)->NotifyTimeout();
}

void ExchangeContext::NotifyTimeout()
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    if (!mFlags.Has(Flags::kTimerScheduled))
    {
        return;
    }
    mFlags.Set(Flags::kTimerScheduled, false);
    Release(); // Life-cycle Timer case

    if (mState == State::SentExpectResponse || mState == State::Sleep)
    {
        State originalState = mState;
        mState              = State::Error;
        DoClose(originalState, true);
    }
}

CHIP_ERROR ExchangeContext::HandleMessage(uint32_t messageCounter, const PayloadHeader & payloadHeader,
                                          const Transport::PeerAddress & peerAddress, MessageFlags msgFlags,
                                          PacketBufferHandle && msgBuf)
{
    ExchangeHandle ref(*this); // Life-cycle Handler case

    bool isStandaloneAck = payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck);
    bool isDuplicate     = msgFlags.Has(MessageFlagValues::kDuplicateMessage);

    ReturnErrorOnFailure(
        mDispatch->OnMessageReceived(messageCounter, payloadHeader, peerAddress, msgFlags, GetReliableMessageContext()));

    if (IsAckPending() && !mDelegate)
    {
        // The incoming message wants an ack, but we have no delegate, so
        // there's not going to be a response to piggyback on.  Just flush the
        // ack out right now.
        ReturnErrorOnFailure(FlushAcks());
    }

    // The SecureChannel::StandaloneAck message type is only used for MRP; do not pass such messages to the application layer.
    if (isStandaloneAck)
    {
        return CHIP_NO_ERROR;
    }

    // Since the message is duplicate, let's not forward it up the stack
    if (isDuplicate)
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(!rc->IsMessageNotAcked(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(CanHandleMessage(), CHIP_ERROR_INCORRECT_STATE);

    if (mState == State::Responder)
    {
        Release(); // Life-cycle Upper-layer case, this releases the initial count.
    }
    else
    {
        // State::SentExpectResponse cancels the respond timer
        // State::Sleep cancels the idle timer
        CancelTimer();
    }

    mState = State::Active;
    if (mDelegate != nullptr)
    {
        return mDelegate->OnMessageReceived(this, payloadHeader, std::move(msgBuf));
    }
    else
    {
        DefaultOnMessageReceived(this, payloadHeader.GetProtocolID(), payloadHeader.GetMessageType(), messageCounter,
                                 std::move(msgBuf));
        return CHIP_NO_ERROR;
    }

    // If WillSendMessage is not called inside mDelegate->OnMessageReceived, the scoped ExchangeHandle should be the
    // last reference to this exchange, the exchange will be release when exiting this function.
}

} // namespace Messaging
} // namespace chip
