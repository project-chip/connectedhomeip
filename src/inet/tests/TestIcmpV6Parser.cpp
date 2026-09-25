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
#include <inet/IPPacketInfo.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>

#include <pw_unit_test/framework.h>

#include <array>
#include <cstdint>
#include <cstring>

namespace {

using namespace chip;
using namespace chip::Inet;

// Synthetic 48-byte inner datagram: IPv6 + UDP headers.
struct InnerDatagram
{
    std::array<uint8_t, 48> bytes{};
};

InnerDatagram MakeInner(const uint8_t (&srcAddr)[16], const uint8_t (&dstAddr)[16], uint8_t nextHdr, uint16_t udpSrcPort,
                        uint16_t udpDstPort, uint16_t udpLen)
{
    InnerDatagram d{};
    d.bytes[0] = 0x60; // version=6
    d.bytes[4] = static_cast<uint8_t>((udpLen >> 8) & 0xff);
    d.bytes[5] = static_cast<uint8_t>(udpLen & 0xff);
    d.bytes[6] = nextHdr;
    d.bytes[7] = 64; // hop limit
    std::memcpy(&d.bytes[8], srcAddr, 16);
    std::memcpy(&d.bytes[24], dstAddr, 16);
    d.bytes[40] = static_cast<uint8_t>((udpSrcPort >> 8) & 0xff);
    d.bytes[41] = static_cast<uint8_t>(udpSrcPort & 0xff);
    d.bytes[42] = static_cast<uint8_t>((udpDstPort >> 8) & 0xff);
    d.bytes[43] = static_cast<uint8_t>(udpDstPort & 0xff);
    d.bytes[44] = static_cast<uint8_t>((udpLen >> 8) & 0xff);
    d.bytes[45] = static_cast<uint8_t>(udpLen & 0xff);
    return d;
}

constexpr uint8_t kSrc[16] = { 0xfd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xaa };
constexpr uint8_t kDst[16] = { 0xfd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xbb };

TEST(IcmpV6Parser, ParseMinimalValid)
{
    auto inner = MakeInner(kSrc, kDst, /*nextHdr*/ 17, /*udpSrcPort*/ 54021, /*udpDstPort*/ 53412, /*udpLen*/ 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), inner.bytes.size(), info), CHIP_NO_ERROR);
    EXPECT_EQ(info.SrcPort, 54021);
    EXPECT_EQ(info.DestPort, 53412);

    uint8_t srcOut[16];
    uint8_t dstOut[16];
    uint8_t * srcPtr = srcOut;
    uint8_t * dstPtr = dstOut;
    info.SrcAddress.WriteAddress(srcPtr);
    info.DestAddress.WriteAddress(dstPtr);
    EXPECT_EQ(0, std::memcmp(srcOut, kSrc, 16));
    EXPECT_EQ(0, std::memcmp(dstOut, kDst, 16));
}

TEST(IcmpV6Parser, RejectsWrongInnerProtocol)
{
    auto inner = MakeInner(kSrc, kDst, /*nextHdr*/ 58 /* ICMPv6 */, 1234, 5678, 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), inner.bytes.size(), info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsTruncatedIPv6)
{
    auto inner = MakeInner(kSrc, kDst, 17, 1, 2, 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), 30, info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsTruncatedUDP)
{
    auto inner = MakeInner(kSrc, kDst, 17, 1, 2, 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), 44, info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsZeroLength)
{
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(nullptr, 0, info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsBadUDPLengthField)
{
    auto inner = MakeInner(kSrc, kDst, 17, 1, 2, /*udpLen*/ 4 /* < 8 */);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), inner.bytes.size(), info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsBadIPVersion)
{
    auto inner     = MakeInner(kSrc, kDst, 17, 1, 2, 8);
    inner.bytes[0] = 0x40; // version=4
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), inner.bytes.size(), info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, RejectsOneByteUnderTheMinimum)
{
    auto inner = MakeInner(kSrc, kDst, 17, 1, 2, 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), kMinInnerDatagramLen - 1, info), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(IcmpV6Parser, AcceptsExactlyTheMinimum)
{
    auto inner = MakeInner(kSrc, kDst, 17, 4242, 5540, 8);
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(inner.bytes.data(), kMinInnerDatagramLen, info), CHIP_NO_ERROR);
    EXPECT_EQ(info.SrcPort, 4242);
    EXPECT_EQ(info.DestPort, 5540);
}

TEST(IcmpV6Parser, IgnoresTrailingPayload)
{
    // A real error carries as much of the datagram as fits the MTU; only the headers are parsed.
    auto inner = MakeInner(kSrc, kDst, 17, 4242, 5540, 40);
    std::array<uint8_t, 240> padded{};
    std::memcpy(padded.data(), inner.bytes.data(), inner.bytes.size());
    IPPacketInfo info{};
    EXPECT_EQ(ParseIcmpV6PortUnreachInnerDatagram(padded.data(), padded.size(), info), CHIP_NO_ERROR);
    EXPECT_EQ(info.SrcPort, 4242);
    EXPECT_EQ(info.DestPort, 5540);
}

} // namespace
