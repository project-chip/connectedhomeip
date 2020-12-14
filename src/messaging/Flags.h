/*
 *
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
 *      This file defines flags for messaging layer.
 */

#pragma once

#include <stdint.h>
#include <support/BitFlags.h>

namespace chip {
namespace Messaging {

/**
 *  @brief
 *    Flags associated with a inbound or outbound CHIP message.
 *
 *    The values defined here are for use within the ChipMessageInfo.Flags field.
 */
enum class MessageFlagValues : uint32_t
{
    /**< Indicates that the existing source node identifier must be reused. */
    kMessageFlag_ReuseSourceId = 0x00000020,
    /**< Indicates that the sending of the message needs to be delayed. */
    kMessageFlag_DelaySend = 0x00000040,
    /**< Indicates that the CHIP message is already encoded. */
    kMessageFlag_MessageEncoded = 0x00001000,
    /**< Indicates that default IPv6 source address selection should be used when sending IPv6 multicast messages. */
    kMessageFlag_DefaultMulticastSourceAddress = 0x00002000,
    /**< Indicates that the sender of the  message requested an acknowledgment. */
    kMessageFlag_PeerRequestedAck = 0x00004000,
    /**< Indicates that the message is a duplicate of a previously received message. */
    kMessageFlag_DuplicateMessage = 0x00008000,
    /**< Indicates that the peer's group key message counter is not synchronized. */
    kMessageFlag_PeerGroupMsgIdNotSynchronized = 0x00010000,
    /**< Indicates that the source of the message is the initiator of the CHIP exchange. */
    kMessageFlag_FromInitiator = 0x00020000,
    /**< Indicates that message is being sent/received via the local ephemeral UDP port. */
    kMessageFlag_ViaEphemeralUDPPort = 0x00040000,
};

using MessageFlags = BitFlags<uint32_t, MessageFlagValues>;

enum class SendMessageFlags : uint16_t
{
    kSendFlag_None = 0x0000,
    /**< Used to indicate that automatic retransmission is enabled. */
    kSendFlag_AutoRetrans = 0x0001,
    /**< Used to indicate that a response is expected within a specified timeout. */
    kSendFlag_ExpectResponse = 0x0002,
    /**< Used to indicate that the sending of the current message needs to be delayed. */
    kSendFlag_DelaySend = 0x0008,
    /**< Used to indicate that the source node ID in the message header can be reused. */
    kSendFlag_ReuseSourceId = 0x0020,
    /**< Used to indicate that the message is already encoded. */
    kSendFlag_AlreadyEncoded = 0x0080,
    /**< Used to indicate that default IPv6 source address selection should be used when sending IPv6 multicast messages. */
    kSendFlag_DefaultMulticastSourceAddress = 0x0100,
    /**< Used to indicate that the current message is the initiator of the exchange. */
    kSendFlag_FromInitiator = 0x0200,
    /**< Used to send a ReliableMessageProtocol message requesting an acknowledgment. */
    kSendFlag_RequestAck = 0x0400,
    /**< Suppress the auto-request acknowledgment feature when sending a message. */
    kSendFlag_NoAutoRequestAck = 0x0800,
};

using SendFlags = BitFlags<uint16_t, SendMessageFlags>;

} // namespace Messaging
} // namespace chip
