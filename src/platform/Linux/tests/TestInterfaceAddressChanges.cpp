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

#include <stdint.h>
#include <string.h>

#include <optional>
#include <vector>

#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

#include <platform/Linux/InterfaceAddressChanges.h>

namespace {

using chip::BitFlags;
using chip::ByteSpan;
using chip::DeviceLayer::Internal::GetInterfaceAddressChanges;
using chip::DeviceLayer::Internal::InterfaceAddressChange;

using Changes = BitFlags<InterfaceAddressChange>;

/// Builds an rtnetlink datagram, one message at a time, laid out as the
/// kernel lays it out.
class Datagram
{
public:
    /// Appends a message of the given type with the given payload.
    Datagram & AddMessage(uint16_t type, const std::vector<uint8_t> & payload)
    {
        struct nlmsghdr header = {};

        header.nlmsg_len  = static_cast<uint32_t>(NLMSG_LENGTH(payload.size()));
        header.nlmsg_type = type;

        Append(&header, sizeof(header));
        Append(payload.data(), payload.size());
        Pad(NLMSG_ALIGNTO);

        return *this;
    }

    /// Appends an address message, with an IFA_ADDRESS attribute and,
    /// when given, an IFA_FLAGS attribute.
    Datagram & AddAddress(uint16_t type, uint8_t family, uint8_t headerFlags, uint8_t scope = RT_SCOPE_UNIVERSE,
                          std::optional<uint32_t> attributeFlags = std::nullopt)
    {
        return AddMessage(type, AddressPayload(family, headerFlags, scope, attributeFlags));
    }

    /// Appends raw bytes, for building malformed datagrams.
    Datagram & AddBytes(const void * bytes, size_t length)
    {
        Append(bytes, length);

        return *this;
    }

    Changes Parse() const { return GetInterfaceAddressChanges(ByteSpan(mBytes.data(), mBytes.size())); }

    static std::vector<uint8_t> AddressPayload(uint8_t family, uint8_t headerFlags, uint8_t scope,
                                               std::optional<uint32_t> attributeFlags)
    {
        std::vector<uint8_t> payload;
        struct ifaddrmsg message = {};

        message.ifa_family    = family;
        message.ifa_prefixlen = (family == AF_INET6) ? 64 : 24;
        message.ifa_flags     = headerFlags;
        message.ifa_scope     = scope;
        message.ifa_index     = 2;

        AppendTo(payload, &message, sizeof(message));

        const uint8_t address[16] = { 0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0x02, 0x04, 0xf3, 0xff, 0xfe, 0x87, 0xeb, 0x94 };

        AppendAttribute(payload, IFA_ADDRESS, address, (family == AF_INET6) ? sizeof(address) : 4);

        if (attributeFlags.has_value())
        {
            const uint32_t flags = attributeFlags.value();

            AppendAttribute(payload, IFA_FLAGS, &flags, sizeof(flags));
        }

        return payload;
    }

    std::vector<uint8_t> mBytes;

private:
    static void AppendTo(std::vector<uint8_t> & bytes, const void * data, size_t length)
    {
        const uint8_t * first = static_cast<const uint8_t *>(data);

        bytes.insert(bytes.end(), first, first + length);
    }

    static void AppendAttribute(std::vector<uint8_t> & bytes, uint16_t type, const void * data, size_t length)
    {
        struct rtattr attribute = {};

        attribute.rta_len  = static_cast<uint16_t>(RTA_LENGTH(length));
        attribute.rta_type = type;

        AppendTo(bytes, &attribute, sizeof(attribute));
        AppendTo(bytes, data, length);
        bytes.resize(RTA_ALIGN(bytes.size()), 0);
    }

    void Append(const void * data, size_t length) { AppendTo(mBytes, data, length); }

    void Pad(size_t alignment) { mBytes.resize((mBytes.size() + alignment - 1) & ~(alignment - 1), 0); }
};

TEST(TestInterfaceAddressChanges, EmptyDatagramReportsNothing)
{
    EXPECT_FALSE(Datagram().Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, NewIPv4AddressIsAssigned)
{
    const Changes changes = Datagram().AddAddress(RTM_NEWADDR, AF_INET, 0).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, DeletedIPv4AddressIsLost)
{
    const Changes changes = Datagram().AddAddress(RTM_DELADDR, AF_INET, 0).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Lost).Raw());
}

TEST(TestInterfaceAddressChanges, NewIPv6AddressIsAssigned)
{
    const Changes changes = Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_PERMANENT, RT_SCOPE_LINK).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, DeletedIPv6AddressIsLost)
{
    const Changes changes = Datagram().AddAddress(RTM_DELADDR, AF_INET6, IFA_F_PERMANENT, RT_SCOPE_LINK).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Lost).Raw());
}

TEST(TestInterfaceAddressChanges, OptimisticIPv6AddressIsAssigned)
{
    // An optimistic address (RFC 4429) is usable while duplicate address
    // detection runs.

    const Changes changes = Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_OPTIMISTIC, RT_SCOPE_LINK).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, TentativeIPv6AddressIsSkipped)
{
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_TENTATIVE, RT_SCOPE_LINK).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, DuplicateIPv6AddressIsSkipped)
{
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_TENTATIVE | IFA_F_DADFAILED, RT_SCOPE_LINK).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_DADFAILED, RT_SCOPE_LINK).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, TentativeIPv6AddressThenUsableIsAssigned)
{
    // The kernel reports a new address once while duplicate address
    // detection runs, and again once it succeeds.

    const Changes changes = Datagram()
                                .AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_TENTATIVE, RT_SCOPE_LINK)
                                .AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK)
                                .Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, DeletedTentativeIPv6AddressIsLost)
{
    const Changes changes = Datagram().AddAddress(RTM_DELADDR, AF_INET6, IFA_F_TENTATIVE, RT_SCOPE_LINK).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Lost).Raw());
}

