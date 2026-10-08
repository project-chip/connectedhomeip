/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <inet/IPPacketInfo.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include <cstddef>

namespace chip {
namespace Inet {
namespace IPv6Datagram {

/// A 40-byte IPv6 header followed directly by an 8-byte UDP header.
constexpr size_t kUDPHeadersLen = 40 + 8;

/**
 * Parse the headers of a UDP-over-IPv6 datagram, or of a prefix of one, such as the copy an ICMPv6 error message
 * carries (RFC 4443 section 2.4(c)).
 *
 * The datagram must be uncompressed IPv6 whose Next Header is UDP; one with extension headers is rejected rather than
 * walked. Unicast Matter datagrams carry none: the SDK sets no extension-header socket options, OpenThread adds
 * Hop-by-Hop options only to multicast, and a Matter message fits the IPv6 minimum MTU, so it is not fragmented.
 *
 * @param[in]  datagram    The datagram, starting at its IPv6 header.
 * @param[out] addresses   Its source and destination addresses and ports. Interface is left null.
 * @param[out] udpPayload  The UDP payload bytes present, up to the UDP length, pointing into @p datagram.
 *
 * @retval CHIP_ERROR_INVALID_ARGUMENT  @p datagram is shorter than kUDPHeadersLen or is not UDP over IPv6.
 */
CHIP_ERROR ParseUDPHeaders(ByteSpan datagram, IPPacketInfo & addresses, ByteSpan & udpPayload);

} // namespace IPv6Datagram
} // namespace Inet
} // namespace chip
