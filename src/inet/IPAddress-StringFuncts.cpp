/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements the human-readable string formatting and
 *      parsing methods from class <tt>Inet::IPAddress</tt>.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <limits>
#include <stdint.h>
#include <string.h>

#include <inet/InetLayer.h>
#include <lib/support/CodeUtils.h>

#if !CHIP_SYSTEM_CONFIG_USE_LWIP
#include <arpa/inet.h>
#endif

namespace chip {
namespace Inet {

char * IPAddress::ToString(char * buf, uint32_t bufSize) const
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
    if (IsIPv4())
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip4_addr_t ip4_addr = ToIPv4();
        ip4addr_ntoa_r(&ip4_addr, buf, (int) bufSize);
#else  // LWIP_VERSION_MAJOR <= 1
        ip_addr_t ip4_addr = ToIPv4();
        ipaddr_ntoa_r(&ip4_addr, buf, (int) bufSize);
#endif // LWIP_VERSION_MAJOR <= 1
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
        ip6_addr_t ip6_addr = ToIPv6();
        ip6addr_ntoa_r(&ip6_addr, buf, (int) bufSize);
    }
#else // !CHIP_SYSTEM_CONFIG_USE_LWIP
    // socklen_t is sometimes signed, sometimes not, so the only safe way to do
    // this is to promote everything to an unsigned type that's known to be big
    // enough for everything, then cast back to uint32_t after taking the min.
    bufSize =
        static_cast<uint32_t>(min(static_cast<uintmax_t>(std::numeric_limits<socklen_t>::max()), static_cast<uintmax_t>(bufSize)));
#if INET_CONFIG_ENABLE_IPV4
    if (IsIPv4())
    {
        const void * addr = &Addr[3];
        const char * s    = inet_ntop(AF_INET, addr, buf, static_cast<socklen_t>(bufSize));
        // This cast is safe because |s| points into |buf| which is not const.
        buf = const_cast<char *>(s);
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
        const void * addr = &Addr[0];
        const char * s    = inet_ntop(AF_INET6, addr, buf, static_cast<socklen_t>(bufSize));
        // This cast is safe because |s| points into |buf| which is not const.
        buf = const_cast<char *>(s);
    }
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

    return buf;
}

bool IPAddress::FromString(const char * str, IPAddress & output)
{
#if INET_CONFIG_ENABLE_IPV4
    if (strchr(str, ':') == nullptr)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip4_addr_t ipv4Addr;
        if (!ip4addr_aton(str, &ipv4Addr))
            return false;
#else  // LWIP_VERSION_MAJOR <= 1
        ip_addr_t ipv4Addr;
        if (!ipaddr_aton(str, &ipv4Addr))
            return false;
#endif // LWIP_VERSION_MAJOR <= 1
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
        struct in_addr ipv4Addr;
        if (inet_pton(AF_INET, str, &ipv4Addr) < 1)
            return false;
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
        output = FromIPv4(ipv4Addr);
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        ip6_addr_t ipv6Addr;
        if (!ip6addr_aton(str, &ipv6Addr))
            return false;
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
        struct in6_addr ipv6Addr;
        if (inet_pton(AF_INET6, str, &ipv6Addr) < 1)
            return false;
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
        output = FromIPv6(ipv6Addr);
    }

    return true;
}

bool IPAddress::FromString(const char * str, size_t strLen, IPAddress & output)
{
    bool res = false;

    if (strLen < INET6_ADDRSTRLEN)
    {
        char hostNameBuf[INET6_ADDRSTRLEN];
        memcpy(hostNameBuf, str, strLen);
        hostNameBuf[strLen] = 0;
        res                 = IPAddress::FromString(hostNameBuf, output);
    }

    return res;
}

} // namespace Inet
} // namespace chip
