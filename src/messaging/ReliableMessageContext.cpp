/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageContext.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Defer.h>
#include <messaging/ErrorCategory.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Messaging {

ReliableMessageContext::ReliableMessageContext() :
    mConfig(gDefaultReliableMessageProtocolConfig), mNextAckTimeTick(0), mPendingPeerAckMessageCounter(0)
{}

void ReliableMessageContext::RetainContext()
{
    GetExchangeContext()->Retain();
}

void ReliableMessageContext::ReleaseContext()
{
    GetExchangeContext()->Release();
}

bool ReliableMessageContext::AutoRequestAck() const
{
    return mFlags.Has(Flags::kFlagAutoRequestAck);
}

bool ReliableMessageContext::IsAckPending() const
{
    return mFlags.Has(Flags::kFlagAckPending);
}

bool ReliableMessageContext::HasRcvdMsgFromPeer() const
{
    return mFlags.Has(Flags::kFlagMsgRcvdFromPeer);
}

void ReliableMessageContext::SetAutoRequestAck(bool autoReqAck)
{
    mFlags.Set(Flags::kFlagAutoRequestAck, autoReqAck);
}

void ReliableMessageContext::SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer)
{
    mFlags.Set(Flags::kFlagMsgRcvdFromPeer, inMsgRcvdFromPeer);
}

void ReliableMessageContext::SetAckPending(bool inAckPending)
{
    mFlags.Set(Flags::kFlagAckPending, inAckPending);
}

void ReliableMessageContext::SetDropAckDebug(bool inDropAckDebug)
{
    mFlags.Set(Flags::kFlagDropAckDebug, inDropAckDebug);
}

bool ReliableMessageContext::IsOccupied() const
{
    return mFlags.Has(Flags::kFlagOccupied);
}

void ReliableMessageContext::SetOccupied(bool inOccupied)
{
    mFlags.Set(Flags::kFlagOccupied, inOccupied);
}

bool ReliableMessageContext::ShouldDropAckDebug() const
{
    return mFlags.Has(Flags::kFlagDropAckDebug);
}

ExchangeContext * ReliableMessageContext::GetExchangeContext()
{
    return static_cast<ExchangeContext *>(this);
}

ReliableMessageMgr * ReliableMessageContext::GetReliableMessageMgr()
{
    return static_cast<ExchangeContext *>(this)->GetExchangeMgr()->GetReliableMessageMgr();
}

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
            ChipLogDetail(ExchangeManager, "Flushed pending ack for MessageCounter:%08" PRIX32, mPendingPeerAckMessageCounter);
#endif
        }
    }

    return err;
}

uint64_t ReliableMessageContext::GetInitialRetransmitTimeoutTick()
{
    return mConfig.mInitialRetransTimeoutTick;
}

uint64_t ReliableMessageContext::GetActiveRetransmitTimeoutTick()
{
    return mConfig.mActiveRetransTimeoutTick;
}

/**
 *  Process received Ack. Remove the corresponding message context from the RetransTable and execute the application
 *  callback
 *
 *  @note
 *    This message is part of the CHIP Reliable Messaging protocol.
 *
 *  @param[in]    ackMessageCounter         The acknowledged message counter of the incoming message.
 *
 *  @retval  #CHIP_ERROR_INVALID_ACK_MESSAGE_COUNTER    if acknowledged message counter of received packet is not in the
 *                                                      RetransTable.
 *  @retval  #CHIP_NO_ERROR                             if the context was removed.
 *
 */
CHIP_ERROR ReliableMessageContext::HandleRcvdAck(uint32_t ackMessageCounter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Msg is an Ack; Check Retrans Table and remove message context
    if (!GetReliableMessageMgr()->CheckAndRemRetransTable(this, ackMessageCounter))
    {
#if !defined(NDEBUG)
        ChipLogError(ExchangeManager, "CHIP MessageCounter:%08" PRIX32 " not in RetransTable", ackMessageCounter);
#endif
        err = CHIP_ERROR_INVALID_ACK_MESSAGE_COUNTER;
        // Optionally call an application callback with this error.
    }
    else
    {
#if !defined(NDEBUG)
        ChipLogDetail(ExchangeManager, "Removed CHIP MessageCounter:%08" PRIX32 " from RetransTable", ackMessageCounter);
#endif
    }

    return err;
}

