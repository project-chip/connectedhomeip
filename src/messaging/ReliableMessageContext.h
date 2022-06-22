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

    /// Determine whether there is message hasn't been acknowledged.
    bool IsMessageNotAcked() const;

    /// Set whether there is a message hasn't been acknowledged.
    void SetMessageNotAcked(bool messageNotAcked);

    /// Set if this exchange is requesting Sleepy End Device active mode
    void SetRequestingActiveMode(bool activeMode);

    /// Determine whether this exchange is requesting Sleepy End Device active mode
    bool IsRequestingActiveMode() const;

    /// Determine whether this exchange is a EphemeralExchange for replying a StandaloneAck
    bool IsEphemeralExchange() const;

    /**
     * Get the reliable message manager that corresponds to this reliable
     * message context.
     */
    ReliableMessageMgr * GetReliableMessageMgr();

protected:
    enum class Flags : uint16_t
    {
        /// When set, signifies that this context is the initiator of the exchange.
        kFlagInitiator = (1u << 0),

        /// When set, signifies that a response is expected for a message that is being sent.
        kFlagResponseExpected = (1u << 1),

        /// When set, automatically request an acknowledgment whenever a message is sent via UDP.
        kFlagAutoRequestAck = (1u << 2),

        /// When set, signifies there is a message which hasn't been acknowledged.
        kFlagMessageNotAcked = (1u << 3),

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

        /// When set, signifies that the exchange is requesting Sleepy End Device active mode.
        kFlagActiveMode = (1u << 8),

        /// When set, signifies that the exchange created sorely for replying a StandaloneAck
        kFlagEphemeralExchange = (1u << 9),

        /// When set, ignore session being released, because we are releasing it ourselves.
        kFlagIgnoreSessionRelease = (1u << 10),
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

inline bool ReliableMessageContext::IsMessageNotAcked() const
{
    return mFlags.Has(Flags::kFlagMessageNotAcked);
}

inline bool ReliableMessageContext::HasPiggybackAckPending() const
{
    return mFlags.Has(Flags::kFlagAckMessageCounterIsValid);
}

inline bool ReliableMessageContext::IsRequestingActiveMode() const
{
    return mFlags.Has(Flags::kFlagActiveMode);
}

inline void ReliableMessageContext::SetAutoRequestAck(bool autoReqAck)
{
    mFlags.Set(Flags::kFlagAutoRequestAck, autoReqAck);
}

inline void ReliableMessageContext::SetAckPending(bool inAckPending)
{
    mFlags.Set(Flags::kFlagAckPending, inAckPending);
}

inline void ReliableMessageContext::SetMessageNotAcked(bool messageNotAcked)
{
    mFlags.Set(Flags::kFlagMessageNotAcked, messageNotAcked);
}

inline void ReliableMessageContext::SetRequestingActiveMode(bool activeMode)
{
    mFlags.Set(Flags::kFlagActiveMode, activeMode);
}

inline bool ReliableMessageContext::IsEphemeralExchange() const
{
    return mFlags.Has(Flags::kFlagEphemeralExchange);
}

} // namespace Messaging
} // namespace chip
