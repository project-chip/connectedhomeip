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

#include <inet/InetLayer.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

class ChipMessageInfo;
class ExchangeContext;
enum class MessageFlagValues : uint32_t;
class ReliableMessageContext;
class ReliableMessageMgr;

class ReliableMessageContext
{
public:
    ReliableMessageContext();

    void SetConfig(ReliableMessageProtocolConfig config) { mConfig = config; }

    /**
     * Flush the pending Ack for current exchange.
     *
     */
    CHIP_ERROR FlushAcks();

    /**
     * Take the pending peer ack id from the context.  This must only be called
     * when IsAckPending() is true.  After this call, IsAckPending() will be
     * false; it's the caller's responsibility to send the ack.
     */
    uint32_t TakePendingPeerAckId()
    {
        SetAckPending(false);
        return mPendingPeerAckId;
    }

    /**
     *  Get the initial retransmission interval. It would be the time to wait before
     *  retransmission after first failure.
     *
     *  @return the initial retransmission interval.
     */
    uint64_t GetInitialRetransmitTimeoutTick();

    /**
     *  Get the active retransmit interval. It would be the time to wait before
     *  retransmission after subsequent failures.
     *
     *  @return the active retransmission interval.
     */
    uint64_t GetActiveRetransmitTimeoutTick();

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
     *  Determine whether the ChipExchangeManager should not send an
     *  acknowledgement.
     *
     *  For internal, debug use only.
     */
    bool ShouldDropAckDebug() const;

    /**
     *  Set whether the ChipExchangeManager should not send acknowledgements
     *  for this context.
     *
     *  For internal, debug use only.
     *
     *  @param[in]  inDropAckDebug  A Boolean indicating whether (true) or not
     *                         (false) the acknowledgements should be not
     *                         sent for the exchange.
     */
    void SetDropAckDebug(bool inDropAckDebug);

    /**
     *  Determine whether there is already an acknowledgment pending to be sent to the peer on this exchange.
     *
     *  @return Returns 'true' if there is already an acknowledgment pending  on this exchange, else 'false'.
     */
    bool IsAckPending() const;

    /**
     *  Determine whether at least one message has been received
     *  on this exchange from peer.
     *
     *  @return Returns 'true' if message received, else 'false'.
     */
    bool HasRcvdMsgFromPeer() const;

    /**
     *  Set if a message has been received from the peer
     *  on this exchange.
     *
     *  @param[in]  inMsgRcvdFromPeer  A Boolean indicating whether (true) or not
     *                                 (false) a message has been received
     *                                 from the peer on this exchange context.
     */
    void SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer);

    /**
     *  Determine whether there is already an acknowledgment pending to be sent to the peer on this exchange.
     *
     *  @return Returns 'true' if there is already an acknowledgment pending  on this exchange, else 'false'.
     */
    bool IsOccupied() const;

    /**
     *  Set whether there is an acknowledgment panding to be send to the peer on
     *  this exchange.
     *
     *  @param[in]  inOccupied Whether there is a pending acknowledgment.
     */
    void SetOccupied(bool inOccupied);

    /**
     * Get the reliable message manager that corresponds to this reliable
     * message context.
     */
    ReliableMessageMgr * GetReliableMessageMgr();

protected:
    enum class Flags : uint16_t
    {
        /// When set, signifies that this context is the initiator of the exchange.
        kFlagInitiator = 0x0001,

        /// When set, signifies that a response is expected for a message that is being sent.
        kFlagResponseExpected = 0x0002,

        /// When set, automatically request an acknowledgment whenever a message is sent via UDP.
        kFlagAutoRequestAck = 0x0004,

        /// Internal and debug only: when set, the exchange layer does not send an acknowledgment.
        kFlagDropAckDebug = 0x0008,

        /// When set, signifies current reliable message context is in usage.
        kFlagOccupied = 0x0010,

        /// When set, signifies that there is an acknowledgment pending to be sent back.
        kFlagAckPending = 0x0020,

        /// When set, signifies that at least one message has been received from peer on this exchange context.
        kFlagMsgRcvdFromPeer = 0x0040,

        /// When set, signifies that this exchange is waiting for a call to SendMessage.
        kFlagWillSendMessage = 0x0080,

        /// When set, signifies that we are currently in the middle of HandleMessage.
        kFlagHandlingMessage = 0x0100,
        /// When set, we have had Close() or Abort() called on us already.
        kFlagClosed = 0x0200,
    };

    BitFlags<Flags> mFlags; // Internal state flags

private:
    void RetainContext();
    void ReleaseContext();
    CHIP_ERROR HandleRcvdAck(uint32_t AckMsgId);
    CHIP_ERROR HandleNeedsAck(uint32_t messageId, BitFlags<MessageFlagValues> messageFlags);
    CHIP_ERROR HandleNeedsAckInner(uint32_t messageId, BitFlags<MessageFlagValues> messageFlags);
    ExchangeContext * GetExchangeContext();

    /**
     *  Set if an acknowledgment needs to be sent back to the peer on this exchange.
     *
     *  @param[in]  inAckPending A Boolean indicating whether (true) or not
     *                          (false) an acknowledgment should be sent back
     *                          in response to a received message.
     */
    void SetAckPending(bool inAckPending);

    // Set our pending peer ack id and any other state needed to ensure that we
    // will send that ack at some point.
    void SetPendingPeerAckId(uint32_t aPeerAckId);

private:
    friend class ReliableMessageMgr;
    friend class ExchangeContext;
    friend class ExchangeMessageDispatch;

    ReliableMessageProtocolConfig mConfig;
    uint16_t mNextAckTimeTick; // Next time for triggering Solo Ack
    uint32_t mPendingPeerAckId;
};

} // namespace Messaging
} // namespace chip
