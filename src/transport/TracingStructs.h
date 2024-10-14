/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#pragma once

#include <lib/support/Span.h>
#include <transport/Session.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Tracing {

/// Concrete definitions of the message tracing info that session managers
/// report

enum class OutgoingMessageType
{
    kGroupMessage,
    kSecureSession,
    kUnauthenticated,
};

/// A message is about to be sent
struct MessageSendInfo
{
    OutgoingMessageType messageType;
    const PayloadHeader * payloadHeader;
    const PacketHeader * packetHeader;
    const chip::ByteSpan payload;
};

enum class IncomingMessageType
{
    kGroupMessage,
    kSecureUnicast,
    kUnauthenticated,
};

/// A message has been received
struct MessageReceivedInfo
{
    const IncomingMessageType messageType;
    const PayloadHeader * payloadHeader;
    const PacketHeader * packetHeader;
    const Transport::Session * session;
    const Transport::PeerAddress * peerAddress;
    const chip::ByteSpan payload;
};

} // namespace Tracing
} // namespace chip
