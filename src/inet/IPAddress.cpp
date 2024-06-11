/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements the class <tt>Inet::IPAddress</tt> and
 *      related enumerated constants. The CHIP Inet Layer uses objects
 *      of this class to represent Internet protocol addresses of both
 *      IPv4 and IPv6 address families. (IPv4 addresses are stored
 *      internally as IPv4-Mapped IPv6 addresses.)
 *
 */

#include <inet/IPAddress.h>

#include <inet/InetError.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

#include "arpa-inet-compatibility.h"

#include <stdint.h>
#include <string.h>

namespace chip {
namespace Inet {

IPAddress IPAddress::Any;

bool IPAddress::operator==(const IPAddress & other) const
{
    return Addr[0] == other.Addr[0] && Addr[1] == other.Addr[1] && Addr[2] == other.Addr[2] && Addr[3] == other.Addr[3];
}

bool IPAddress::operator!=(const IPAddress & other) const
{
    return Addr[0] != other.Addr[0] || Addr[1] != other.Addr[1] || Addr[2] != other.Addr[2] || Addr[3] != other.Addr[3];
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

IPAddress::IPAddress(const ip6_addr_t & ipv6Addr)
{
    static_assert(sizeof(ipv6Addr.addr) == sizeof(Addr), "ip6_addr_t size mismatch");
    memcpy(Addr, &ipv6Addr.addr, sizeof(ipv6Addr.addr));
}

#if INET_CONFIG_ENABLE_IPV4 || LWIP_IPV4

IPAddress::IPAddress(const ip4_addr_t & ipv4Addr)
{
    Addr[0] = 0;
    Addr[1] = 0;
    Addr[2] = htonl(0xFFFF);
    Addr[3] = ipv4Addr.addr;
}

IPAddress::IPAddress(const ip_addr_t & addr)
{
    switch (IP_GET_TYPE(&addr))
    {
#if INET_CONFIG_ENABLE_IPV4
    case IPADDR_TYPE_V4:
        *this = IPAddress(*ip_2_ip4(&addr));
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPADDR_TYPE_V6:
        *this = IPAddress(*ip_2_ip6(&addr));
        break;

    default:
        *this = Any;
        break;
    }
}

#endif // INET_CONFIG_ENABLE_IPV4 || LWIP_IPV4

#if INET_CONFIG_ENABLE_IPV4

ip4_addr_t IPAddress::ToIPv4() const
{
    ip4_addr_t ipAddr;
    memcpy(&ipAddr, &Addr[3], sizeof(ipAddr));
    return ipAddr;
}

#endif // INET_CONFIG_ENABLE_IPV4

ip_addr_t IPAddress::ToLwIPAddr() const
{
    ip_addr_t ret;

    switch (Type())
    {
#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        ip_addr_copy_from_ip4(ret, IPAddress::ToIPv4());
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6:
        ip_addr_copy_from_ip6(ret, IPAddress::ToIPv6());
        break;

    default:
        ret = *IP6_ADDR_ANY;
        break;
    }

    return ret;
}

CHIP_ERROR IPAddress::ToLwIPAddr(IPAddressType addressType, ip_addr_t & outAddress) const
{
    VerifyOrReturnError(addressType != IPAddressType::kUnknown, CHIP_ERROR_INVALID_ARGUMENT);

    switch (Type())
    {
#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        ip_addr_copy_from_ip4(outAddress, IPAddress::ToIPv4());
        return (addressType == IPAddressType::kIPv6) ? INET_ERROR_WRONG_ADDRESS_TYPE : CHIP_NO_ERROR;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6:
        ip_addr_copy_from_ip6(outAddress, IPAddress::ToIPv6());
#if INET_CONFIG_ENABLE_IPV4
        return (addressType == IPAddressType::kIPv4) ? INET_ERROR_WRONG_ADDRESS_TYPE : CHIP_NO_ERROR;
#else
        return CHIP_NO_ERROR;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kAny:
#if INET_CONFIG_ENABLE_IPV4
        if (addressType == IPAddressType::kIPv4)
        {
            outAddress = *IP4_ADDR_ANY;
            return CHIP_NO_ERROR;
        }
#endif // INET_CONFIG_ENABLE_IPV4
        outAddress = *IP6_ADDR_ANY;
        return CHIP_NO_ERROR;

    default:
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
}

lwip_ip_addr_type IPAddress::ToLwIPAddrType(IPAddressType typ)
{
    lwip_ip_addr_type ret;

    switch (typ)
    {
#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        ret = IPADDR_TYPE_V4;
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6:
        ret = IPADDR_TYPE_V6;
        break;

    default:
        ret = IPADDR_TYPE_ANY;
        break;
    }

    return ret;
}

ip6_addr_t IPAddress::ToIPv6() const
{
    ip6_addr_t ipAddr = {};
    static_assert(sizeof(ipAddr.addr) == sizeof(Addr), "ip6_addr_t size mismatch");
    memcpy(&ipAddr.addr, Addr, sizeof(ipAddr.addr));
    return ipAddr;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if INET_CONFIG_ENABLE_IPV4
IPAddress::IPAddress(const struct in_addr & ipv4Addr)
{
    Addr[0] = 0;
    Addr[1] = 0;
    Addr[2] = htonl(0xFFFF);
    Addr[3] = ipv4Addr.s_addr;
}
#endif // INET_CONFIG_ENABLE_IPV4

IPAddress::IPAddress(const struct in6_addr & ipv6Addr)
{
    static_assert(sizeof(*this) == sizeof(ipv6Addr), "in6_addr size mismatch");
    memcpy(Addr, &ipv6Addr, sizeof(ipv6Addr));
}

#if INET_CONFIG_ENABLE_IPV4
struct in_addr IPAddress::ToIPv4() const
{
    struct in_addr ipv4Addr;
    ipv4Addr.s_addr = Addr[3];
    return ipv4Addr;
}
#endif // INET_CONFIG_ENABLE_IPV4

struct in6_addr IPAddress::ToIPv6() const
{
    in6_addr ipAddr;
    static_assert(sizeof(ipAddr) == sizeof(Addr), "in6_addr size mismatch");
    memcpy(&ipAddr, Addr, sizeof(ipAddr));
    return ipAddr;
}

CHIP_ERROR IPAddress::GetIPAddressFromSockAddr(const SockAddrWithoutStorage & sockaddr, IPAddress & outIPAddress)
{
#if INET_CONFIG_ENABLE_IPV4
    if (sockaddr.any.sa_family == AF_INET)
    {
        outIPAddress = FromSockAddr(sockaddr.in);
        return CHIP_NO_ERROR;
    }
#endif // INET_CONFIG_ENABLE_IPV4
    if (sockaddr.any.sa_family == AF_INET6)
    {
        outIPAddress = FromSockAddr(sockaddr.in6);
        return CHIP_NO_ERROR;
    }
    return INET_ERROR_WRONG_ADDRESS_TYPE;
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
IPAddress::IPAddress(const otIp6Address & ipv6Addr)
{
    static_assert(sizeof(ipv6Addr.mFields.m32) == sizeof(Addr), "otIp6Address size mismatch");
    memcpy(Addr, ipv6Addr.mFields.m32, sizeof(Addr));
}
otIp6Address IPAddress::ToIPv6() const
{
    otIp6Address otAddr;
    static_assert(sizeof(otAddr.mFields.m32) == sizeof(Addr), "otIp6Address size mismatch");
    memcpy(otAddr.mFields.m32, Addr, sizeof(otAddr.mFields.m32));
    return otAddr;
}

IPAddress IPAddress::FromOtAddr(const otIp6Address & address)
{
    IPAddress addr;
    static_assert(sizeof(address.mFields.m32) == sizeof(addr), "otIp6Address size mismatch");
    memcpy(addr.Addr, address.mFields.m32, sizeof(addr.Addr));
    return addr;
}
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

// Is address an IPv4 address encoded in IPv6 format?
bool IPAddress::IsIPv4() const
{
    return Addr[0] == 0 && Addr[1] == 0 && Addr[2] == htonl(0xFFFF);
}

// Is address a IPv4 multicast address?
bool IPAddress::IsIPv4Multicast() const
{
    return (IsIPv4() && ((ntohl(Addr[3]) & 0xF0000000U) == 0xE0000000U));
}

// Is address the IPv4 broadcast address?
bool IPAddress::IsIPv4Broadcast() const
{
    return (IsIPv4() && (Addr[3] == 0xFFFFFFFFU));
}

// Is address an IPv4 or IPv6 multicast address?
bool IPAddress::IsMulticast() const
{
    return (IsIPv6Multicast() || IsIPv4Multicast());
}

bool IPAddress::IsIPv6() const
{
    return *this != Any && !IsIPv4();
}

// Is address an IPv6 multicast address?
bool IPAddress::IsIPv6Multicast() const
{
    return (ntohl(Addr[0]) & 0xFF000000U) == 0xFF000000U;
}

// Is address an IPv6 Global Unicast Address?
bool IPAddress::IsIPv6GlobalUnicast() const
{
    return (ntohl(Addr[0]) & 0xE0000000U) == 0x20000000U;
}

// Is address an IPv6 Unique Local Address?
bool IPAddress::IsIPv6ULA() const
{
    return (ntohl(Addr[0]) & 0xFE000000U) == 0xFC000000U;
}

// Is address an IPv6 Link-local Address?
bool IPAddress::IsIPv6LinkLocal() const
{
    return (Addr[0] == htonl(0xFE800000U) && Addr[1] == 0);
}

// Extract the interface id from a IPv6 ULA address.  Returns 0 if the address
// is not a ULA.
uint64_t IPAddress::InterfaceId() const
{
    if (IsIPv6ULA())
        return ((static_cast<uint64_t>(ntohl(Addr[2]))) << 32) | (static_cast<uint64_t>(ntohl(Addr[3])));
    return 0;
}

// Extract the subnet id from a IPv6 ULA address.  Returns 0 if the address
// is not a ULA.
uint16_t IPAddress::Subnet() const
{
    if (IsIPv6ULA())
        return static_cast<uint16_t>(ntohl(Addr[1]));
    return 0;
}

// Extract the global id from a IPv6 ULA address.  Returns 0 if the address
// is not a ULA.
uint64_t IPAddress::GlobalId() const
{
    if (IsIPv6ULA())
        return ((static_cast<uint64_t>(ntohl(Addr[0]) & 0xFFFFFF)) << 16) |
            (static_cast<uint64_t>(ntohl(Addr[1])) & 0xFFFF0000) >> 16;
    return 0;
}

IPAddressType IPAddress::Type() const
{
    if (Addr[0] == 0 && Addr[1] == 0 && Addr[2] == 0 && Addr[3] == 0)
        return IPAddressType::kAny;
#if INET_CONFIG_ENABLE_IPV4
    if (Addr[0] == 0 && Addr[1] == 0 && Addr[2] == htonl(0xFFFF))
        return IPAddressType::kIPv4;
#endif // INET_CONFIG_ENABLE_IPV4
    return IPAddressType::kIPv6;
}

// Encode IPAddress to buffer in network byte order. Buffer must have at least 128 bits of available space.
// Decoder must infer IP address type from context.
void IPAddress::WriteAddress(uint8_t *& p) const
{
    // Since each of the 32bit values in the Addr array is in network byte order, a simple
    // memcpy of the entire array is sufficient while copying the address.

    memcpy(p, &Addr[0], NL_INET_IPV6_ADDR_LEN_IN_BYTES);

    p += NL_INET_IPV6_ADDR_LEN_IN_BYTES;
}

// Decode IPAddress from buffer in network byte order. Must infer IP address type from context.
void IPAddress::ReadAddress(const uint8_t *& p, IPAddress & output)
{
    // Since we want to store the address in the output array in network byte order, a simple
    // memcpy of the entire array is used to retrieve from the buffer.

    memcpy(&output.Addr[0], p, NL_INET_IPV6_ADDR_LEN_IN_BYTES);

    p += NL_INET_IPV6_ADDR_LEN_IN_BYTES;
}

// Construct an IPv6 unique local address.
IPAddress IPAddress::MakeULA(uint64_t globalId, uint16_t subnet, uint64_t interfaceId)
{
    IPAddress addr;

    addr.Addr[0] = 0xFD000000 | static_cast<uint32_t>((globalId & 0xFFFFFF0000ULL) >> 16);
    addr.Addr[0] = htonl(addr.Addr[0]);

    addr.Addr[1] = static_cast<uint32_t>((globalId & 0x000000FFFFULL) << 16) | subnet;
    addr.Addr[1] = htonl(addr.Addr[1]);

    addr.Addr[2] = htonl(static_cast<uint32_t>(interfaceId >> 32));
    addr.Addr[3] = htonl(static_cast<uint32_t>(interfaceId));

    return addr;
}

IPAddress IPAddress::MakeLLA(uint64_t interfaceId)
{
    IPAddress addr;

    addr.Addr[0] = htonl(0xFE800000);
    addr.Addr[1] = 0;

    addr.Addr[2] = htonl(static_cast<uint32_t>(interfaceId >> 32));
    addr.Addr[3] = htonl(static_cast<uint32_t>(interfaceId));

    return addr;
}

IPAddress IPAddress::MakeIPv6Multicast(IPv6MulticastFlags aFlags, uint8_t aScope,
                                       const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES])
{
    const uint32_t lFlagsAndScope =
        (((static_cast<uint32_t>(aFlags.Raw()) & 0xF) << 20) | ((static_cast<uint32_t>(aScope) & 0xF) << 16));
    IPAddress addr;

    addr.Addr[0] = htonl((0xFF000000U | lFlagsAndScope) | (uint32_t(aGroupId[0]) << 8) | (uint32_t(aGroupId[1]) << 0));
    addr.Addr[1] = htonl((uint32_t(aGroupId[2]) << 24) | (uint32_t(aGroupId[3]) << 16) | (uint32_t(aGroupId[4]) << 8) |
                         (uint32_t(aGroupId[5]) << 0));
    addr.Addr[2] = htonl((uint32_t(aGroupId[6]) << 24) | (uint32_t(aGroupId[7]) << 16) | (uint32_t(aGroupId[8]) << 8) |
                         (uint32_t(aGroupId[9]) << 0));
    addr.Addr[3] = htonl((uint32_t(aGroupId[10]) << 24) | (uint32_t(aGroupId[11]) << 16) | (uint32_t(aGroupId[12]) << 8) |
                         (uint32_t(aGroupId[13]) << 0));

    return addr;
}

IPAddress IPAddress::MakeIPv6Multicast(IPv6MulticastFlags aFlags, uint8_t aScope, uint32_t aGroupId)
{
    const uint8_t lGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] = { 0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      static_cast<uint8_t>((aGroupId & 0xFF000000U) >> 24),
                                                                      static_cast<uint8_t>((aGroupId & 0x00FF0000U) >> 16),
                                                                      static_cast<uint8_t>((aGroupId & 0x0000FF00U) >> 8),
                                                                      static_cast<uint8_t>((aGroupId & 0x000000FFU) >> 0) };

    return (MakeIPv6Multicast(aFlags, aScope, lGroupId));
}

IPAddress IPAddress::MakeIPv6WellKnownMulticast(uint8_t aScope, uint32_t aGroupId)
{
    constexpr IPv6MulticastFlags lFlags;

    return (MakeIPv6Multicast(lFlags, aScope, aGroupId));
}

IPAddress IPAddress::MakeIPv6TransientMulticast(IPv6MulticastFlags aFlags, uint8_t aScope,
                                                const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES])
{
    aFlags.Set(IPv6MulticastFlag::kTransient);
    return (MakeIPv6Multicast(aFlags, aScope, aGroupId));
}

IPAddress IPAddress::MakeIPv6PrefixMulticast(uint8_t aScope, uint8_t aPrefixLength, const uint64_t & aPrefix, uint32_t aGroupId)
{
    const uint8_t lReserved                                       = 0;
    const IPv6MulticastFlags lFlags                               = IPv6MulticastFlag::kPrefix;
    const uint8_t lGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] = { lReserved,
                                                                      aPrefixLength,
                                                                      static_cast<uint8_t>((aPrefix & 0xFF00000000000000ULL) >> 56),
                                                                      static_cast<uint8_t>((aPrefix & 0x00FF000000000000ULL) >> 48),
                                                                      static_cast<uint8_t>((aPrefix & 0x0000FF0000000000ULL) >> 40),
                                                                      static_cast<uint8_t>((aPrefix & 0x000000FF00000000ULL) >> 32),
                                                                      static_cast<uint8_t>((aPrefix & 0x00000000FF000000ULL) >> 24),
                                                                      static_cast<uint8_t>((aPrefix & 0x0000000000FF0000ULL) >> 16),
                                                                      static_cast<uint8_t>((aPrefix & 0x000000000000FF00ULL) >> 8),
                                                                      static_cast<uint8_t>((aPrefix & 0x00000000000000FFULL) >> 0),
                                                                      static_cast<uint8_t>((aGroupId & 0xFF000000U) >> 24),
                                                                      static_cast<uint8_t>((aGroupId & 0x00FF0000U) >> 16),
                                                                      static_cast<uint8_t>((aGroupId & 0x0000FF00U) >> 8),
                                                                      static_cast<uint8_t>((aGroupId & 0x000000FFU) >> 0) };

    return (MakeIPv6TransientMulticast(lFlags, aScope, lGroupId));
}

IPAddress IPAddress::MakeIPv4Broadcast()
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = 0;
    ipAddr.Addr[1] = 0;
    ipAddr.Addr[2] = htonl(0xFFFF);
    ipAddr.Addr[3] = 0xFFFFFFFF;
    return ipAddr;
}

IPAddress IPAddress::Loopback(IPAddressType type)
{
    IPAddress address;
#if INET_CONFIG_ENABLE_IPV4
    if (type == IPAddressType::kIPv4)
    {
        address.Addr[0] = 0;
        address.Addr[1] = 0;
        address.Addr[2] = htonl(0xFFFF);
        address.Addr[3] = htonl(0x7F000001);
    }
    else
#endif
    {
        address.Addr[0] = 0;
        address.Addr[1] = 0;
        address.Addr[2] = 0;
        address.Addr[3] = htonl(1);
    }

    return address;
}

} // namespace Inet
} // namespace chip
