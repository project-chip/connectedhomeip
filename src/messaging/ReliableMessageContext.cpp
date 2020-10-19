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

#include <messaging/ReliableMessageContext.h>

#include <core/CHIPEncoding.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageManager.h>
#include <messaging/Utils.h>
#include <protocols/CHIPProtocols.h>
#include <protocols/common/CommonProtocol.h>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>

namespace chip {
namespace messaging {

void ReliableMessageContextDeletor::Release(ReliableMessageContext * obj)
{
    obj->mDelegate.GetManager().FreeContext(obj);
}

ReliableMessageContext::ReliableMessageContext(ReliableMessageDelegate & delegate) :
    mConfig(gDefaultReliableMessageProtocolConfig), mNextAckTimeTick(0), mThrottleTimeoutTick(0), mPendingPeerAckId(0),
    mDelegate(delegate)
{}

/**
 *  Determine whether there is already an acknowledgment pending to be sent
 *  to the peer on this exchange.
 *
 */
bool ReliableMessageContext::IsAckPending() const
{
    return GetFlag(mFlags, kFlagAckPending);
}

/**
 *  Determine whether peer requested acknowledgment for at least one message
 *  on this exchange.
 *
 *  @return Returns 'true' if acknowledgment requested, else 'false'.
 */
bool ReliableMessageContext::HasPeerRequestedAck() const
{
    return GetFlag(mFlags, kFlagPeerRequestedAck);
}

/**
 *  Determine whether at least one message has been received
 *  on this exchange from peer.
 *
 *  @return Returns 'true' if message received, else 'false'.
 */
bool ReliableMessageContext::HasRcvdMsgFromPeer() const
{
    return GetFlag(mFlags, kFlagMsgRcvdFromPeer);
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
    SetFlag(mFlags, kFlagMsgRcvdFromPeer, inMsgRcvdFromPeer);
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
    SetFlag(mFlags, kFlagAckPending, inAckPending);
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
    SetFlag(mFlags, kFlagPeerRequestedAck, inPeerRequestedAck);
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
    SetFlag(mFlags, kFlagDropAckDebug, inDropAckDebug);
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
    return GetFlag(mFlags, kFlagDropAckDebug);
}

// Flush the pending Ack
CHIP_ERROR ReliableMessageContext::FlushAcks()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (IsAckPending())
    {
        // Send the acknowledgment as a Common::Null message
        err = SendCommonNullMessage();

        if (err == CHIP_NO_ERROR)
        {
#if defined(DEBUG)
            ChipLogProgress(ExchangeManager, "Flushed pending ack for MsgId:%08" PRIX32 " to Peer %016" PRIX64, mPendingPeerAckId,
                            PeerNodeId);
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
 *  Send a Throttle Flow message to the peer node requesting it to throttle its sending of messages.
 *
 *  @note
 *    This message is part of the CHIP Reliable Messaging protocol.
 *
 *  @param[in]    pauseTimeMillis    The time (in milliseconds) that the recipient is expected
 *                                   to throttle its sending.
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               If an invalid argument was passed to this SendMessage API.
 *  @retval  #CHIP_ERROR_SEND_THROTTLED                 If this exchange context has been throttled when using the
 *                                                       CHIP reliable messaging protocol.
 *  @retval  #CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE If there is a mismatch in the specific send operation and the
 *                                                       CHIP message protocol version that is supported. For example,
 *                                                       this error would be generated if CHIP Reliable Messaging
 *                                                       semantics are being attempted when the CHIP message protocol
 *                                                       version is V1.
 *  @retval  #CHIP_ERROR_NOT_CONNECTED                  If the context was associated with a connection that is now
 *                                                       closed.
 *  @retval  #CHIP_ERROR_INCORRECT_STATE                If the state of the exchange context is incorrect.
 *  @retval  #CHIP_NO_ERROR                             If the CHIP layer successfully sent the message down to the
 *                                                       network layer.
 *
 */
CHIP_ERROR ReliableMessageContext::SendThrottleFlow(uint32_t pauseTimeMillis)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * msgBuf = nullptr;
    uint8_t * p                   = nullptr;
    uint8_t msgLen                = sizeof(pauseTimeMillis);

    msgBuf = System::PacketBuffer::NewWithAvailableSize(msgLen);
    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    p = msgBuf->Start();

    // Encode the fields in the buffer
    Encoding::LittleEndian::Write32(p, pauseTimeMillis);
    msgBuf->SetDataLength(msgLen);

    // Send a Throttle Flow message to the peer.  Throttle Flow messages must never request
    // acknowledgment, so suppress the auto-request ACK feature on the exchange in case it has been
    // enabled by the application.
    err = mDelegate.SendMessage(Protocols::kChipProtocol_Common, Protocols::Common::kMsgType_RMP_Throttle_Flow, msgBuf,
                                kSendFlag_NoAutoRequestAck);

exit:
    return err;
}

/**
 *  Send a Delayed Delivery message to notify a sender node that its previously sent message would experience an expected
 *  delay before being delivered to the recipient. One of the possible causes for messages to be delayed before being
 *  delivered is when the recipient end node is sleepy. This message is potentially generated by a suitable intermediate
 *  node in the send path who has enough knowledge of the recipient to infer about the delayed delivery. Upon receiving
 *  this message, the sender would re-adjust its retransmission timers for messages that seek acknowledgments back.
 *
 *  @note
 *    This message is part of the CHIP Reliable Messaging protocol.
 *
 *  @param[in]    pauseTimeMillis    The time (in milliseconds) that the previously sent message is expected
 *                                   to be delayed before being delivered.
 *
 *  @param[in]    delayedNodeId      The node identifier of the peer node to whom the mesage delivery would be delayed.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this SendMessage API.
 *  @retval  #CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE if there is a mismatch in the specific send operation and the
 *                                                       CHIP message protocol version that is supported. For example,
 *                                                       this error would be generated if CHIP Reliable Messaging
 *                                                       semantics are being attempted when the CHIP message protocol
 *                                                       version is V1.
 *  @retval  #CHIP_ERROR_NOT_CONNECTED                  if the context was associated with a connection that is now
 *                                                       closed.
 *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
 *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully sent the message down to the
 *                                                       network layer.
 *
 */
CHIP_ERROR ReliableMessageContext::SendDelayedDelivery(uint32_t pauseTimeMillis, uint64_t delayedNodeId)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * msgBuf = nullptr;
    uint8_t * p                   = nullptr;
    uint8_t msgLen                = sizeof(pauseTimeMillis) + sizeof(delayedNodeId);

    msgBuf = System::PacketBuffer::NewWithAvailableSize(msgLen);
    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    p = msgBuf->Start();
    // Set back the pointer by the length of the fields

    // Encode the fields in the buffer
    Encoding::LittleEndian::Write32(p, pauseTimeMillis);
    Encoding::LittleEndian::Write64(p, delayedNodeId);
    msgBuf->SetDataLength(msgLen);

    // Send a Delayed Delivery message to the peer.  Delayed Delivery messages must never request
    // acknowledgment, so suppress the auto-request ACK feature on the exchange in case it has been
    // enabled by the application.
    err = mDelegate.SendMessage(Protocols::kChipProtocol_Common, Protocols::Common::kMsgType_RMP_Delayed_Delivery, msgBuf,
                                kSendFlag_NoAutoRequestAck);

exit:
    return err;
}

/**
 *  Process received Ack. Remove the corresponding message context from the RetransTable and execute the application
 *  callback
 *
 *  @note
 *    This message is part of the CHIP Reliable Messaging protocol.
 *
 *  @param[in]    exchHeader         CHIP exchange information for incoming Ack message.
 *
 *  @retval  #CHIP_ERROR_INVALID_ACK_ID                 if the msgId of received Ack is not in the RetransTable.
 *  @retval  #CHIP_NO_ERROR                             if the context was removed.
 *
 */
CHIP_ERROR ReliableMessageContext::HandleRcvdAck(uint32_t AckMsgId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Msg is an Ack; Check Retrans Table and remove message context
    if (!mDelegate.GetManager().CheckAndRemRetransTable(this, AckMsgId))
    {
#if defined(DEBUG)
        ChipLogError(ExchangeManager, "CHIP MsgId:%08" PRIX32 " not in RetransTable", AckMsgId);
#endif
        err = CHIP_ERROR_INVALID_ACK_ID;
        // Optionally call an application callback with this error.
    }
    else
    {
        mDelegate.OnAckRcvd();
#if defined(DEBUG)
        ChipLogProgress(ExchangeManager, "Removed CHIP MsgId:%08" PRIX32 " from RetransTable", AckMsgId);
#endif
    }

    return err;
}

CHIP_ERROR ReliableMessageContext::HandleNeedsAck(uint32_t MessageId, uint32_t Flags)

{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    mDelegate.GetManager().ExpireTicks();

    // If the message IS a duplicate.
    if (Flags & kChipMessageFlag_DuplicateMessage)
    {
#if defined(DEBUG)
        ChipLogProgress(ExchangeManager, "Forcing tx of solitary ack for duplicate MsgId:%08" PRIX32, MessageId);
#endif
        // Is there pending ack for a different message id.
        bool wasAckPending = IsAckPending() && mPendingPeerAckId != MessageId;

        // Temporary store currently pending ack id (even if there is none).
        uint32_t tempAckId = mPendingPeerAckId;

        // Set the pending ack id.
        mPendingPeerAckId = MessageId;

        // Send the Ack for the duplication message in a Common::Null message.
        err = SendCommonNullMessage();

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
#if defined(DEBUG)
            ChipLogProgress(ExchangeManager, "Pending ack queue full; forcing tx of solitary ack for MsgId:%08" PRIX32,
                            mPendingPeerAckId);
#endif
            // Send the Ack for the currently pending Ack in a Common::Null message.
            err = SendCommonNullMessage();
            SuccessOrExit(err);
        }

        // Replace the Pending ack id.
        mPendingPeerAckId = MessageId;
        mNextAckTimeTick  = mConfig.mAckPiggybackTimeoutTick +
            static_cast<uint16_t>(mDelegate.GetManager().GetTickCounterFromTimeDelta(System::Timer::GetCurrentEpoch()));
        SetAckPending(true);
    }

exit:
    // Schedule next physical wakeup
    mDelegate.GetManager().StartTimer();
    return err;
}

CHIP_ERROR ReliableMessageContext::HandleThrottleFlow(uint32_t PauseTimeMillis)
{
    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    mDelegate.GetManager().ExpireTicks();

    // Flow Control Message Received; Adjust Throttle timeout accordingly.
    // A PauseTimeMillis of zero indicates that peer is unthrottling this Exchange.

    if (0 != PauseTimeMillis)
    {
        mThrottleTimeoutTick = static_cast<uint16_t>(
            mDelegate.GetManager().GetTickCounterFromTimeDelta(System::Timer::GetCurrentEpoch() + PauseTimeMillis));
        mDelegate.GetManager().PauseRetransTable(this, PauseTimeMillis);
    }
    else
    {
        mThrottleTimeoutTick = 0;
        mDelegate.GetManager().ResumeRetransTable(this);
    }

    // Call OnThrottleRcvd application callback
    mDelegate.OnThrottleRcvd(PauseTimeMillis);

    // Schedule next physical wakeup
    mDelegate.GetManager().StartTimer();
    return CHIP_NO_ERROR;
}

/**
 *  Send a Common::Null message.
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
CHIP_ERROR ReliableMessageContext::SendCommonNullMessage()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * msgBuf = nullptr;

    // Allocate a buffer for the null message
    msgBuf = System::PacketBuffer::NewWithAvailableSize(0);
    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // Send the null message
    err = mDelegate.SendMessage(chip::Protocols::kChipProtocol_Common, chip::Protocols::Common::kMsgType_Null, msgBuf,
                                kSendFlag_NoAutoRequestAck);

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

} // namespace messaging
} // namespace chip
