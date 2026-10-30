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

#include <stdint.h>

#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  A kind of change to the IP addresses of the system's network
 *  interfaces, as the kernel reports it in rtnetlink address messages.
 */
enum class InterfaceAddressChange : uint8_t
{
    kIPv4Assigned = 0x01, ///< An IPv4 address was added or updated.
    kIPv4Lost     = 0x02, ///< An IPv4 address was removed.
    kIPv6Assigned = 0x04, ///< An IPv6 address was added or updated, and is usable.
    kIPv6Lost     = 0x08, ///< An IPv6 address was removed.
};

/**
 *  Returns the kinds of address change that a datagram read from an
 *  rtnetlink socket reports.
 *
 *  Only RTM_NEWADDR and RTM_DELADDR messages report a change; any other
 *  message is skipped. So is a message:
 *
 *  - For an address of host scope, such as a loopback address, which is
 *    reachable from nowhere else.
 *  - For an IPv4 or IPv6 address being added that is still tentative,
 *    or that failed duplicate address detection, and so cannot yet be
 *    used. The kernel reports the address again once duplicate address
 *    detection succeeds, and that report counts as the address being
 *    assigned. The flags are read from the IFA_FLAGS attribute, which
 *    carries all 32 of them, when the message has one, and otherwise
 *    from the 8 in the message header. An address undergoing optimistic
 *    duplicate address detection (RFC 4429), which the kernel reports
 *    with IFA_F_OPTIMISTIC and IFA_F_TENTATIVE both set, counts as
 *    tentative, as it does for Avahi.
 *  - Of any address family other than IPv4 or IPv6.
 *
 *  The datagram is read up to an NLMSG_DONE message, or up to the first
 *  message that is malformed or truncated. The datagram need not be
 *  aligned in memory.
 *
 *  @param[in]  datagram  The datagram, as read from the socket.
 *
 *  @returns
 *    The kinds of change reported, each at most once however many
 *    messages report it, or none if the datagram reports no change.
 */
BitFlags<InterfaceAddressChange> GetInterfaceAddressChanges(ByteSpan datagram);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
