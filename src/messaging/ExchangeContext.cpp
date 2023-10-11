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

#include <app/icd/ICDNotifier.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/Defer.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ApplicationExchangeDispatch.h>
#include <messaging/EphemeralExchangeDispatch.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <platform/LockTracker.h>
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

bool ExchangeContext::IsResponseExpected() const
{
    return mFlags.Has(Flags::kFlagResponseExpected);
}

void ExchangeContext::SetResponseExpected(bool inResponseExpected)
{
    mFlags.Set(Flags::kFlagResponseExpected, inResponseExpected);
    SetWaitingForResponseOrAck(inResponseExpected);
}

void ExchangeContext::UseSuggestedResponseTimeout(Timeout applicationProcessingTimeout)
{
    SetResponseTimeout(mSession->ComputeRoundTripTimeout(applicationProcessingTimeout));
}

void ExchangeContext::SetResponseTimeout(Timeout timeout)
{
    mResponseTimeout = timeout;
}

CHIP_ERROR ExchangeContext::SendMessage(Protocols::Id protocolId, uint8_t msgType, PacketBufferHandle && msgBuf,
                                        const SendFlags & sendFlags)
{
    // This is the first point all outgoing messages funnel through.  Ensure
    // that our message sends are all synchronized correctly.
    assertChipStackLockedByCurrentThread();

    bool isStandaloneAck =
        (protocolId == Protocols::SecureChannel::Id) && msgType == to_underlying(Protocols::SecureChannel::MsgType::StandaloneAck);

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mSession, CHIP_ERROR_CONNECTION_ABORTED);

    // Don't let method get called on a freed object.
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() > 0);

    // we hold the exchange context here in case the entity that
    // originally generated it tries to close it as a result of
    // an error arising below. at the end, we have to close it.
    ExchangeHandle ref(*this);

    // If session requires MRP, NoAutoRequestAck send flag is not specified and is not a group exchange context, request reliable
    // transmission.
    bool reliableTransmissionRequested =
        GetSessionHandle()->RequireMRP() && !sendFlags.Has(SendMessageFlags::kNoAutoRequestAck) && !IsGroupExchangeContext();

    bool currentMessageExpectResponse = false;
    // If a response message is expected...
    if (sendFlags.Has(SendMessageFlags::kExpectResponse) && !IsGroupExchangeContext())
    {
        // Only one 'response expected' message can be outstanding at a time.
        if (IsResponseExpected())
        {
            // TODO: add a test for this case.
            return CHIP_ERROR_INCORRECT_STATE;
        }

        SetResponseExpected(true);

        // Arm the response timer if a timeout has been specified.
        if (mResponseTimeout > System::Clock::kZero)
        {
            CHIP_ERROR err = StartResponseTimer();
            if (err != CHIP_NO_ERROR)
            {
                SetResponseExpected(false);
                return err;
            }
            currentMessageExpectResponse = true;
        }
    }

    {
        // ExchangeContext for group are supposed to always be Initiator
        if (IsGroupExchangeContext() && !IsInitiator())
        {
            return CHIP_ERROR_INTERNAL;
        }

        //
        // It is possible that we might have evicted a session as a side-effect of processing an inbound message on this exchange.
        // We cannot proceed any further sending a message since we don't have an attached session, so let's error out.
        //
        // This should not happen to well-behaved logic attempting to sending messages on exchanges, so let's print out a warning
        // to ensure it alerts someone to fixing their logic...
        //
        if (!mSession)
        {
            ChipLogError(ExchangeManager,
                         "WARNING: We shouldn't be sending a message on an exchange that has no attached session...");
            return CHIP_ERROR_MISSING_SECURE_SESSION;
        }

        SessionHandle session = GetSessionHandle();
        CHIP_ERROR err;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (mInjectedFailures.Has(InjectedFailureType::kFailOnSend))
        {
            err = CHIP_ERROR_SENDING_BLOCKED;
        }
        else
        {
#endif
            err = mDispatch.SendMessage(GetExchangeMgr()->GetSessionManager(), session, mExchangeId, IsInitiator(),
                                        GetReliableMessageContext(), reliableTransmissionRequested, protocolId, msgType,
                                        std::move(msgBuf));
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        }
#endif
        if (err != CHIP_NO_ERROR)
        {
            // We should only cancel the response timer if the ExchangeContext fails to send the message that expects a
            // response.
            if (currentMessageExpectResponse)
            {
                CancelResponseTimer();
                SetResponseExpected(false);
            }

            // If we can't even send a message (send failed with a non-transient
            // error), mark the session as defunct, just like we would if we
            // thought we sent the message and never got a response.
            if (session->IsSecureSession() && session->AsSecureSession()->IsCASESession())
            {
                session->AsSecureSession()->MarkAsDefunct();
            }
        }
        else
        {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
            app::ICDNotifier::GetInstance().BroadcastNetworkActivityNotification();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

            // Standalone acks are not application-level message sends.
            if (!isStandaloneAck)
            {
                //
                // Once we've sent the message successfully, we can clear out the WillSendMessage flag.
                //
                mFlags.Clear(Flags::kFlagWillSendMessage);
                MessageHandled();
            }
        }

        return err;
    }
}