CHIP_ERROR ReliableMessageContext::HandleNeedsAck(uint32_t messageCounter, BitFlags<MessageFlagValues> messageFlags)

{
    // Skip processing ack if drop ack debug is enabled.
    if (ShouldDropAckDebug())
        return CHIP_NO_ERROR;

    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    GetReliableMessageMgr()->ExpireTicks();

    CHIP_ERROR err = HandleNeedsAckInner(messageCounter, messageFlags);

    // Schedule next physical wakeup on function exit
    GetReliableMessageMgr()->StartTimer();

    return err;
}

CHIP_ERROR ReliableMessageContext::HandleNeedsAckInner(uint32_t messageCounter, BitFlags<MessageFlagValues> messageFlags)

{
    // If the message IS a duplicate there will never be a response to it, so we
    // should not wait for one and just immediately send a standalone ack.
    if (messageFlags.Has(MessageFlagValues::kDuplicateMessage))
    {
#if !defined(NDEBUG)
        ChipLogDetail(ExchangeManager, "Forcing tx of solitary ack for duplicate MessageCounter:%08" PRIX32, messageCounter);
#endif
        // Is there pending ack for a different message counter.
        bool wasAckPending = IsAckPending() && mPendingPeerAckMessageCounter != messageCounter;

        // Temporary store currently pending ack message counter (even if there is none).
        uint32_t tempAckMessageCounter = mPendingPeerAckMessageCounter;

        // Set the pending ack message counter.
        SetPendingPeerAckMessageCounter(messageCounter);

        // Send the Ack for the duplication message in a SecureChannel::StandaloneAck message.
        CHIP_ERROR err = SendStandaloneAckMessage();

        // If there was pending ack for a different message counter.
        if (wasAckPending)
        {
            // Restore previously pending ack message counter.
            SetPendingPeerAckMessageCounter(tempAckMessageCounter);
        }

        return err;
    }
    // Otherwise, the message IS NOT a duplicate.
    else
    {
        if (IsAckPending())
        {
#if !defined(NDEBUG)
            ChipLogDetail(ExchangeManager, "Pending ack queue full; forcing tx of solitary ack for MessageCounter:%08" PRIX32,
                          mPendingPeerAckMessageCounter);
#endif
            // Send the Ack for the currently pending Ack in a SecureChannel::StandaloneAck message.
            ReturnErrorOnFailure(SendStandaloneAckMessage());
        }

        // Replace the Pending ack message counter.
        SetPendingPeerAckMessageCounter(messageCounter);
        mNextAckTimeTick =
            static_cast<uint16_t>(CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK +
                                  GetReliableMessageMgr()->GetTickCounterFromTimeDelta(System::Clock::GetMonotonicMilliseconds()));
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR ReliableMessageContext::SendStandaloneAckMessage()
{
    // Allocate a buffer for the null message
    System::PacketBufferHandle msgBuf = MessagePacketBuffer::New(0);
    if (msgBuf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Send the null message
#if !defined(NDEBUG)
    ChipLogDetail(ExchangeManager, "Sending Standalone Ack for MessageCounter:%08" PRIX32, mPendingPeerAckMessageCounter);
#endif

    CHIP_ERROR err = GetExchangeContext()->SendMessage(Protocols::SecureChannel::MsgType::StandaloneAck, std::move(msgBuf),
                                                       BitFlags<SendMessageFlags>{ SendMessageFlags::kNoAutoRequestAck });
    if (IsSendErrorNonCritical(err))
    {
        ChipLogError(ExchangeManager, "Non-crit err %" CHIP_ERROR_FORMAT " sending solitary ack", err.Format());
        return CHIP_NO_ERROR;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send Solitary ack for MessageCounter:%08" PRIX32 ":%" CHIP_ERROR_FORMAT,
                     mPendingPeerAckMessageCounter, err.Format());
    }

    return err;
}

void ReliableMessageContext::SetPendingPeerAckMessageCounter(uint32_t aPeerAckMessageCounter)
{
    mPendingPeerAckMessageCounter = aPeerAckMessageCounter;
    SetAckPending(true);
}

} // namespace Messaging
} // namespace chip
