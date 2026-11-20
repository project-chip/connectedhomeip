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
#include <lib/dnssd/minimal_mdns/Server.h>
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

bool IsAccepted(const IPPacketInfo & info, FakeAddressIterator & addresses)
{
    return IsLinkScoped(info, BroadcastIpAddresses::Get(info.DestAddress.Type())) || IsFromAttachedSubnet(info, addresses);
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

    EXPECT_TRUE(IsAccepted(info, addresses));
    EXPECT_EQ(addresses.VisitedCount(), 0u);
}

// Groups other than ff02::fb can be routed (ff05::/16 is site-scoped), so the
// destination alone does not show that the sender is on the local link.
TEST(TestPacketSourceFilter, RejectsOffSubnetSourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "ff05::fb", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsOnSubnetSourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "ff05::fb", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv6LinkLocalSourceWithoutCheckingSubnets)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("fe80::abcd", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
    EXPECT_EQ(addresses.VisitedCount(), 0u);
}

TEST(TestPacketSourceFilter, AcceptsIPv6SourceInReceivingInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsIPv6SourceOutsideInterfaceSubnets)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsSourceInSubnetOfAnotherInterface)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:2::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, ChecksAllInterfacesWhenReceivingInterfaceIsUnknown)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("2001:db8:2::20", "2001:db8:1::10", InterfaceId::Null());

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, IgnoresZeroAndFullLengthPrefixes)
{
    const FakeInterfaceAddress addressList[] = {
        { kInterfaceA, "2001:db8:1::10", 0 },
        { kInterfaceA, "2001:db8:1::11", 128 },
        { kInterfaceA, "2001:db8:2::10", 64 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(addressList) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

// Without a subnet to compare against, the source cannot be checked, so the
// packet is accepted rather than breaking discovery.
TEST(TestPacketSourceFilter, AcceptsWhenInterfaceHasNoAddresses)
{
    FakeAddressIterator addresses(Span<const FakeInterfaceAddress>{});
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsWhenOnlyFullLengthPrefixesAreReported)
{
    const FakeInterfaceAddress hostRoutes[] = {
        { kInterfaceA, "2001:db8:1::10", 128 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(hostRoutes) };
    IPPacketInfo info = MakePacketInfo("2001:db8:1::20", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsWhenOnlyAnotherInterfaceHasASubnet)
{
    const FakeInterfaceAddress otherInterface[] = {
        { kInterfaceB, "2001:db8:2::10", 64 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(otherInterface) };
    IPPacketInfo info = MakePacketInfo("2001:db8:ffff::1", "2001:db8:1::10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, ChecksSourceWhenDestinationIsUnknown)
{
    IPPacketInfo onSubnet  = MakePacketInfo("2001:db8:1::20", "::", kInterfaceA);
    IPPacketInfo offSubnet = MakePacketInfo("2001:db8:ffff::1", "::", kInterfaceA);

    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    EXPECT_TRUE(IsAccepted(onSubnet, addresses));

    FakeAddressIterator addressesAgain{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    EXPECT_FALSE(IsAccepted(offSubnet, addressesAgain));
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

    EXPECT_TRUE(IsAccepted(info, addresses));
}

// 239.0.0.0/8 is administratively scoped and can be routed.
TEST(TestPacketSourceFilter, RejectsOffSubnetIPv4SourceSentToOtherMulticastGroup)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo info = MakePacketInfo("198.51.100.7", "239.1.2.3", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv4SourceInInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo info = MakePacketInfo("192.168.1.200", "192.168.1.10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, RejectsIPv4SourceOutsideInterfaceSubnet)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    IPPacketInfo neighbourSubnet = MakePacketInfo("192.168.2.1", "192.168.1.10", kInterfaceA);
    IPPacketInfo remote          = MakePacketInfo("198.51.100.7", "192.168.1.10", kInterfaceA);

    EXPECT_FALSE(IsAccepted(neighbourSubnet, addresses));

    FakeAddressIterator addressesAgain{ Span<const FakeInterfaceAddress>(kIPv4Addresses) };
    EXPECT_FALSE(IsAccepted(remote, addressesAgain));
}

TEST(TestPacketSourceFilter, IPv6PrefixDoesNotMatchIPv4Source)
{
    // ::/0 is skipped as zero-length, so use ::/64, which covers the
    // IPv4-mapped form (::ffff:a.b.c.d) of every IPv4 address.
    const FakeInterfaceAddress addressList[] = {
        { kInterfaceA, "::1:2:3:4", 64 },
        { kInterfaceA, "192.168.1.10", 24 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(addressList) };
    IPPacketInfo info = MakePacketInfo("198.51.100.7", "192.168.1.10", kInterfaceA);

    EXPECT_FALSE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv4SourceWhenNoIPv4SubnetIsReported)
{
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(kIPv6Addresses) };
    IPPacketInfo info = MakePacketInfo("192.168.1.200", "192.168.1.10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

TEST(TestPacketSourceFilter, AcceptsIPv4SourceWhenOnlyFullLengthPrefixIsReported)
{
    const FakeInterfaceAddress hostRoute[] = {
        { kInterfaceA, "192.168.1.10", 32 },
    };
    FakeAddressIterator addresses{ Span<const FakeInterfaceAddress>(hostRoute) };
    IPPacketInfo info = MakePacketInfo("192.168.1.200", "192.168.1.10", kInterfaceA);

    EXPECT_TRUE(IsAccepted(info, addresses));
}

#endif // INET_CONFIG_ENABLE_IPV4

} // namespace
