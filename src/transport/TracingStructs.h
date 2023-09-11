/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/support/Span.h>
#include <transport/Session.h>
#include <transport/raw/MessageHeader.h>

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
