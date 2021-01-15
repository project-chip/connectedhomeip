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

#pragma once

#include <stdint.h>
#include <string.h>

#include <messaging/ReliableMessageProtocolConfig.h>

#include <core/CHIPError.h>
#include <inet/InetLayer.h>
#include <lib/core/ReferenceCounted.h>
#include <support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

class ChipMessageInfo;
class ExchangeContext;
enum class MessageFlagValues : uint32_t;
class ReliableMessageContext;
class ReliableMessageManager;

class ReliableMessageDelegate
{
public:
    virtual ~ReliableMessageDelegate() {}

    /* Application callbacks */
    virtual void OnSendError(CHIP_ERROR err) = 0; /**< Application callback for error while sending. */
    virtual void OnAckRcvd()                 = 0; /**< Application callback for received acknowledgment. */
};

class ReliableMessageContext
{
public:
    ReliableMessageContext();

    void Init(ReliableMessageManager * manager, ExchangeContext * exchange);
    void SetConfig(ReliableMessageProtocolConfig config) { mConfig = config; }
    void SetDelegate(ReliableMessageDelegate * delegate) { mDelegate = delegate; }

    CHIP_ERROR FlushAcks();
    uint64_t GetCurrentRetransmitTimeoutTick();

    CHIP_ERROR SendStandaloneAckMessage();

    bool AutoRequestAck() const;
    void SetAutoRequestAck(bool autoReqAck);
    bool ShouldDropAckDebug() const;
    void SetDropAckDebug(bool inDropAckDebug);
    bool IsAckPending() const;
    void SetAckPending(bool inAckPending);
    bool HasPeerRequestedAck() const;
    void SetPeerRequestedAck(bool inPeerRequestedAck);
    bool HasRcvdMsgFromPeer() const;
    void SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer);

private:
    enum class Flags : uint16_t
    {
        /// When set, automatically request an acknowledgment whenever a message is sent via UDP.
        kFlagAutoRequestAck = 0x0004,

        /// Internal and debug only: when set, the exchange layer does not send an acknowledgment.
        kFlagDropAckDebug = 0x0008,

        /// If a response is expected for a message that is being sent.
        kFlagResponseExpected = 0x0010,

        /// When set, signifies that there is an acknowledgment pending to be sent back.
        kFlagAckPending = 0x0020,

        /// When set, signifies that at least one message received on this exchange requested an acknowledgment.
        /// This flag is read by the application to decide if it needs to request an acknowledgment for the
        /// response message it is about to send. This flag can also indicate whether peer is using ReliableMessageProtocol.
        kFlagPeerRequestedAck = 0x0040,

        /// When set, signifies that at least one message has been received from peer on this exchange context.
        kFlagMsgRcvdFromPeer = 0x0080,
    };

    BitFlags<uint16_t, Flags> mFlags; // Internal state flags

    void Retain();
    void Release();
    CHIP_ERROR HandleRcvdAck(uint32_t AckMsgId);
    CHIP_ERROR HandleNeedsAck(uint32_t MessageId, BitFlags<uint32_t, MessageFlagValues> Flags);

private:
    friend class ReliableMessageManager;
    friend class ExchangeContext;

    ReliableMessageManager * mManager;
    ExchangeContext * mExchange;
    ReliableMessageDelegate * mDelegate;
    ReliableMessageProtocolConfig mConfig;
    uint16_t mNextAckTimeTick; // Next time for triggering Solo Ack
    uint32_t mPendingPeerAckId;
};

} // namespace Messaging
} // namespace chip
