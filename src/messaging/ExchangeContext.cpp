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
                                     uint8_t msgType, PacketBufferHandle payload)
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

CHIP_ERROR ExchangeContext::SendMessage(Protocols::Id protocolId, uint8_t msgType, PacketBufferHandle msgBuf,
                                        const SendFlags & sendFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INTERNAL);

    // Don't let method get called on a freed object.
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() > 0);

    // we hold the exchange context here in case the entity that
    // originally generated it tries to close it as a result of
    // an error arising below. at the end, we have to close it.
    Retain();

    bool reliableTransmissionRequested = !sendFlags.Has(SendMessageFlags::kNoAutoRequestAck);

    ExchangeMessageDispatch * dispatch = GetMessageDispatch();
    ApplicationExchangeDispatch defaultDispatch;

    if (dispatch == nullptr)
    {
        defaultDispatch.Init(mExchangeMgr->GetReliableMessageMgr(), mExchangeMgr->GetSessionMgr());
        dispatch = &defaultDispatch;
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

    err = dispatch->SendMessage(mSecureSession, mExchangeId, IsInitiator(), GetReliableMessageContext(),
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
    // needs to clear the CRMP retransmission table immediately.
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
    ChipLogProgress(ExchangeManager, "ec id: %d [%04" PRIX16 "], %s", (this - mExchangeMgr->mContextPool.begin()), mExchangeId,
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
    ChipLogProgress(ExchangeManager, "ec id: %d [%04" PRIX16 "], %s", (this - mExchangeMgr->mContextPool.begin()), mExchangeId,
                    __func__);
#endif

    DoClose(true);
    Release();
}

void ExchangeContext::Reset()
{
    *this = ExchangeContext();
}

ExchangeMessageDispatch * ExchangeContext::GetMessageDispatch()
{
    if (mDelegate != nullptr)
    {
        return mDelegate->GetMessageDispatch(mExchangeMgr->GetReliableMessageMgr(), mExchangeMgr->GetSessionMgr());
    }

    return nullptr;
}

ExchangeContext * ExchangeContext::Alloc(ExchangeManager * em, uint16_t ExchangeId, SecureSessionHandle session, bool Initiator,
                                         ExchangeDelegateBase * delegate)
{
    VerifyOrDie(mExchangeMgr == nullptr && GetReferenceCount() == 0);

    Reset();
    Retain();
    mExchangeMgr = em;
    em->IncrementContextsInUse();
    mExchangeId    = ExchangeId;
    mSecureSession = session;
    mFlags.Set(Flags::kFlagInitiator, Initiator);
    mDelegate = delegate;

    SetDropAckDebug(false);
    SetAckPending(false);
    SetPeerRequestedAck(false);
    SetMsgRcvdFromPeer(false);
    SetAutoRequestAck(true);

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogProgress(ExchangeManager, "ec++ id: %d, inUse: %d, addr: 0x%x", (this - em->mContextPool.begin()),
                    em->GetContextsInUse(), this);
#endif
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumContexts);

    return this;
}

void ExchangeContext::Free()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() == 0);

    // Ideally, in this scenario, the retransmit table should
    // be clear of any outstanding messages for this context and
    // the boolean parameter passed to DoClose() should not matter.
    ExchangeManager * em = mExchangeMgr;

    DoClose(false);
    mExchangeMgr = nullptr;

    em->DecrementContextsInUse();

    if (mExchangeACL != nullptr)
    {
        chip::Platform::Delete(mExchangeACL);
        mExchangeACL = nullptr;
    }

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogProgress(ExchangeManager, "ec-- id: %d [%04" PRIX16 "], inUse: %d, addr: 0x%x", (this - em->mContextPool.begin()),
                    mExchangeId, em->GetContextsInUse(), this);
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

        // AND The message was sent by an initiator and the exchange context is a responder (IsInitiator==false)
        //    OR The message was sent by a responder and the exchange context is an initiator (IsInitiator==true) (for the broadcast
        //    case, the initiator is ill defined)

        && (payloadHeader.IsInitiator() != IsInitiator());
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

void ExchangeContext::HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError)
{
    ExchangeContext * ec = reinterpret_cast<ExchangeContext *>(aAppState);

    if (ec == nullptr)
        return;

    // NOTE: we don't set mResponseExpected to false here because the response could still arrive. If the user
    // wants to never receive the response, they must close the exchange context.

    ExchangeDelegateBase * delegate = ec->GetDelegate();

    // Call the user's timeout handler.
    if (delegate != nullptr)
        delegate->OnResponseTimeout(ec);
}

CHIP_ERROR ExchangeContext::HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                          const Transport::PeerAddress & peerAddress, PacketBufferHandle msgBuf)
{
    // We hold a reference to the ExchangeContext here to
    // guard against Close() calls(decrementing the reference
    // count) by the protocol before the CHIP Exchange
    // layer has completed its work on the ExchangeContext.
    Retain();

    ExchangeMessageDispatch * dispatch = GetMessageDispatch();
    ApplicationExchangeDispatch defaultDispatch;

    if (dispatch == nullptr)
    {
        defaultDispatch.Init(mExchangeMgr->GetReliableMessageMgr(), mExchangeMgr->GetSessionMgr());
        dispatch = &defaultDispatch;
    }

    CHIP_ERROR err =
        dispatch->OnMessageReceived(payloadHeader, packetHeader.GetMessageId(), peerAddress, GetReliableMessageContext());
    SuccessOrExit(err);

    // The SecureChannel::StandaloneAck message type is only used for CRMP; do not pass such messages to the application layer.
    if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck))
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
        mDelegate->OnMessageReceived(this, packetHeader, payloadHeader, std::move(msgBuf));
    }
    else
    {
        DefaultOnMessageReceived(this, packetHeader, payloadHeader.GetProtocolID(), payloadHeader.GetMessageType(),
                                 std::move(msgBuf));
    }

exit:
    // Release the reference to the ExchangeContext that was held at the beginning of this function.
    // This call should also do the needful of closing the ExchangeContext if the protocol has
    // already made a prior call to Close().
    Release();

    return err;
}

} // namespace Messaging
} // namespace chip
