/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file implements the class <tt>nl::Inet::IPAddress</tt> and
 *      related enumerated constants. The Nest Inet Layer uses objects
 *      of this class to represent Internet protocol addresses of both
 *      IPv4 and IPv6 address families. (IPv4 addresses are stored
 *      internally in the V4COMPAT format, reserved for that purpose.)
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <string.h>

#include <Weave/Core/WeaveEncoding.h>
#include <InetLayer/InetLayer.h>

#include "arpa-inet-compatibility.h"

namespace nl {
namespace Inet {

IPAddress IPAddress::Any;

bool IPAddress::operator==(const IPAddress& other) const
{
    return Addr[0] == other.Addr[0] && Addr[1] == other.Addr[1] && Addr[2] == other.Addr[2] && Addr[3] == other.Addr[3];
}

bool IPAddress::operator!=(const IPAddress& other) const
{
    return Addr[0] != other.Addr[0] || Addr[1] != other.Addr[1] || Addr[2] != other.Addr[2] || Addr[3] != other.Addr[3];
}

IPAddress & IPAddress::operator=(const IPAddress& other)
{
    if (this != &other)
    {
        Addr[0] = other.Addr[0];
        Addr[1] = other.Addr[1];
        Addr[2] = other.Addr[2];
        Addr[3] = other.Addr[3];
    }

    return *this;
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
ip_addr_t IPAddress::ToLwIPAddr(void) const
{
    ip_addr_t ret;

    switch (Type())
    {
#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        IP_SET_TYPE_VAL(ret, IPADDR_TYPE_V4);
        *ip_2_ip4(&ret) = IPAddress::ToIPv4();
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kIPAddressType_IPv6:
        IP_SET_TYPE_VAL(ret, IPADDR_TYPE_V6);
        *ip_2_ip6(&ret) = IPAddress::ToIPv6();
        break;

    default:
        ret = *IP_ADDR_ANY;
        break;
    }

    return ret;
}

IPAddress IPAddress::FromLwIPAddr(const ip_addr_t &addr)
{
    IPAddress ret;

    switch (IP_GET_TYPE(&addr))
    {
#if INET_CONFIG_ENABLE_IPV4
    case IPADDR_TYPE_V4:
        ret = IPAddress::FromIPv4(*ip_2_ip4(&addr));
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPADDR_TYPE_V6:
        ret = IPAddress::FromIPv6(*ip_2_ip6(&addr));
        break;

    default:
        ret = Any;
        break;
    }

    return ret;
}

lwip_ip_addr_type IPAddress::ToLwIPAddrType(IPAddressType typ)
{
    lwip_ip_addr_type ret;

    switch (typ)
    {
#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        ret = IPADDR_TYPE_V4;
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kIPAddressType_IPv6:
        ret = IPADDR_TYPE_V6;
        break;

    default:
        ret = IPADDR_TYPE_ANY;
        break;
    }

    return ret;
}
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5

#if INET_CONFIG_ENABLE_IPV4
ip4_addr_t IPAddress::ToIPv4() const
{
    return *(ip4_addr_t *)&Addr[3];
}

IPAddress IPAddress::FromIPv4(const ip4_addr_t &ipv4Addr)
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = 0;
    ipAddr.Addr[1] = 0;
    ipAddr.Addr[2] = htonl(0xFFFF);
    ipAddr.Addr[3] = ipv4Addr.addr;
    return ipAddr;
}
#endif // INET_CONFIG_ENABLE_IPV4

ip6_addr_t IPAddress::ToIPv6() const
{
    return *(ip6_addr_t *)Addr;
}

IPAddress IPAddress::FromIPv6(const ip6_addr_t &ipv6Addr)
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = ipv6Addr.addr[0];
    ipAddr.Addr[1] = ipv6Addr.addr[1];
    ipAddr.Addr[2] = ipv6Addr.addr[2];
    ipAddr.Addr[3] = ipv6Addr.addr[3];
    return ipAddr;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if INET_CONFIG_ENABLE_IPV4
struct in_addr IPAddress::ToIPv4() const
{
    struct in_addr ipv4Addr;
    ipv4Addr.s_addr = Addr[3];
    return ipv4Addr;
}

IPAddress IPAddress::FromIPv4(const struct in_addr &ipv4Addr)
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = 0;
    ipAddr.Addr[1] = 0;
    ipAddr.Addr[2] = htonl(0xFFFF);
    ipAddr.Addr[3] = ipv4Addr.s_addr;
    return ipAddr;
}
#endif // INET_CONFIG_ENABLE_IPV4

struct in6_addr IPAddress::ToIPv6() const
{
    return *(struct in6_addr *) &Addr;
}

IPAddress IPAddress::FromIPv6(const struct in6_addr &ipv6Addr)
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = htonl(((uint32_t)ipv6Addr.s6_addr[0])  << 24 |
                           ((uint32_t)ipv6Addr.s6_addr[1])  << 16 |
                           ((uint32_t)ipv6Addr.s6_addr[2])  << 8  |
                           ((uint32_t)ipv6Addr.s6_addr[3]));
    ipAddr.Addr[1] = htonl(((uint32_t)ipv6Addr.s6_addr[4])  << 24 |
                           ((uint32_t)ipv6Addr.s6_addr[5])  << 16 |
                           ((uint32_t)ipv6Addr.s6_addr[6])  << 8  |
                           ((uint32_t)ipv6Addr.s6_addr[7]));
    ipAddr.Addr[2] = htonl(((uint32_t)ipv6Addr.s6_addr[8])  << 24 |
                           ((uint32_t)ipv6Addr.s6_addr[9])  << 16 |
                           ((uint32_t)ipv6Addr.s6_addr[10]) << 8  |
                           ((uint32_t)ipv6Addr.s6_addr[11]));
    ipAddr.Addr[3] = htonl(((uint32_t)ipv6Addr.s6_addr[12]) << 24 |
                           ((uint32_t)ipv6Addr.s6_addr[13]) << 16 |
                           ((uint32_t)ipv6Addr.s6_addr[14]) << 8  |
                           ((uint32_t)ipv6Addr.s6_addr[15]));
    return ipAddr;
}

