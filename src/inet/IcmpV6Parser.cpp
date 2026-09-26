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

#include <inet/IcmpV6Parser.h>

#include <inet/IPAddress.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Inet {

namespace {

constexpr size_t kIPv6HeaderLen = 40;
constexpr size_t kUDPHeaderLen  = 8;

constexpr size_t kIPv6NextHeaderOffset = 6;
constexpr size_t kIPv6SrcAddressOffset = 8;
constexpr size_t kIPv6DstAddressOffset = 24;

constexpr uint8_t kIPv6Version   = 6;
constexpr uint8_t kIPProtocolUDP = 17;

} // namespace

CHIP_ERROR ParseIcmpV6PortUnreachInnerDatagram(const uint8_t * buf, size_t len, IPPacketInfo & out)
{
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(len >= kMinInnerDatagramLen, CHIP_ERROR_INVALID_ARGUMENT);

    // Extension headers mean it is not a Matter message we sent.
    VerifyOrReturnError((buf[0] >> 4) == kIPv6Version, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(buf[kIPv6NextHeaderOffset] == kIPProtocolUDP, CHIP_ERROR_INVALID_ARGUMENT);

    // ReadAddress avoids any platform address type, so this builds under every networking backend.
    const uint8_t * src = buf + kIPv6SrcAddressOffset;
    const uint8_t * dst = buf + kIPv6DstAddressOffset;
    IPAddress srcAddress;
    IPAddress dstAddress;
    IPAddress::ReadAddress(src, srcAddress);
    IPAddress::ReadAddress(dst, dstAddress);

    Encoding::BigEndian::Reader udpReader(buf + kIPv6HeaderLen, kUDPHeaderLen);
    uint16_t srcPort = 0;
    uint16_t dstPort = 0;
    uint16_t udpLen  = 0;
    ReturnErrorOnFailure(udpReader.Read16(&srcPort).Read16(&dstPort).Read16(&udpLen).StatusCode());
    VerifyOrReturnError(udpLen >= kUDPHeaderLen, CHIP_ERROR_INVALID_ARGUMENT);

    out.SrcAddress  = srcAddress;
    out.DestAddress = dstAddress;
    out.SrcPort     = srcPort;
    out.DestPort    = dstPort;
    out.Interface   = InterfaceId::Null();
    return CHIP_NO_ERROR;
}

} // namespace Inet
} // namespace chip