TEST(TestInterfaceAddressChanges, AttributeFlagsOverrideHeaderFlags)
{
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK, IFA_F_TENTATIVE).Parse().HasAny());

    EXPECT_EQ(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, IFA_F_TENTATIVE, RT_SCOPE_LINK, IFA_F_MANAGETEMPADDR).Parse().Raw(),
              Changes(InterfaceAddressChange::kIPv6Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, MalformedAttributeLeavesHeaderFlags)
{
    std::vector<uint8_t> payload = Datagram::AddressPayload(AF_INET6, IFA_F_TENTATIVE, RT_SCOPE_LINK, std::nullopt);
    struct rtattr attribute      = {};

    // An attribute shorter than its own header ends the attributes.

    attribute.rta_len  = 2;
    attribute.rta_type = IFA_FLAGS;
    payload.insert(payload.end(), reinterpret_cast<const uint8_t *>(&attribute),
                   reinterpret_cast<const uint8_t *>(&attribute) + sizeof(attribute));

    EXPECT_FALSE(Datagram().AddMessage(RTM_NEWADDR, payload).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, HostScopeAddressIsSkipped)
{
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET, 0, RT_SCOPE_HOST).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_HOST).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddAddress(RTM_DELADDR, AF_INET6, 0, RT_SCOPE_HOST).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, OtherAddressFamilyIsSkipped)
{
    EXPECT_FALSE(Datagram().AddAddress(RTM_NEWADDR, AF_UNSPEC, 0).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddAddress(RTM_DELADDR, AF_PACKET, 0).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, OtherMessageTypesAreSkipped)
{
    const std::vector<uint8_t> payload = Datagram::AddressPayload(AF_INET6, 0, RT_SCOPE_LINK, std::nullopt);

    EXPECT_FALSE(Datagram().AddMessage(RTM_NEWLINK, payload).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddMessage(RTM_NEWROUTE, payload).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddMessage(NLMSG_NOOP, payload).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddMessage(NLMSG_ERROR, payload).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, ShortAddressMessageIsSkipped)
{
    const std::vector<uint8_t> payload(sizeof(struct ifaddrmsg) - 1, 0);

    EXPECT_FALSE(Datagram().AddMessage(RTM_NEWADDR, payload).Parse().HasAny());
    EXPECT_FALSE(Datagram().AddMessage(RTM_DELADDR, payload).Parse().HasAny());
}

TEST(TestInterfaceAddressChanges, EveryMessageInADatagramIsRead)
{
    const Changes changes = Datagram()
                                .AddAddress(RTM_NEWADDR, AF_INET, 0)
                                .AddAddress(RTM_NEWADDR, AF_INET, 0)
                                .AddMessage(RTM_NEWLINK, {})
                                .AddAddress(RTM_DELADDR, AF_INET6, 0, RT_SCOPE_LINK)
                                .Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Assigned, InterfaceAddressChange::kIPv6Lost).Raw());
}

TEST(TestInterfaceAddressChanges, ReadingStopsAtDone)
{
    const Changes changes = Datagram()
                                .AddAddress(RTM_NEWADDR, AF_INET, 0)
                                .AddMessage(NLMSG_DONE, {})
                                .AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK)
                                .Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, ReadingStopsAtTruncatedMessage)
{
    Datagram datagram;

    datagram.AddAddress(RTM_NEWADDR, AF_INET, 0).AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK);

    // Drop the last byte, so that the second message is longer than what
    // remains of the datagram.

    datagram.mBytes.pop_back();

    EXPECT_EQ(datagram.Parse().Raw(), Changes(InterfaceAddressChange::kIPv4Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, ReadingStopsAtMalformedLength)
{
    struct nlmsghdr header = {};

    header.nlmsg_len  = sizeof(header) - 1;
    header.nlmsg_type = RTM_NEWADDR;

    const Changes changes = Datagram()
                                .AddAddress(RTM_DELADDR, AF_INET, 0)
                                .AddBytes(&header, sizeof(header))
                                .AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK)
                                .Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Lost).Raw());
}

TEST(TestInterfaceAddressChanges, ShortTrailerIsIgnored)
{
    const uint8_t trailer[sizeof(struct nlmsghdr) - 1] = {};

    const Changes changes = Datagram().AddAddress(RTM_NEWADDR, AF_INET, 0).AddBytes(trailer, sizeof(trailer)).Parse();

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv4Assigned).Raw());
}

TEST(TestInterfaceAddressChanges, UnalignedDatagramIsRead)
{
    Datagram datagram;

    datagram.AddAddress(RTM_NEWADDR, AF_INET6, 0, RT_SCOPE_LINK, 0).AddAddress(RTM_DELADDR, AF_INET, 0);

    std::vector<uint8_t> unaligned(datagram.mBytes.size() + 1);

    memcpy(unaligned.data() + 1, datagram.mBytes.data(), datagram.mBytes.size());

    const Changes changes = GetInterfaceAddressChanges(ByteSpan(unaligned.data() + 1, datagram.mBytes.size()));

    EXPECT_EQ(changes.Raw(), Changes(InterfaceAddressChange::kIPv6Assigned, InterfaceAddressChange::kIPv4Lost).Raw());
}

} // namespace
