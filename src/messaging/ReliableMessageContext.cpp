/*
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements the CHIP reliable message protocol.
 *
 */

#include <inttypes.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ReliableMessageContext.h>

#include <core/CHIPEncoding.h>
#include <messaging/ErrorCategory.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageManager.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Messaging {

ReliableMessageContext::ReliableMessageContext() :
    mManager(nullptr), mExchange(nullptr), mDelegate(nullptr), mConfig(gDefaultReliableMessageProtocolConfig), mNextAckTimeTick(0),
    mPendingPeerAckId(0)
{}

void ReliableMessageContext::Init(ReliableMessageManager * manager, ExchangeContext * exchange)
{
    mManager  = manager;
    mExchange = exchange;
    mDelegate = nullptr;

    SetDropAckDebug(false);
    SetAckPending(false);
    SetPeerRequestedAck(false);
    SetMsgRcvdFromPeer(false);
    SetAutoRequestAck(true);
}

void ReliableMessageContext::Retain()
{
    mExchange->Retain();
}

void ReliableMessageContext::Release()
{
    mExchange->Release();
}

/**
 * Returns whether an acknowledgment will be requested whenever a message is sent.
 */
bool ReliableMessageContext::AutoRequestAck() const
{
    return mFlags.Has(Flags::kFlagAutoRequestAck);
}

/**
 *  Determine whether there is already an acknowledgment pending to be sent
 *  to the peer on this exchange.
 *
 */
bool ReliableMessageContext::IsAckPending() const
{
    return mFlags.Has(Flags::kFlagAckPending);
}

/**
 *  Determine whether peer requested acknowledgment for at least one message
 *  on this exchange.
 *
 *  @return Returns 'true' if acknowledgment requested, else 'false'.
 */
bool ReliableMessageContext::HasPeerRequestedAck() const
{
    return mFlags.Has(Flags::kFlagPeerRequestedAck);
}

/**
 *  Determine whether at least one message has been received
 *  on this exchange from peer.
 *
 *  @return Returns 'true' if message received, else 'false'.
 */
bool ReliableMessageContext::HasRcvdMsgFromPeer() const
{
    return mFlags.Has(Flags::kFlagMsgRcvdFromPeer);
}

/**
 * Set whether an acknowledgment should be requested whenever a message is sent.
 *
 * @param[in] autoReqAck            A Boolean indicating whether or not an
 *                                  acknowledgment should be requested whenever a
 *                                  message is sent.
 */
void ReliableMessageContext::SetAutoRequestAck(bool autoReqAck)
{
    mFlags.Set(Flags::kFlagAutoRequestAck, autoReqAck);
}

/**
 *  Set if a message has been received from the peer
 *  on this exchange.
 *
 *  @param[in]  inMsgRcvdFromPeer  A Boolean indicating whether (true) or not
 *                                 (false) a message has been received
 *                                 from the peer on this exchange context.
 *
 */
void ReliableMessageContext::SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer)
{
    mFlags.Set(Flags::kFlagMsgRcvdFromPeer, inMsgRcvdFromPeer);
}

/**
 *  Set if an acknowledgment needs to be sent back to the peer on this exchange.
 *
 *  @param[in]  inAckPending A Boolean indicating whether (true) or not
 *                          (false) an acknowledgment should be sent back
 *                          in response to a received message.
 *
 */
void ReliableMessageContext::SetAckPending(bool inAckPending)
{
    mFlags.Set(Flags::kFlagAckPending, inAckPending);
}

/**
 *  Set if an acknowledgment was requested in the last message received
 *  on this exchange.
 *
 *  @param[in]  inPeerRequestedAck A Boolean indicating whether (true) or not
 *                                 (false) an acknowledgment was requested
 *                                 in the last received message.
 *
 */
void ReliableMessageContext::SetPeerRequestedAck(bool inPeerRequestedAck)
{
    mFlags.Set(Flags::kFlagPeerRequestedAck, inPeerRequestedAck);
}

/**
 *  Set whether the ChipExchangeManager should not send acknowledgements
 *  for this context.
 *
 *  For internal, debug use only.
 *
 *  @param[in]  inDropAckDebug  A Boolean indicating whether (true) or not
 *                         (false) the acknowledgements should be not
 *                         sent for the exchange.
 *
 */
void ReliableMessageContext::SetDropAckDebug(bool inDropAckDebug)
{
    mFlags.Set(Flags::kFlagDropAckDebug, inDropAckDebug);
}

/**
 *  Determine whether the ChipExchangeManager should not send an
 *  acknowledgement.
 *
 *  For internal, debug use only.
 *
 */
bool ReliableMessageContext::ShouldDropAckDebug() const
{
    return mFlags.Has(Flags::kFlagDropAckDebug);
}

// Flush the pending Ack
CHIP_ERROR ReliableMessageContext::FlushAcks()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (IsAckPending())
    {
        // Send the acknowledgment as a SecureChannel::StandStandaloneAck message
        err = SendStandaloneAckMessage();

        if (err == CHIP_NO_ERROR)
        {
#if !defined(NDEBUG)
            ChipLogProgress(ExchangeManager, "Flushed pending ack for MsgId:%08" PRIX32, mPendingPeerAckId);
#endif
        }
    }

    return err;
}

/**
 *  Get the current retransmit timeout. It would be either the initial or
 *  the active retransmit timeout based on whether the ExchangeContext has
 *  an active message exchange going with its peer.
 *
 *  @return the current retransmit time.
 */
