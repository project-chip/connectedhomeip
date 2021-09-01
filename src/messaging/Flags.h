/*
 *
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
 *      This file defines flags for messaging layer.
 */

#pragma once

#include <lib/support/BitFlags.h>
#include <stdint.h>

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
    kReuseSourceId = 0x00000020,
    /**< Indicates that the CHIP message is already encoded. */
    kMessageEncoded = 0x00001000,
    /**< Indicates that default IPv6 source address selection should be used when sending IPv6 multicast messages. */
    kDefaultMulticastSourceAddress = 0x00002000,
    /**< Indicates that the message is a duplicate of a previously received message. */
    kDuplicateMessage = 0x00004000,
    /**< Indicates that the peer's group key message counter is not synchronized. */
    kPeerGroupMsgIdNotSynchronized = 0x00008000,
    /**< Indicates that the source of the message is the initiator of the CHIP exchange. */
    kFromInitiator = 0x00010000,
    /**< Indicates that message is being sent/received via the local ephemeral UDP port. */
    kViaEphemeralUDPPort = 0x00020000,
};

using MessageFlags = BitFlags<MessageFlagValues>;

enum class SendMessageFlags : uint16_t
{
    kNone = 0x0000,
    /**< Used to indicate that automatic retransmission is enabled. */
    kAutoRetrans = 0x0001,
    /**< Used to indicate that a response is expected within a specified timeout. */
    kExpectResponse = 0x0002,
    /**< Used to indicate that the source node ID in the message header can be reused. */
    kReuseSourceId = 0x0020,
    /**< Used to indicate that the message is already encoded. */
    kAlreadyEncoded = 0x0080,
    /**< Used to indicate that default IPv6 source address selection should be used when sending IPv6 multicast messages. */
    kDefaultMulticastSourceAddress = 0x0100,
    /**< Used to indicate that the current message is the initiator of the exchange. */
    kFromInitiator = 0x0200,
    /**< Suppress the auto-request acknowledgment feature when sending a message. */
    kNoAutoRequestAck = 0x0400,
};

using SendFlags = BitFlags<SendMessageFlags>;

} // namespace Messaging
} // namespace chip
