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

/**
 *    @file
 *      This file implements the ExchangeContext class.
 *
 */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPKeyIds.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemTimer.h>

using namespace chip::Encoding;
using namespace chip::Inet;
using namespace chip::System;

namespace chip {
namespace Messaging {

static void DefaultOnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, Protocols::Id protocolId,
                                     uint8_t msgType, PacketBufferHandle && payload)
{
    ChipLogError(ExchangeManager, "Dropping unexpected message %08" PRIX32 ":%d %04" PRIX16 " MsgId:%08" PRIX32,
                 protocolId.ToFullyQualifiedSpecForm(), msgType, ec->GetExchangeId(), packetHeader.GetMessageId());
}

bool ExchangeContext::IsInitiator() const
{
    return mFlags.Has(Flags::kFlagInitiator);
}

bool ExchangeContext::IsResponseExpected() const
{
    return mFlags.Has(Flags::kFlagResponseExpected);
}

void ExchangeContext::SetResponseExpected(bool inResponseExpected)
{
    mFlags.Set(Flags::kFlagResponseExpected, inResponseExpected);
}

void ExchangeContext::SetResponseTimeout(Timeout timeout)
{
    mResponseTimeout = timeout;
}

CHIP_ERROR ExchangeContext::SendMessage(Protocols::Id protocolId, uint8_t msgType, PacketBufferHandle && msgBuf,
                                        const SendFlags & sendFlags)
{
    // If we were waiting for a message send, this is it.
    mFlags.Clear(Flags::kFlagWillSendMessage);

    CHIP_ERROR err                         = CHIP_NO_ERROR;
    Transport::PeerConnectionState * state = nullptr;

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INTERNAL);

    // Don't let method get called on a freed object.
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() > 0);

    // we hold the exchange context here in case the entity that
    // originally generated it tries to close it as a result of
    // an error arising below. at the end, we have to close it.
    Retain();

    bool reliableTransmissionRequested = true;

    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(mSecureSession);
    // If sending via UDP and NoAutoRequestAck send flag is not specificed, request reliable transmission.
    if (state != nullptr && state->GetPeerAddress().GetTransportType() != Transport::Type::kUdp)
    {
        reliableTransmissionRequested = false;
    }
    else
    {
        reliableTransmissionRequested = !sendFlags.Has(SendMessageFlags::kNoAutoRequestAck);
    }

    // If a response message is expected...
    if (sendFlags.Has(SendMessageFlags::kExpectResponse))
    {
        // Only one 'response expected' message can be outstanding at a time.
        VerifyOrExit(!IsResponseExpected(), err = CHIP_ERROR_INCORRECT_STATE);

        SetResponseExpected(true);

        // Arm the response timer if a timeout has been specified.
        if (mResponseTimeout > 0)
        {
            err = StartResponseTimer();
            SuccessOrExit(err);
        }
    }

    err = mDispatch->SendMessage(mSecureSession, mExchangeId, IsInitiator(), GetReliableMessageContext(),
                                 reliableTransmissionRequested, protocolId, msgType, std::move(msgBuf));

exit:
    if (err != CHIP_NO_ERROR && IsResponseExpected())
    {
        CancelResponseTimer();
        SetResponseExpected(false);
    }

    // Release the reference to the exchange context acquired above. Under normal circumstances
    // this will merely decrement the reference count, without actually freeing the exchange context.
    // However if one of the function calls in this method resulted in a callback to the protocol,
    // the protocol may have released its reference, resulting in the exchange context actually
    // being freed here.
    Release();

    return err;
}

void ExchangeContext::DoClose(bool clearRetransTable)
{
    mFlags.Set(Flags::kFlagClosed);

    // Clear protocol callbacks
    if (mDelegate != nullptr)
    {
        mDelegate->OnExchangeClosing(this);
    }
    mDelegate = nullptr;

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

    // Cancel the response timer.
    CancelResponseTimer();
}

/**
 *  Gracefully close an exchange context. This call decrements the
 *  reference count and releases the exchange when the reference
 *  count goes to zero.
 *
 */
void ExchangeContext::Close()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() > 0);

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec id: %d [%04" PRIX16 "], %s", (this - mExchangeMgr->mContextPool.begin()), mExchangeId,
                  __func__);