void ExchangeContext::DoClose(bool clearRetransTable)
{
    if (mFlags.Has(Flags::kFlagClosed))
    {
        return;
    }

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
        mExchangeMgr->GetReliableMessageMgr()->ClearRetransTable(this);
    }

    if (IsResponseExpected())
    {
        // Cancel the response timer.
        CancelResponseTimer();
    }
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
    ChipLogDetail(ExchangeManager, "ec - close[" ChipLogFormatExchange "], %s", ChipLogValueExchange(this), __func__);
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
    ChipLogDetail(ExchangeManager, "ec - abort[" ChipLogFormatExchange "], %s", ChipLogValueExchange(this), __func__);
#endif

    DoClose(true);
    Release();
}

void ExchangeContextDeletor::Release(ExchangeContext * ec)
{
    ec->mExchangeMgr->ReleaseContext(ec);
}

ExchangeContext::ExchangeContext(ExchangeManager * em, uint16_t ExchangeId, const SessionHandle & session, bool Initiator,
                                 ExchangeDelegate * delegate, bool isEphemeralExchange) :
    mDispatch(GetMessageDispatch(isEphemeralExchange, delegate)),
    mSession(*this)
{
    VerifyOrDie(mExchangeMgr == nullptr);

    mExchangeMgr = em;
    mExchangeId  = ExchangeId;
    mSession.Grab(session);
    mFlags.Set(Flags::kFlagInitiator, Initiator);
    mFlags.Set(Flags::kFlagEphemeralExchange, isEphemeralExchange);
    mDelegate = delegate;

    //
    // If we're an initiator and we just created this exchange, we obviously did so to send a message. Let's go ahead and
    // set the flag on this to correctly mark it as so.
    //
    // This only applies to non-ephemeral exchanges. Ephemeral exchanges do not have an intention of sending out a message
    // since they're created expressly for the purposes of sending out a standalone ACK when the message could not be handled
    // through normal means.
    //
    if (Initiator && !isEphemeralExchange)
    {
        WillSendMessage();
    }

    SetAckPending(false);

    // Do not request Ack for multicast
    SetAutoRequestAck(!session->IsGroupSession());

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::ICDNotifier::GetInstance().BroadcastActiveRequestNotification(app::ICDListener::KeepActiveFlags::kExchangeContextOpen);
#endif

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec++ id: " ChipLogFormatExchange, ChipLogValueExchange(this));
#endif
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

ExchangeContext::~ExchangeContext()
{
    VerifyOrDie(mExchangeMgr != nullptr && GetReferenceCount() == 0);

    //
    // Ensure that DoClose has been called by the time we get here. If not, we have a leak somewhere.
    //
    VerifyOrDie(mFlags.Has(Flags::kFlagClosed));

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::ICDNotifier::GetInstance().BroadcastActiveRequestWithdrawal(app::ICDListener::KeepActiveFlags::kExchangeContextOpen);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    // Ideally, in this scenario, the retransmit table should
    // be clear of any outstanding messages for this context and
    // the boolean parameter passed to DoClose() should not matter.

    DoClose(false);
    mExchangeMgr = nullptr;

#if defined(CHIP_EXCHANGE_CONTEXT_DETAIL_LOGGING)
    ChipLogDetail(ExchangeManager, "ec-- id: " ChipLogFormatExchange, ChipLogValueExchange(this));
#endif
    SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumContexts);
}

