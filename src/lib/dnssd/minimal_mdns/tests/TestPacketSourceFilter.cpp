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

#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <inet/IPPacketInfo.h>
#include <inet/InetConfig.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/minimal_mdns/PacketSourceFilter.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace mdns::Minimal;

struct FakeInterfaceAddress
{
    InterfaceId interfaceId;
    const char * address;
    uint8_t prefixLength;
};

/// Iterates a fixed list of addresses with the subset of the
/// InterfaceAddressIterator API used by IsFromAttachedSubnet.
class FakeAddressIterator
{
public:
    explicit FakeAddressIterator(Span<const FakeInterfaceAddress> addresses) : mAddresses(addresses) {}

    bool HasCurrent() const { return mIndex < mAddresses.size(); }
    bool Next()
    {
        mIndex++;
        return HasCurrent();
    }
    CHIP_ERROR GetAddress(IPAddress & outAddress) const
    {
        return IPAddress::FromString(mAddresses[mIndex].address, outAddress) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t GetPrefixLength() const { return mAddresses[mIndex].prefixLength; }
    InterfaceId GetInterfaceId() const { return mAddresses[mIndex].interfaceId; }

    size_t VisitedCount() const { return mIndex; }

private:
    Span<const FakeInterfaceAddress> mAddresses;
    size_t mIndex = 0;
};

const InterfaceId kInterfaceA(1u);
const InterfaceId kInterfaceB(2u);

IPPacketInfo MakePacketInfo(const char * source, const char * destination, InterfaceId interfaceId)
{
    IPPacketInfo info;
    info.Clear();
    EXPECT_TRUE(IPAddress::FromString(source, info.SrcAddress));
    EXPECT_TRUE(IPAddress::FromString(destination, info.DestAddress));
    info.Interface = interfaceId;
    info.SrcPort   = 5353;
    info.DestPort  = 5353;
    return info;
}

const FakeInterfaceAddress kIPv6Addresses[] = {
    { kInterfaceA, "fe80::1", 64 },
    { kInterfaceA, "2001:db8:1::10", 64 },
    { kInterfaceB, "2001:db8:2::10", 64 },
};

TEST(TestPacketSourceFilter, AcceptsMdnsGroupDestinationWithoutCheckingSubnets)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "ff02::fb", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
    EXPECT_EQ(addresses.VisitedCount(), 0u);
}

// Groups other than ff02::fb can be routed (ff05::/16 is site-scoped), so the
// destination alone does not show that the sender is on the local link.
TEST(TestPacketSourceFilter, RejectsOffSubnetSourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "ff05::fb", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsOnSubnetSourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "ff05::fb", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv6LinkLocalSourceWithoutCheckingSubnets)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("fe80::abcd", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
    EXPECT_EQ(addresses.VisitedCount(), 0u);
}

TEST(TestPacketSourceFilter, AcceptsIPv6SourceInReceivingInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsIPv6SourceOutsideInterfaceSubnets)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsSourceInSubnetOfAnotherInterface)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:2::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, ChecksAllInterfacesWhenReceivingInterfaceIsUnknown)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:2::20", "2001:db8:1::10", InterfaceId::Null());

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, IgnoresZeroLengthPrefix)
{
    const FakeInterfaceAddress zeroPrefix[] = {
        { kInterfaceA, "2001:db8:1::10", 0 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(zeroPrefix) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsWhenInterfaceHasNoAddresses)
{
    FakeAddressIterator addresses(Span<const FakeInterfaceAddress>{});
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

#if INET_CONFIG_ENABLE_IPV4

const FakeInterfaceAddress kIPv4Addresses[] = {
    { kInterfaceA, "192.168.1.10", 24 },
    { kInterfaceA, "2001:db8:1::10", 64 },
};

TEST(TestPacketSourceFilter, AcceptsIPv4MdnsGroupDestination)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo info = MakePacketInfo("198.51.100.7", "224.0.0.251", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
}

// 239.0.0.0/8 is administratively scoped and can be routed.
TEST(TestPacketSourceFilter, RejectsOffSubnetIPv4SourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo info = MakePacketInfo("198.51.100.7", "239.1.2.3", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv4SourceInInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo info = MakePacketInfo("192.168.1.200", "192.168.1.10", kInterfaceA);

    EXPECT_TRUE(IsFromAttachedSubnet(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsIPv4SourceOutsideInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo neighbourSubnet = MakePacketInfo("192.168.2.1", "192.168.1.10", kInterfaceA);
    IPPacketInfo remote          = MakePacketInfo("198.51.100.7", "192.168.1.10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(neighbourSubnet, addresses));

    FakeAddressIterator addressesAgain{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    EXPECT_FALSE(IsFromAttachedSubnet(remote, addressesAgain));
}

TEST(TestPacketSourceFilter, IPv6PrefixDoesNotMatchIPv4Source)
{
    // ::/0 is skipped as zero-length, so use ::/64, which covers the
    // IPv4-mapped form (::ffff:a.b.c.d) of every IPv4 address.
    const FakeInterfaceAddress ipv6Only[] = {
        { kInterfaceA, "::1:2:3:4", 64 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(ipv6Only) };
    IPPacketInfo info = MakePacketInfo("198.51.100.7", "192.168.1.10", kInterfaceA);

    EXPECT_FALSE(IsFromAttachedSubnet(info, addresses));
}

#endif // INET_CONFIG_ENABLE_IPV4

} // namespace
