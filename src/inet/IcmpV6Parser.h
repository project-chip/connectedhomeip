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

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Inet {

/// Inner IPv6 + UDP headers; nothing past them is parsed.
constexpr size_t kMinInnerDatagramLen = 48;

// RFC 4443 s3.1. otIcmp6Code omits the code.
constexpr uint8_t kIcmp6TypeDestUnreachable = 1;
constexpr uint8_t kIcmp6CodePortUnreachable = 4;

/// Parse the datagram embedded in an ICMPv6 port-unreachable error. We sent it, so @p out reads as
/// SrcAddress/SrcPort = us, DestAddress/DestPort = the unreachable peer.
CHIP_ERROR ParseIcmpV6PortUnreachInnerDatagram(const uint8_t * buf, size_t len, IPPacketInfo & out);

} // namespace Inet
} // namespace chip