bool ExchangeContext::MatchExchange(const SessionHandle & session, const PacketHeader & packetHeader,
                                    const PayloadHeader & payloadHeader)
{
    // A given message is part of a particular exchange if...
    return

        // The exchange identifier of the message matches the exchange identifier of the context.
        (mExchangeId == payloadHeader.GetExchangeID())

        // AND The Session associated with the incoming message matches the Session associated with the exchange.
        && (mSession.Contains(session))

        // TODO: This check should be already implied by the equality of session check,
        // It should be removed after we have implemented the temporary node id for PASE and CASE sessions
        && (IsEncryptionRequired() == packetHeader.IsEncrypted())

        // AND The message was sent by an initiator and the exchange context is a responder (IsInitiator==false)
        //    OR The message was sent by a responder and the exchange context is an initiator (IsInitiator==true) (for the broadcast
        //    case, the initiator is ill defined)

        && (payloadHeader.IsInitiator() != IsInitiator());
}

void ExchangeContext::OnSessionReleased()
{
    if (ShouldIgnoreSessionRelease())
    {
        return;
    }

    if (mFlags.Has(Flags::kFlagClosed))
    {
        // Exchange is already being closed. It may occur when closing an exchange after sending
        // RemoveFabric response which triggers removal of all sessions for the given fabric.
        mExchangeMgr->GetReliableMessageMgr()->ClearRetransTable(this);
        return;
    }

    // Hold a ref to ourselves so we can make calls into our delegate that might
    // decrease our refcount without worrying about use-after-free.
    ExchangeHandle ref(*this);

    //
    // If a send is not expected (either because we're waiting for a response OR
    // we're in the middle of processing a OnMessageReceived call), we can go ahead
    // and notify our delegate and abort the exchange since we still own the ref.
    //
    if (!IsSendExpected())
    {
        if (IsResponseExpected())
        {
            // If we're waiting on a response, we now know it's never going to show up
            // and we should notify our delegate accordingly.
            CancelResponseTimer();
            // We want to Abort, not just Close, so that RMP bits are cleared, so
            // don't let NotifyResponseTimeout close us.
            NotifyResponseTimeout(/* aCloseIfNeeded = */ false);
        }

        Abort();
    }
    else
    {
        DoClose(true /* clearRetransTable */);
    }
}

CHIP_ERROR ExchangeContext::StartResponseTimer()
{
    System::Layer * lSystemLayer = mExchangeMgr->GetSessionManager()->SystemLayer();
    if (lSystemLayer == nullptr)
    {
        // this is an assertion error, which shall never happen
        return CHIP_ERROR_INTERNAL;
    }

    return lSystemLayer->StartTimer(mResponseTimeout, HandleResponseTimeout, this);
}

void ExchangeContext::CancelResponseTimer()
{
    System::Layer * lSystemLayer = mExchangeMgr->GetSessionManager()->SystemLayer();
    if (lSystemLayer == nullptr)
    {
        // this is an assertion error, which shall never happen
        return;
    }

    lSystemLayer->CancelTimer(HandleResponseTimeout, this);
}

void ExchangeContext::HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState)
{
    ExchangeContext * ec = reinterpret_cast<ExchangeContext *>(aAppState);

    if (ec == nullptr)
        return;

    ec->NotifyResponseTimeout(/* aCloseIfNeeded = */ true);
}

void ExchangeContext::NotifyResponseTimeout(bool aCloseIfNeeded)
{
    // Grab the value of WaitingForResponseOrAck() before we mess with our state.
    bool gotMRPAck = !WaitingForResponseOrAck();

    SetResponseExpected(false);

    // Hold a ref to ourselves so we can make calls into our delegate that might
    // decrease our refcount (e.g. by expiring out session) without worrying
    // about use-after-free.
    ExchangeHandle ref(*this);

    // mSession might be null if this timeout is due to the session being
    // evicted.
    if (mSession)
    {
        // If we timed out _after_ getting an ack for the message, that means
        // the session is probably fine (since our message and the ack got
        // through), so don't mark the session defunct if we got an MRP ack.
        if (!gotMRPAck)
        {
            if (mSession->IsSecureSession() && mSession->AsSecureSession()->IsCASESession())
            {
                mSession->AsSecureSession()->MarkAsDefunct();
            }
            mSession->DispatchSessionEvent(&SessionDelegate::OnSessionHang);
        }
    }

    ExchangeDelegate * delegate = GetDelegate();

    // Call the user's timeout handler.
    if (delegate != nullptr)
    {
        delegate->OnResponseTimeout(this);
    }

    if (aCloseIfNeeded)
    {
        MessageHandled();
    }
}

