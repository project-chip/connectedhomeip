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

#include "InterfaceAddressChanges.h"

#include <string.h>

#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// The sizes and alignment the rtnetlink macros use, as size_t, so that
// the arithmetic below mixes no signed and unsigned operands.

constexpr size_t kMessageHeaderLength   = NLMSG_HDRLEN;
constexpr size_t kAddressMessageLength  = NLMSG_ALIGN(sizeof(struct ifaddrmsg));
constexpr size_t kAttributeHeaderLength = RTA_LENGTH(0);

constexpr size_t AlignMessage(size_t length)
{
    return NLMSG_ALIGN(length);
}

constexpr size_t AlignAttribute(size_t length)
{
    return RTA_ALIGN(length);
}

/**
 *  Returns the flags of an address message: those of its IFA_FLAGS
 *  attribute when it has one, which carries all 32 of them, and
 *  otherwise the 8 in its header.
 *
 *  @param[in]  message     The address message header.
 *  @param[in]  attributes  The attributes that follow the header.
 */
uint32_t GetAddressFlags(const struct ifaddrmsg & message, ByteSpan attributes)
{
    const uint8_t * data = attributes.data();
    size_t remaining     = attributes.size();

    while (remaining >= sizeof(struct rtattr))
    {
        struct rtattr attribute;

        memcpy(&attribute, data, sizeof(attribute));

        if ((attribute.rta_len < kAttributeHeaderLength) || (attribute.rta_len > remaining))
        {
            break;
        }

        if ((attribute.rta_type == IFA_FLAGS) && (attribute.rta_len >= kAttributeHeaderLength + sizeof(uint32_t)))
        {
            uint32_t flags;

            memcpy(&flags, data + kAttributeHeaderLength, sizeof(flags));

            return flags;
        }

        const size_t advance = AlignAttribute(attribute.rta_len);

        if (advance >= remaining)
        {
            break;
        }

        data += advance;
        remaining -= advance;
    }

    return message.ifa_flags;
}

/**
 *  Adds to @a changes the change, if any, that one rtnetlink message
 *  reports.
 *
 *  @param[in]     type     The message type.
 *  @param[in]     payload  The message payload, after its header.
 *  @param[inout]  changes  The changes reported so far.
 */
void AddInterfaceAddressChange(uint16_t type, ByteSpan payload, BitFlags<InterfaceAddressChange> & changes)
{
    if ((type != RTM_NEWADDR) && (type != RTM_DELADDR))
    {
        return;
    }

    if (payload.size() < sizeof(struct ifaddrmsg))
    {
        return;
    }

    struct ifaddrmsg message;

    memcpy(&message, payload.data(), sizeof(message));

    // An address of host scope, such as 127.0.0.1 or ::1, is reachable
    // from nowhere else.

    if (message.ifa_scope == RT_SCOPE_HOST)
    {
        return;
    }

    const bool assigned = (type == RTM_NEWADDR);

    // An address still undergoing duplicate address detection, or one
    // that failed it, cannot be used yet. The kernel reports it again
    // once duplicate address detection succeeds.

    if (assigned)
    {
        const ByteSpan attributes = (payload.size() > kAddressMessageLength) ? payload.SubSpan(kAddressMessageLength) : ByteSpan();

        if ((GetAddressFlags(message, attributes) & (IFA_F_TENTATIVE | IFA_F_DADFAILED)) != 0)
        {
            return;
        }
    }

    switch (message.ifa_family)
    {
    case AF_INET:
        changes.Set(assigned ? InterfaceAddressChange::kIPv4Assigned : InterfaceAddressChange::kIPv4Lost);
        break;

    case AF_INET6:
        changes.Set(assigned ? InterfaceAddressChange::kIPv6Assigned : InterfaceAddressChange::kIPv6Lost);
        break;

    default:
        break;
    }
}

} // namespace

BitFlags<InterfaceAddressChange> GetInterfaceAddressChanges(ByteSpan datagram)
{
    BitFlags<InterfaceAddressChange> changes;
    const uint8_t * data = datagram.data();
    size_t remaining     = datagram.size();

    while (remaining >= sizeof(struct nlmsghdr))
    {
        struct nlmsghdr header;

        memcpy(&header, data, sizeof(header));

        if ((header.nlmsg_len < kMessageHeaderLength) || (header.nlmsg_len > remaining))
        {
            break;
        }

        if (header.nlmsg_type == NLMSG_DONE)
        {
            break;
        }

        AddInterfaceAddressChange(header.nlmsg_type, ByteSpan(data + kMessageHeaderLength, header.nlmsg_len - kMessageHeaderLength),
                                  changes);

        const size_t advance = AlignMessage(header.nlmsg_len);

        if (advance >= remaining)
        {
            break;
        }

        data += advance;
        remaining -= advance;
    }

    return changes;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
