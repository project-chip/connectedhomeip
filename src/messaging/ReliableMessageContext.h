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
 *      This file defines the reliable message context for the CHIP Message
 *      Layer. The context is one-on-one relationship with a chip session.
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include <messaging/ReliableMessageProtocolConfig.h>

#include <lib/core/CHIPError.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/DLLUtil.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/SystemLayer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace app {
class TestCommandInteraction;
class TestReadInteraction;
class TestWriteInteraction;
} // namespace app
namespace Messaging {

class ChipMessageInfo;
class ExchangeContext;
enum class MessageFlagValues : uint32_t;
class ReliableMessageMgr;

class ReliableMessageContext
{
public:
    ReliableMessageContext();

    /**
     * Flush the pending Ack for current exchange.
     *
     */
    CHIP_ERROR FlushAcks();

    /**
     * Take the pending peer ack message counter from the context.  This must
     * only be called when HasPiggybackAckPending() is true.  After this call,
     * IsAckPending() will be false; it's the caller's responsibility to send
     * the ack.
     */
    uint32_t TakePendingPeerAckMessageCounter()
    {
        SetAckPending(false);
        return mPendingPeerAckMessageCounter;
    }

    /**
     * Check whether we have a mPendingPeerAckMessageCounter. The counter is
     * valid once we receive a message which requests an ack. Once
     * mPendingPeerAckMessageCounter is valid, it never stops being valid.
     */
    bool HasPiggybackAckPending() const;

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
     */
    CHIP_ERROR SendStandaloneAckMessage();

    /**
     *  Determine whether an acknowledgment will be requested whenever a message is sent for the exchange.
     *
     *  @return Returns 'true' an acknowledgment will be requested whenever a message is sent, else 'false'.
     */
    bool AutoRequestAck() const;

    /**
     * Set whether an acknowledgment should be requested whenever a message is sent.
     *
     * @param[in] autoReqAck            A Boolean indicating whether or not an
     *                                  acknowledgment should be requested whenever a
     *                                  message is sent.
     */
    void SetAutoRequestAck(bool autoReqAck);

    /**
     *  Determine whether there is already an acknowledgment pending to be sent to the peer on this exchange.
     *
     *  @return Returns 'true' if there is already an acknowledgment pending  on this exchange, else 'false'.
     */
    bool IsAckPending() const;

    /// Determine whether the reliable message context is waiting for an ack.
    bool IsWaitingForAck() const;

    /// Set whether the reliable message context is waiting for an ack.
    void SetWaitingForAck(bool waitingForAck);

    /// Set if this exchange is requesting Sleepy End Device active mode
    void SetRequestingActiveMode(bool activeMode);

    /// Determine whether this exchange is a EphemeralExchange for replying a StandaloneAck
    bool IsEphemeralExchange() const;

    /**
     * Get the reliable message manager that corresponds to this reliable
     * message context.
     */
    ReliableMessageMgr * GetReliableMessageMgr();

protected:
    bool WaitingForResponseOrAck() const;
    void SetWaitingForResponseOrAck(bool waitingForResponseOrAck);

    enum class Flags : uint16_t
    {
        /// When set, signifies that this context is the initiator of the exchange.
        kFlagInitiator = (1u << 0),

        /// When set, signifies that a response is expected for a message that is being sent.
        kFlagResponseExpected = (1u << 1),

        /// When set, automatically request an acknowledgment whenever a message is sent via UDP.
        kFlagAutoRequestAck = (1u << 2),

        /// When set, signifies the reliable message context is waiting for an
        /// ack: a message that needs an ack has been sent, no ack has been
        /// received, and we have not yet run out of MRP retries.
        kFlagWaitingForAck = (1u << 3),

        /// When set, signifies that there is an acknowledgment pending to be sent back.
        kFlagAckPending = (1u << 4),

        /// When set, signifies that mPendingPeerAckMessageCounter is valid.
        /// The counter is valid once we receive a message which requests an ack.
        /// Once mPendingPeerAckMessageCounter is valid, it never stops being valid.
        kFlagAckMessageCounterIsValid = (1u << 5),