#endif

    DoClose(false);
    Release();
}
/**
 *  Abort the Exchange context immediately and release all
 *  references to it.
 *
 */
void ExchangeContext::Abort()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() > 0);

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec id: %d [%04" PRIX16 "], %s", (this - mExchangeMgr->mContextPool.begin()), mExchangeId,
                  __func__);
#endif

    DoClose(true);
    Release();
}

void ExchangeContextDeletor::Release(ExchangeContext * ec)
{
    ec->mExchangeMgr->ReleaseContext(ec);
}

ExchangeContext::ExchangeContext(ExchangeManager * em, uint16_t ExchangeId, SecureSessionHandle session, bool Initiator,
                                 ExchangeDelegate * delegate)
{
    VerifyOrDie(mExchangeMgr == nullptr);

    mExchangeMgr   = em;
    mExchangeId    = ExchangeId;
    mSecureSession = session;
    mFlags.Set(Flags::kFlagInitiator, Initiator);
    mDelegate = delegate;

    ExchangeMessageDispatch * dispatch = nullptr;
    if (delegate != nullptr)
    {
        dispatch = delegate->GetMessageDispatch(em->GetReliableMessageMgr(), em->GetSessionMgr());
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
    SetMsgRcvdFromPeer(false);
    SetAutoRequestAck(true);

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec++ id: %d", ExchangeId);
#endif
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

ExchangeContext::~ExchangeContext()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() == 0);
    VerifyOrDie(!IsAckPending());

    // Ideally, in this scenario, the retransmit table should
    // be clear of any outstanding messages for this context and
    // the boolean parameter passed to DoClose() should not matter.

    DoClose(false);
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

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec-- id: %d", mExchangeId);
#endif
    SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

bool ExchangeContext::MatchExchange(SecureSessionHandle session, const PacketHeader & packetHeader,
                                    const PayloadHeader & payloadHeader)
{
    // A given message is part of a particular exchange if...
    return

        // The exchange identifier of the message matches the exchange identifier of the context.
        (mExchangeId == payloadHeader.GetExchangeID())

        // AND The message was received from the peer node associated with the exchange
        && (mSecureSession == session)

        // AND The message's source Node ID matches the peer Node ID associated with the exchange, or the peer Node ID of the
        // exchange is 'any'.
        && ((mSecureSession.GetPeerNodeId() == kAnyNodeId) ||
            (packetHeader.GetSourceNodeId().HasValue() && mSecureSession.GetPeerNodeId() == packetHeader.GetSourceNodeId().Value()))

        // AND The message was sent by an initiator and the exchange context is a responder (IsInitiator==false)
        //    OR The message was sent by a responder and the exchange context is an initiator (IsInitiator==true) (for the broadcast
        //    case, the initiator is ill defined)

        && (payloadHeader.IsInitiator() != IsInitiator());
}

void ExchangeContext::OnConnectionExpired()
{
    // Reset our mSecureSession to a default-initialized (hence not matching any
    // connection state) value, because it's still referencing the now-expired
    // connection.  This will mean that no more messages can be sent via this
    // exchange, which seems fine given the semantics of connection expiration.
    mSecureSession = SecureSessionHandle();

    if (!IsResponseExpected())
    {
        // Nothing to do in this case
        return;
    }

    // If we're waiting on a response, we now know it's never going to show up
    // and we should notify our delegate accordingly.
    CancelResponseTimer();
    SetResponseExpected(false);
    NotifyResponseTimeout();
}

CHIP_ERROR ExchangeContext::StartResponseTimer()
{
    System::Layer * lSystemLayer = mExchangeMgr->GetSessionMgr()->SystemLayer();
    if (lSystemLayer == nullptr)
    {
        // this is an assertion error, which shall never happen
        return CHIP_ERROR_INTERNAL;
    }

    return lSystemLayer->StartTimer(mResponseTimeout, HandleResponseTimeout, this);
}

void ExchangeContext::CancelResponseTimer()
{
    System::Layer * lSystemLayer = mExchangeMgr->GetSessionMgr()->SystemLayer();
    if (lSystemLayer == nullptr)
    {
        // this is an assertion error, which shall never happen
        return;
    }

    lSystemLayer->CancelTimer(HandleResponseTimeout, this);
}

void ExchangeContext::HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    ExchangeContext * ec = reinterpret_cast<ExchangeContext *>(aAppState);

    if (ec == nullptr)
        return;

    // NOTE: we don't set mResponseExpected to false here because the response could still arrive. If the user
    // wants to never receive the response, they must close the exchange context.

    ec->NotifyResponseTimeout();
}