CHIP_ERROR ExchangeContext::HandleMessage(uint32_t messageCounter, const PayloadHeader & payloadHeader, MessageFlags msgFlags,
                                          PacketBufferHandle && msgBuf)
{
    // We hold a reference to the ExchangeContext here to
    // guard against Close() calls(decrementing the reference
    // count) by the protocol before the CHIP Exchange
    // layer has completed its work on the ExchangeContext.
    ExchangeHandle ref(*this);

    bool isStandaloneAck = payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck);
    bool isDuplicate     = msgFlags.Has(MessageFlagValues::kDuplicateMessage);

    auto deferred = MakeDefer([&]() {
        // Duplicates and standalone acks are not application-level messages, so they should generally not lead to any state
        // changes.  The one exception to that is that if we have a null mDelegate then our lifetime is not application-defined,
        // since we don't interact with the application at that point.  That can happen when we are already closed (in which case
        // MessageHandled is a no-op) or if we were just created to send a standalone ack for this incoming message, in which case
        // we should treat it as an app-level message for purposes of our state.
        if ((isStandaloneAck || isDuplicate) && mDelegate != nullptr)
        {
            return;
        }

        MessageHandled();
    });

    if (mDispatch.IsReliableTransmissionAllowed() && !IsGroupExchangeContext())
    {
        if (!msgFlags.Has(MessageFlagValues::kDuplicateMessage) && payloadHeader.IsAckMsg() &&
            payloadHeader.GetAckMessageCounter().HasValue())
        {
            HandleRcvdAck(payloadHeader.GetAckMessageCounter().Value());
        }

        if (payloadHeader.NeedsAck())
        {
            // An acknowledgment needs to be sent back to the peer for this message on this exchange,

            HandleNeedsAck(messageCounter, msgFlags);
        }
    }

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

    if (IsEphemeralExchange())
    {
        // The EphemeralExchange has done its job, since StandaloneAck is sent in previous FlushAcks() call.
        return CHIP_NO_ERROR;
    }

    if (IsWaitingForAck())
    {
        // The only way we can get here is a spec violation on the other side:
        // we sent a message that needs an ack, and the other side responded
        // with a message that does not contain an ack for the message we sent.
        // Just drop this message; if we delivered it to our delegate it might
        // try to send another message-needing-an-ack in response, which would
        // violate our internal invariants.
        ChipLogError(ExchangeManager, "Dropping message without piggyback ack when we are waiting for an ack.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // message received
    app::ICDNotifier::GetInstance().BroadcastNetworkActivityNotification();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    if (IsResponseExpected())
    {
        // Since we got the response, cancel the response timer.
        CancelResponseTimer();

        // If the context was expecting a response to a previously sent message, this message
        // is implicitly that response.
        SetResponseExpected(false);
    }

    // Don't send messages on to our delegate if our dispatch does not allow
    // those messages.
    if (mDelegate != nullptr && mDispatch.MessagePermitted(payloadHeader.GetProtocolID(), payloadHeader.GetMessageType()))
    {
        return mDelegate->OnMessageReceived(this, payloadHeader, std::move(msgBuf));
    }

    DefaultOnMessageReceived(this, payloadHeader.GetProtocolID(), payloadHeader.GetMessageType(), messageCounter,
                             std::move(msgBuf));
    return CHIP_NO_ERROR;
}

void ExchangeContext::MessageHandled()
{
    if (mFlags.Has(Flags::kFlagClosed) || IsResponseExpected() || IsSendExpected())
    {
        return;
    }

    Close();
}

ExchangeMessageDispatch & ExchangeContext::GetMessageDispatch(bool isEphemeralExchange, ExchangeDelegate * delegate)
{
    if (isEphemeralExchange)
        return EphemeralExchangeDispatch::Instance();

    if (delegate != nullptr)
        return delegate->GetMessageDispatch();

    return ApplicationExchangeDispatch::Instance();
}

void ExchangeContext::AbortAllOtherCommunicationOnFabric()
{
    if (!mSession || !mSession->IsSecureSession())
    {
        ChipLogError(ExchangeManager, "AbortAllOtherCommunicationOnFabric called when we don't have a PASE/CASE session");
        return;
    }

    // Save our session so it does not actually go away.
    Optional<SessionHandle> session = mSession.Get();

    SetIgnoreSessionRelease(true);

    GetExchangeMgr()->GetSessionManager()->ExpireAllSessionsForFabric(mSession->GetFabricIndex());

    mSession.GrabExpiredSession(session.Value());

    SetIgnoreSessionRelease(false);
}

void ExchangeContext::ExchangeSessionHolder::GrabExpiredSession(const SessionHandle & session)
{
    VerifyOrDie(session->AsSecureSession()->IsPendingEviction());
    GrabUnchecked(session);
}

} // namespace Messaging
} // namespace chip