IPAddress IPAddress::FromSockAddr(const struct sockaddr& sockaddr)
{
#if INET_CONFIG_ENABLE_IPV4
    if (sockaddr.sa_family == AF_INET)
        return FromIPv4(((sockaddr_in *) &sockaddr)->sin_addr);
#endif // INET_CONFIG_ENABLE_IPV4
    if (sockaddr.sa_family == AF_INET6)
        return FromIPv6(((sockaddr_in6 *) &sockaddr)->sin6_addr);
    return Any;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

// Is address an IPv4 address encoded in IPv6 format?
bool IPAddress::IsIPv4() const
{
    return Addr[0] == 0 && Addr[1] == 0 && Addr[2] == htonl(0xFFFF);
}

// Is address a IPv4 multicast address?
bool IPAddress::IsIPv4Multicast(void) const
{
    return (IsIPv4() && ((ntohl(Addr[3]) & 0xF0000000U) == 0xE0000000U));
}

// Is address the IPv4 broadcast address?
bool IPAddress::IsIPv4Broadcast(void) const
{
    return (IsIPv4() && (Addr[3] == 0xFFFFFFFFU));
}

// Is address an IPv4 or IPv6 multicast address?
bool IPAddress::IsMulticast(void) const
{
    return (IsIPv6Multicast() || IsIPv4Multicast());
}

bool IPAddress::IsIPv6(void) const
{
    return *this != Any && !IsIPv4();
}

// Is address an IPv6 multicast address?
bool IPAddress::IsIPv6Multicast(void) const
{
    return (ntohl(Addr[0]) & 0xFF000000U) == 0xFF000000U;
}

// Is address an IPv6 Global Unicast Address?
bool IPAddress::IsIPv6GlobalUnicast(void) const
{
    return (ntohl(Addr[0]) & 0xE0000000U) == 0x20000000U;
}

// Is address an IPv6 Unique Local Address?
bool IPAddress::IsIPv6ULA() const
{
    return (ntohl(Addr[0]) & 0xFF000000U) == 0xFD000000U;
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
        return (((uint64_t) ntohl(Addr[2])) << 32) | ((uint64_t) ntohl(Addr[3]));
    else
        return 0;
}

// Extract the subnet id from a IPv6 ULA address.  Returns 0 if the address
// is not a ULA.
uint16_t IPAddress::Subnet() const
{
    if (IsIPv6ULA())
        return (uint16_t) ntohl(Addr[1]);
    else
        return 0;
}

// Extract the global id from a IPv6 ULA address.  Returns 0 if the address
// is not a ULA.
uint64_t IPAddress::GlobalId() const
{
    if (IsIPv6ULA())
        return (((uint64_t) (ntohl(Addr[0]) & 0xFFFFFF)) << 16) | ((uint64_t) (ntohl(Addr[1])) & 0xFFFF0000) >> 16;
    else
        return 0;
}

IPAddressType IPAddress::Type() const
{
    if (Addr[0] == 0 && Addr[1] == 0 && Addr[2] == 0 && Addr[3] == 0)
        return kIPAddressType_Any;
#if INET_CONFIG_ENABLE_IPV4
    if (Addr[0] == 0 && Addr[1] == 0 && Addr[2] == htonl(0xFFFF))
        return kIPAddressType_IPv4;
#endif // INET_CONFIG_ENABLE_IPV4
    return kIPAddressType_IPv6;
}

// Encode IPAddress to buffer in network byte order. Buffer must have at least 128 bits of available space.
// Decoder must infer IP address type from context.
void IPAddress::WriteAddress(uint8_t *&p) const
{
    // Since each of the 32bit values in the Addr array is in network byte order, a simple
    // memcpy of the entire array is sufficient while copying the address.

    memcpy(p, &Addr[0], NL_INET_IPV6_ADDR_LEN_IN_BYTES);

    p += NL_INET_IPV6_ADDR_LEN_IN_BYTES;
}

// Decode IPAddress from buffer in network byte order. Must infer IP address type from context.
void IPAddress::ReadAddress(const uint8_t *&p, IPAddress &output)
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

    addr.Addr[0] = 0xFD000000 | (uint32_t) ((globalId & 0xFFFFFF0000ULL) >> 16);
    addr.Addr[0] = htonl(addr.Addr[0]);

    addr.Addr[1] = (uint32_t) ((globalId & 0x000000FFFFULL) << 16) | subnet;
    addr.Addr[1] = htonl(addr.Addr[1]);

    addr.Addr[2] = htonl((uint32_t) (interfaceId >> 32));
    addr.Addr[3] = htonl((uint32_t) (interfaceId));

    return addr;
}

