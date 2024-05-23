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
    /**< Indicates that the message is a duplicate of a previously received message. */
    kDuplicateMessage = 0x00000001,
};

using MessageFlags = BitFlags<MessageFlagValues>;

enum class SendMessageFlags : uint16_t
{
    kNone = 0x0000,
    /**< Used to indicate that a response is expected within a specified timeout. */
    kExpectResponse = 0x0001,
    /**< Suppress the auto-request acknowledgment feature when sending a message. */
    kNoAutoRequestAck = 0x0002,
};

using SendFlags = BitFlags<SendMessageFlags>;

} // namespace Messaging
} // namespace chip
