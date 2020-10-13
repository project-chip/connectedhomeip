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
 *      This file defines the reliable message context for the CHIP Message
 *      Layer. The context is one-on-one relationship with a chip session.
 */

#ifndef CHIP_RELIABLE_MESSAGE_CONTEXT_H
#define CHIP_RELIABLE_MESSAGE_CONTEXT_H

#include <stdint.h>
#include <string.h>

#include <core/CHIPError.h>
#include <core/ReferenceCounted.h>
#include <inet/InetLayer.h>
#include <messaging/CHIPReliableMessageConfig.h>
#include <support/DLLUtil.h>
#include <system/SystemLayer.h>

namespace chip {

class ChipMessageInfo;
class CHIPReliableMessageManager;

class CHIPReliableMessageDelegate
{
public:
    /* Application callbacks */
    virtual void OnThrottleRcvd(uint32_t pauseTime) = 0; /**< Application callback for received Throttle message. */
    virtual void OnDDRcvd(uint32_t pauseTime)       = 0; /**< Application callback for received Delayed Delivery message. */
    virtual void OnSendError(CHIP_ERROR err)        = 0; /**< Application callback for error while sending. */
    virtual void OnAckRcvd()                        = 0; /**< Application callback for received acknowledgment. */

    /* Transport delegates */
    virtual CHIPReliableMessageManager & GetManager() = 0;
    virtual bool IsNode(uint64_t id)                  = 0;

    virtual CHIP_ERROR SendMessage(System::PacketBuffer * msgBuf, uint16_t sendFlags)                                      = 0;
    virtual CHIP_ERROR SendMessage(uint32_t profileId, uint8_t msgType, System::PacketBuffer * msgBuf, uint16_t sendFlags) = 0;
};

class CHIPReliableMessageContext : public ReferenceCounted<CHIPReliableMessageContext>
{
public:
    CHIPReliableMessageContext(CHIPReliableMessageDelegate & delegate);

    CHIP_ERROR RMPFlushAcks();
    uint32_t GetCurrentRetransmitTimeout();

    CHIP_ERROR RMPSendThrottleFlow(uint32_t PauseTimeMillis);
    CHIP_ERROR RMPSendDelayedDelivery(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);
    CHIP_ERROR SendCommonNullMessage();

    bool ShouldDropAck() const;
    bool IsAckPending() const;
    void SetDropAck(bool inDropAck);
    void SetAckPending(bool inAckPending);
    bool HasPeerRequestedAck() const;
    void SetPeerRequestedAck(bool inPeerRequestedAck);
    bool HasRcvdMsgFromPeer() const;
    void SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer);

private:
    static constexpr const uint16_t kFlagAutoRequestAck =
        0x0004; /// When set, automatically request an acknowledgment whenever a message is sent via UDP.
    static constexpr const uint16_t kFlagDropAck =
        0x0008; /// Internal and debug only: when set, the exchange layer does not send an acknowledgment.
    static constexpr const uint16_t kFlagResponseExpected = 0x0010; /// If a response is expected for a message that is being sent.
    static constexpr const uint16_t kFlagAckPending =
        0x0020; /// When set, signifies that there is an acknowledgment pending to be sent back.
    static constexpr const uint16_t kFlagPeerRequestedAck =
        0x0040; /// When set, signifies that at least one message received on this exchange requested an acknowledgment.
                /// This flag is read by the application to decide if it needs to request an acknowledgment for the
                /// response message it is about to send. This flag can also indicate whether peer is using RMP.
    static constexpr const uint16_t kFlagMsgRcvdFromPeer =
        0x0080; /// When set, signifies that at least one message has been received from peer on this exchange context.

    uint16_t mFlags; // Internal state flags

    CHIP_ERROR RMPHandleRcvdAck(uint32_t AckMsgId);
    CHIP_ERROR RMPHandleNeedsAck(uint32_t MessageId, uint32_t Flags);
    CHIP_ERROR HandleThrottleFlow(uint32_t PauseTimeMillis);

public:
    RMPConfig mRMPConfig;         /**< RMP configuration. */
    uint16_t mRMPNextAckTime;     // Next time for triggering Solo Ack
    uint16_t mRMPThrottleTimeout; // Timeout until when Throttle is On when RMPThrottleEnabled is set
    uint32_t mPendingPeerAckId;

    CHIPReliableMessageDelegate & mDelegate;
};

#endif // CHIP_RELIABLE_MESSAGE_CONTEXT_H

} // namespace chip