uint64_t ReliableMessageContext::GetCurrentRetransmitTimeoutTick()
{
    return (HasRcvdMsgFromPeer() ? mConfig.mActiveRetransTimeoutTick : mConfig.mInitialRetransTimeoutTick);
}

/**
 *  Process received Ack. Remove the corresponding message context from the RetransTable and execute the application
 *  callback
 *
 *  @note
 *    This message is part of the CHIP Reliable Messaging protocol.
 *
 *  @param[in]    AckMsgId         The msgId of incoming Ack message.
 *
 *  @retval  #CHIP_ERROR_INVALID_ACK_ID                 if the msgId of received Ack is not in the RetransTable.
 *  @retval  #CHIP_NO_ERROR                             if the context was removed.
 *
 */
CHIP_ERROR ReliableMessageContext::HandleRcvdAck(uint32_t AckMsgId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Msg is an Ack; Check Retrans Table and remove message context
    if (!mManager->CheckAndRemRetransTable(this, AckMsgId))
    {
#if !defined(NDEBUG)
        ChipLogError(ExchangeManager, "CHIP MsgId:%08" PRIX32 " not in RetransTable", AckMsgId);
#endif
        err = CHIP_ERROR_INVALID_ACK_ID;
        // Optionally call an application callback with this error.
    }
    else
    {
        if (mDelegate)
        {
            mDelegate->OnAckRcvd();
        }

#if !defined(NDEBUG)
        ChipLogProgress(ExchangeManager, "Removed CHIP MsgId:%08" PRIX32 " from RetransTable", AckMsgId);
#endif
    }

    return err;
}

CHIP_ERROR ReliableMessageContext::HandleNeedsAck(uint32_t MessageId, BitFlags<uint32_t, MessageFlagValues> MsgFlags)

{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Skip processing ack if drop ack debug is enabled.
    if (ShouldDropAckDebug())
        return err;

    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    mManager->ExpireTicks();

    // If the message IS a duplicate.
    if (MsgFlags.Has(MessageFlagValues::kDuplicateMessage))
    {
#if !defined(NDEBUG)
        ChipLogProgress(ExchangeManager, "Forcing tx of solitary ack for duplicate MsgId:%08" PRIX32, MessageId);
#endif
        // Is there pending ack for a different message id.
        bool wasAckPending = IsAckPending() && mPendingPeerAckId != MessageId;

        // Temporary store currently pending ack id (even if there is none).
        uint32_t tempAckId = mPendingPeerAckId;

        // Set the pending ack id.
        mPendingPeerAckId = MessageId;

        // Send the Ack for the duplication message in a SecureChannel::StandaloneAck message.
        err = SendStandaloneAckMessage();

        // If there was pending ack for a different message id.
        if (wasAckPending)
        {
            // Restore previously pending ack id.
            mPendingPeerAckId = tempAckId;
            SetAckPending(true);
        }

        SuccessOrExit(err);
    }
    // Otherwise, the message IS NOT a duplicate.
    else
    {
        if (IsAckPending())
        {
#if !defined(NDEBUG)
            ChipLogProgress(ExchangeManager, "Pending ack queue full; forcing tx of solitary ack for MsgId:%08" PRIX32,
                            mPendingPeerAckId);
#endif
            // Send the Ack for the currently pending Ack in a SecureChannel::StandaloneAck message.
            err = SendStandaloneAckMessage();
            SuccessOrExit(err);
        }

        // Replace the Pending ack id.
        mPendingPeerAckId = MessageId;
        mNextAckTimeTick  = static_cast<uint16_t>(mConfig.mAckPiggybackTimeoutTick +
                                                 mManager->GetTickCounterFromTimeDelta(System::Timer::GetCurrentEpoch()));
        SetAckPending(true);
    }

exit:
    // Schedule next physical wakeup
    mManager->StartTimer();
    return err;
}

/**
 *  Send a SecureChannel::StandaloneAck message.
 *
 *  @note  When sent via UDP, the null message is sent *without* requesting an acknowledgment,
 *  even in the case where the auto-request acknowledgment feature has been enabled on the
 *  exchange.
 *
 *  @retval  #CHIP_ERROR_NO_MEMORY   If no available PacketBuffers.
 *  @retval  #CHIP_NO_ERROR          If the method succeeded or the error wasn't critical.
 *  @retval  other                    Another critical error returned by SendMessage().
 *
 */
CHIP_ERROR ReliableMessageContext::SendStandaloneAckMessage()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Allocate a buffer for the null message
    System::PacketBufferHandle msgBuf = System::PacketBuffer::NewWithAvailableSize(0);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Send the null message
    if (mExchange != nullptr)
    {
        err = mExchange->SendMessage(Protocols::SecureChannel::MsgType::StandaloneAck, std::move(msgBuf),
                                     BitFlags<uint16_t, SendMessageFlags>{ SendMessageFlags::kNoAutoRequestAck });
    }
    else
    {
        ChipLogError(ExchangeManager, "ExchangeContext is not initilized in ReliableMessageContext");
        err = CHIP_ERROR_NOT_CONNECTED;
    }

exit:
    if (IsSendErrorNonCritical(err))
    {
        ChipLogError(ExchangeManager, "Non-crit err %ld sending solitary ack", long(err));
        err = CHIP_NO_ERROR;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send Solitary ack for MsgId:%08" PRIX32 ":%ld", mPendingPeerAckId, (long) err);
    }

    return err;
}

} // namespace Messaging
} // namespace chip