void ExchangeContext::NotifyResponseTimeout()
{
    ExchangeDelegate * delegate = GetDelegate();

    // Call the user's timeout handler.
    if (delegate != nullptr)
    {
        delegate->OnResponseTimeout(this);
    }
}

CHIP_ERROR ExchangeContext::HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                          const Transport::PeerAddress & peerAddress, MessageFlags msgFlags,
                                          PacketBufferHandle && msgBuf)
{
    // We hold a reference to the ExchangeContext here to
    // guard against Close() calls(decrementing the reference
    // count) by the protocol before the CHIP Exchange
    // layer has completed its work on the ExchangeContext.
    Retain();

    // Keep track of whether we're nested under an outer HandleMessage
    // invocation.
    bool alreadyHandlingMessage = mFlags.Has(Flags::kFlagHandlingMessage);
    mFlags.Set(Flags::kFlagHandlingMessage);

    bool isStandaloneAck = payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck);
    bool isDuplicate     = msgFlags.Has(MessageFlagValues::kDuplicateMessage);

    CHIP_ERROR err = mDispatch->OnMessageReceived(payloadHeader, packetHeader.GetMessageId(), peerAddress, msgFlags,
                                                  GetReliableMessageContext());
    SuccessOrExit(err);

    if (IsAckPending() && !mDelegate)
    {
        // The incoming message wants an ack, but we have no delegate, so
        // there's not going to be a response to piggyback on.  Just flush the
        // ack out right now.
        err = FlushAcks();
        SuccessOrExit(err);
    }

    // The SecureChannel::StandaloneAck message type is only used for MRP; do not pass such messages to the application layer.
    if (isStandaloneAck)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    // Since the message is duplicate, let's not forward it up the stack
    if (isDuplicate)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    // Since we got the response, cancel the response timer.
    CancelResponseTimer();

    // If the context was expecting a response to a previously sent message, this message
    // is implicitly that response.
    SetResponseExpected(false);

    if (mDelegate != nullptr)
    {
        err = mDelegate->OnMessageReceived(this, packetHeader, payloadHeader, std::move(msgBuf));
    }
    else
    {
        DefaultOnMessageReceived(this, packetHeader, payloadHeader.GetProtocolID(), payloadHeader.GetMessageType(),
                                 std::move(msgBuf));
    }

exit:
    // Don't close ourselves if we're already closed.
    //
    // Don't close ourselves if this message is a standalone ack. We're still
    // not closed and getting an ack should not affect that.  In particular,
    // since the standalone ack was not passed to the delegate, the delegate
    // never got a chance to say "stay open". The one exception here is if
    // mDelegate is null: in that case this is an unsolicited message and we
    // were just created to ack it and close after that.
    //
    // Don't close for duplicates for similar reasons, with the same exception.
    //
    // Also don't close if there's an outer HandleMessage invocation.  It'll
    // deal with the closing.
    if (!mFlags.Has(Flags::kFlagClosed) && !mFlags.Has(Flags::kFlagWillSendMessage) && !IsResponseExpected() &&
        (!isStandaloneAck || (mDelegate == nullptr)) && (!isDuplicate || (mDelegate == nullptr)) && !alreadyHandlingMessage)
    {
        Close();
    }

    if (!alreadyHandlingMessage)
    {
        // We are the outermost HandleMessage invocation.  We're not handling a
        // message anymore.
        mFlags.Clear(Flags::kFlagHandlingMessage);
    }

    // Release the reference to the ExchangeContext that was held at the beginning of this function.
    // This call should also do the needful of closing the ExchangeContext if the protocol has
    // already made a prior call to Close().
    Release();

    return err;
}

} // namespace Messaging
} // namespace chip
