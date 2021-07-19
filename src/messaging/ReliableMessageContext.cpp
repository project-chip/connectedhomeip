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

#include <core/CHIPEncoding.h>
#include <messaging/ErrorCategory.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Messaging {

ReliableMessageContext::ReliableMessageContext() :
    mConfig(gDefaultReliableMessageProtocolConfig), mNextAckTimeTick(0), mPendingPeerAckId(0)
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
            ChipLogDetail(ExchangeManager, "Flushed pending ack for MsgId:%08" PRIX32, mPendingPeerAckId);
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
    if (!GetReliableMessageMgr()->CheckAndRemRetransTable(this, AckMsgId))
    {
#if !defined(NDEBUG)
        ChipLogError(ExchangeManager, "CHIP MsgId:%08" PRIX32 " not in RetransTable", AckMsgId);
#endif
        err = CHIP_ERROR_INVALID_ACK_ID;
        // Optionally call an application callback with this error.
    }
    else
    {
#if !defined(NDEBUG)
        ChipLogDetail(ExchangeManager, "Removed CHIP MsgId:%08" PRIX32 " from RetransTable", AckMsgId);
#endif
    }

    return err;
}

CHIP_ERROR ReliableMessageContext::HandleNeedsAck(uint32_t MessageId, BitFlags<MessageFlagValues> MsgFlags)

{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Skip processing ack if drop ack debug is enabled.
    if (ShouldDropAckDebug())
        return err;

    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    GetReliableMessageMgr()->ExpireTicks();

    // If the message IS a duplicate there will never be a response to it, so we
    // should not wait for one and just immediately send a standalone ack.
    if (MsgFlags.Has(MessageFlagValues::kDuplicateMessage))
    {
#if !defined(NDEBUG)
        ChipLogDetail(ExchangeManager, "Forcing tx of solitary ack for duplicate MsgId:%08" PRIX32, MessageId);
#endif
        // Is there pending ack for a different message id.
        bool wasAckPending = IsAckPending() && mPendingPeerAckId != MessageId;

        // Temporary store currently pending ack id (even if there is none).
        uint32_t tempAckId = mPendingPeerAckId;

        // Set the pending ack id.
        SetPendingPeerAckId(MessageId);

        // Send the Ack for the duplication message in a SecureChannel::StandaloneAck message.
        err = SendStandaloneAckMessage();

        // If there was pending ack for a different message id.
        if (wasAckPending)
        {
            // Restore previously pending ack id.
            SetPendingPeerAckId(tempAckId);
        }

        SuccessOrExit(err);
    }
    // Otherwise, the message IS NOT a duplicate.
    else
    {
        if (IsAckPending())
        {
#if !defined(NDEBUG)
            ChipLogDetail(ExchangeManager, "Pending ack queue full; forcing tx of solitary ack for MsgId:%08" PRIX32,
                          mPendingPeerAckId);
#endif
            // Send the Ack for the currently pending Ack in a SecureChannel::StandaloneAck message.
            err = SendStandaloneAckMessage();
            SuccessOrExit(err);
        }

        // Replace the Pending ack id.
        SetPendingPeerAckId(MessageId);
        mNextAckTimeTick =
            static_cast<uint16_t>(CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK +
                                  GetReliableMessageMgr()->GetTickCounterFromTimeDelta(System::Clock::GetMonotonicMilliseconds()));
    }

exit:
    // Schedule next physical wakeup
    GetReliableMessageMgr()->StartTimer();
    return err;
}

CHIP_ERROR ReliableMessageContext::SendStandaloneAckMessage()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Allocate a buffer for the null message
    System::PacketBufferHandle msgBuf = MessagePacketBuffer::New(0);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Send the null message
#if !defined(NDEBUG)
    ChipLogDetail(ExchangeManager, "Sending Standalone Ack for MsgId:%08" PRIX32, mPendingPeerAckId);
#endif

    err = GetExchangeContext()->SendMessage(Protocols::SecureChannel::MsgType::StandaloneAck, std::move(msgBuf),
                                            BitFlags<SendMessageFlags>{ SendMessageFlags::kNoAutoRequestAck });

exit:
    if (IsSendErrorNonCritical(err))
    {
        ChipLogError(ExchangeManager, "Non-crit err %" CHIP_ERROR_FORMAT " sending solitary ack", ChipError::FormatError(err));
        err = CHIP_NO_ERROR;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send Solitary ack for MsgId:%08" PRIX32 ":%" CHIP_ERROR_FORMAT, mPendingPeerAckId,
                     ChipError::FormatError(err));
    }

    return err;
}

void ReliableMessageContext::SetPendingPeerAckId(uint32_t aPeerAckId)
{
    mPendingPeerAckId = aPeerAckId;
    SetAckPending(true);
}

} // namespace Messaging
} // namespace chip
