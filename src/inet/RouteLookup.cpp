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

#include <inet/RouteLookup.h>

#include <inet/InetConfig.h>

#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <errno.h>
#include <lib/support/FileDescriptor.h>
#include <sys/socket.h>
#endif

namespace chip {
namespace Inet {

#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
namespace {

constexpr uint16_t kAnyPort = 9; // connect() needs a port, but nothing is sent to it.

RouteLookupResult LookUpRouteToSockAddr(const SockAddr & remote, socklen_t remoteLength)
{
    FileDescriptor socketFd(socket(remote.any.sa_family, SOCK_DGRAM, 0));
    if (socketFd.Get() < 0)
    {
        // A host that doesn't support the address family has no route to any address in it.
        return (errno == EAFNOSUPPORT) ? RouteLookupResult::Create<NoRoute>() : RouteLookupResult::Create<UnknownRoute>();
    }

    // connect() on a datagram socket sends nothing. It picks the local address the host would send from, and fails with
    // ENETUNREACH or EHOSTUNREACH when there is no route; any other failure says nothing about routing.
    if (connect(socketFd.Get(), &remote.any, remoteLength) != 0)
    {
        return (errno == ENETUNREACH || errno == EHOSTUNREACH) ? RouteLookupResult::Create<NoRoute>()
                                                               : RouteLookupResult::Create<UnknownRoute>();
    }

    SockAddr local        = {};
    socklen_t localLength = sizeof(local);
    IPAddress localAddress;
    if (getsockname(socketFd.Get(), &local.any, &localLength) != 0 ||
        IPAddress::GetIPAddressFromSockAddr(local.any, localAddress) != CHIP_NO_ERROR)
    {
        return RouteLookupResult::Create<UnknownRoute>();
    }

    // A link-local IPv6 address is only unique on its link, so only the socket knows its interface.
    const InterfaceId localInterfaceId = (local.any.sa_family == AF_INET6 && local.in6.sin6_scope_id != 0)
        ? InterfaceId(local.in6.sin6_scope_id)
        : InterfaceId::FromIPAddress(localAddress);
    return RouteLookupResult::Create<RouteFromLocalAddress>(RouteFromLocalAddress{ localAddress, localInterfaceId });
}

} // namespace
#endif

RouteLookupResult LookUpRouteTo(const IPAddress & destination, InterfaceId interfaceId)
{
#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    SockAddr remote = {};
    socklen_t remoteLength;
    if (destination.IsIPv6())
    {
        // A link-local destination only means something on a given interface.
        if (destination.IsIPv6LinkLocal())
        {
            if (!interfaceId.IsPresent())
            {
                return RouteLookupResult::Create<UnknownRoute>();
            }
            remote.in6.sin6_scope_id = interfaceId.GetPlatformInterface();
        }
        remote.in6.sin6_family = AF_INET6;
        remote.in6.sin6_port   = htons(kAnyPort);
        remote.in6.sin6_addr   = destination.ToIPv6();
        remoteLength           = sizeof(remote.in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (destination.IsIPv4())
    {
        remote.in.sin_family = AF_INET;
        remote.in.sin_port   = htons(kAnyPort);
        remote.in.sin_addr   = destination.ToIPv4();
        remoteLength         = sizeof(remote.in);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        return RouteLookupResult::Create<UnknownRoute>();
    }

    return LookUpRouteToSockAddr(remote, remoteLength);
#else
    return RouteLookupResult::Create<UnknownRoute>();
#endif
}

} // namespace Inet
} // namespace chip