IPAddress IPAddress::MakeLLA(uint64_t interfaceId)
{
    IPAddress addr;

    addr.Addr[0] = htonl(0xFE800000);
    addr.Addr[1] = 0;

    addr.Addr[2] = htonl((uint32_t) (interfaceId >> 32));
    addr.Addr[3] = htonl((uint32_t) (interfaceId));

    return addr;
}

IPAddress IPAddress::MakeIPv6Multicast(uint8_t aFlags, uint8_t aScope, const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES])
{
    const uint32_t lFlagsAndScope = (((aFlags & 0xF) << 20) |
                                     ((aScope & 0xF) << 16));
    IPAddress addr;

    addr.Addr[0] = htonl((0xFF000000U | lFlagsAndScope) |
                         (aGroupId[ 0] <<  8) |
                         (aGroupId[ 1] <<  0));
    addr.Addr[1] = htonl((aGroupId[ 2] << 24) |
                         (aGroupId[ 3] << 16) |
                         (aGroupId[ 4] <<  8) |
                         (aGroupId[ 5] <<  0));
    addr.Addr[2] = htonl((aGroupId[ 6] << 24) |
                         (aGroupId[ 7] << 16) |
                         (aGroupId[ 8] <<  8) |
                         (aGroupId[ 9] <<  0));
    addr.Addr[3] = htonl((aGroupId[10] << 24) |
                         (aGroupId[11] << 16) |
                         (aGroupId[12] <<  8) |
                         (aGroupId[13] <<  0));

    return addr;
}

IPAddress IPAddress::MakeIPv6Multicast(uint8_t aFlags, uint8_t aScope, uint32_t aGroupId)
{
    const uint8_t lGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] =
    {
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        (uint8_t)((aGroupId & 0xFF000000U) >> 24),
        (uint8_t)((aGroupId & 0x00FF0000U) >> 16),
        (uint8_t)((aGroupId & 0x0000FF00U) >>  8),
        (uint8_t)((aGroupId & 0x000000FFU) >>  0)
    };

    return (MakeIPv6Multicast(aFlags, aScope, lGroupId));
}

IPAddress IPAddress::MakeIPv6WellKnownMulticast(uint8_t aScope, uint32_t aGroupId)
{
    const uint8_t lFlags = 0;

    return (MakeIPv6Multicast(lFlags, aScope, aGroupId));
}

IPAddress IPAddress::MakeIPv6TransientMulticast(uint8_t aFlags, uint8_t aScope, const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES])
{
    const uint8_t lFlags = (aFlags | kIPv6MulticastFlag_Transient);

    return (MakeIPv6Multicast(lFlags, aScope, aGroupId));
}

IPAddress IPAddress::MakeIPv6PrefixMulticast(uint8_t aScope, uint8_t aPrefixLength, const uint64_t &aPrefix, uint32_t aGroupId)
{
    const uint8_t lReserved    = 0;
    const uint8_t lFlags       = kIPv6MulticastFlag_Prefix;
    const uint8_t lGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] =
    {
        lReserved,
        aPrefixLength,
        (uint8_t)((aPrefix  & 0xFF00000000000000ULL) >> 56),
        (uint8_t)((aPrefix  & 0x00FF000000000000ULL) >> 48),
        (uint8_t)((aPrefix  & 0x0000FF0000000000ULL) >> 40),
        (uint8_t)((aPrefix  & 0x000000FF00000000ULL) >> 32),
        (uint8_t)((aPrefix  & 0x00000000FF000000ULL) >> 24),
        (uint8_t)((aPrefix  & 0x0000000000FF0000ULL) >> 16),
        (uint8_t)((aPrefix  & 0x000000000000FF00ULL) >>  8),
        (uint8_t)((aPrefix  & 0x00000000000000FFULL) >>  0),
        (uint8_t)((aGroupId & 0xFF000000U)           >> 24),
        (uint8_t)((aGroupId & 0x00FF0000U)           >> 16),
        (uint8_t)((aGroupId & 0x0000FF00U)           >>  8),
        (uint8_t)((aGroupId & 0x000000FFU)           >>  0)
    };

    return (MakeIPv6TransientMulticast(lFlags, aScope, lGroupId));
}

IPAddress IPAddress::MakeIPv4Broadcast(void)
{
    IPAddress ipAddr;
    ipAddr.Addr[0] = 0;
    ipAddr.Addr[1] = 0;
    ipAddr.Addr[2] = htonl(0xFFFF);
    ipAddr.Addr[3] = 0xFFFFFFFF;
    return ipAddr;
}


} // namespace Inet
} // namespace nl