        /// When set, signifies that this exchange is waiting for a call to SendMessage.
        kFlagWillSendMessage = (1u << 6),

        /// When set, we have had Close() or Abort() called on us already.
        kFlagClosed = (1u << 7),

        /// When set, signifies that the exchange created sorely for replying a StandaloneAck
        kFlagEphemeralExchange = (1u << 8),

        /// When set, ignore session being released, because we are releasing it ourselves.
        kFlagIgnoreSessionRelease = (1u << 9),

        // This flag is used to determine if the peer (receiver) should be considered active or not.
        // When set, sender knows it has received at least one application-level message
        // from the peer and can assume the peer (receiver) is active.
        // If the flag is not set, we don't know if the peer (receiver) is active or not.
        kFlagReceivedAtLeastOneMessage = (1u << 10),

        /// When set:
        ///
        /// (1) We sent a message that expected a response (hence
        ///     IsResponseExpected() is true).
        /// (2) We have received neither a response nor an ack for that message.
        kFlagWaitingForResponseOrAck = (1u << 11),
    };

    BitFlags<Flags> mFlags; // Internal state flags

private:
    void HandleRcvdAck(uint32_t ackMessageCounter);
    CHIP_ERROR HandleNeedsAck(uint32_t messageCounter, BitFlags<MessageFlagValues> messageFlags);
    CHIP_ERROR HandleNeedsAckInner(uint32_t messageCounter, BitFlags<MessageFlagValues> messageFlags);
    ExchangeContext * GetExchangeContext();

    /**
     *  Set if an acknowledgment needs to be sent back to the peer on this exchange.
     *
     *  @param[in]  inAckPending A Boolean indicating whether (true) or not
     *                          (false) an acknowledgment should be sent back
     *                          in response to a received message.
     */
    void SetAckPending(bool inAckPending);

    // Set our pending peer ack message counter and any other state needed to ensure that we
    // will send that ack at some point.
    void SetPendingPeerAckMessageCounter(uint32_t aPeerAckMessageCounter);

    friend class ReliableMessageMgr;
    friend class ExchangeContext;
    friend class ExchangeMessageDispatch;
    friend class ::chip::app::TestCommandInteraction;
    friend class ::chip::app::TestReadInteraction;
    friend class ::chip::app::TestWriteInteraction;

    System::Clock::Timestamp mNextAckTime; // Next time for triggering Solo Ack
    uint32_t mPendingPeerAckMessageCounter;
};

inline bool ReliableMessageContext::AutoRequestAck() const
{
    return mFlags.Has(Flags::kFlagAutoRequestAck);
}

inline bool ReliableMessageContext::IsAckPending() const
{
    return mFlags.Has(Flags::kFlagAckPending);
}

inline bool ReliableMessageContext::IsWaitingForAck() const
{
    return mFlags.Has(Flags::kFlagWaitingForAck);
}

inline bool ReliableMessageContext::HasPiggybackAckPending() const
{
    return mFlags.Has(Flags::kFlagAckMessageCounterIsValid);
}

inline void ReliableMessageContext::SetAutoRequestAck(bool autoReqAck)
{
    mFlags.Set(Flags::kFlagAutoRequestAck, autoReqAck);
}

inline void ReliableMessageContext::SetAckPending(bool inAckPending)
{
    mFlags.Set(Flags::kFlagAckPending, inAckPending);
}

inline bool ReliableMessageContext::IsEphemeralExchange() const
{
    return mFlags.Has(Flags::kFlagEphemeralExchange);
}

inline bool ReliableMessageContext::WaitingForResponseOrAck() const
{
    return mFlags.Has(Flags::kFlagWaitingForResponseOrAck);
}

inline void ReliableMessageContext::SetWaitingForResponseOrAck(bool waitingForResponseOrAck)
{
    mFlags.Set(Flags::kFlagWaitingForResponseOrAck, waitingForResponseOrAck);
}

} // namespace Messaging
} // namespace chip
