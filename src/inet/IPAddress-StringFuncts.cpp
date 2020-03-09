/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements the human-readable string formatting and
 *      parsing methods from class <tt>nl::Inet::IPAddress</tt>.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <string.h>

#include <InetLayer/InetLayer.h>

#if !WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <arpa/inet.h>
#endif

namespace nl {
namespace Inet {

char *IPAddress::ToString(char *buf, uint32_t bufSize) const
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
    if (IsIPv4())
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip4_addr_t ip4_addr = ToIPv4();
        ip4addr_ntoa_r(&ip4_addr, buf, (int)bufSize);
#else // LWIP_VERSION_MAJOR <= 1
        ip_addr_t ip4_addr = ToIPv4();
        ipaddr_ntoa_r(&ip4_addr, buf, (int)bufSize);
#endif // LWIP_VERSION_MAJOR <= 1
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
        ip6_addr_t ip6_addr = ToIPv6();
        ip6addr_ntoa_r(&ip6_addr, buf, (int)bufSize);
    }
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
    if (IsIPv4())
    {
        buf = (char *)inet_ntop(AF_INET, (const void *) &Addr[3], buf, bufSize);
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
        buf = (char *)inet_ntop(AF_INET6, (const void *) Addr, buf, bufSize);
    }
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP

    return buf;
}

bool IPAddress::FromString(const char *str, IPAddress& output)
{
#if INET_CONFIG_ENABLE_IPV4
    if (strchr(str, ':') == NULL)
    {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip4_addr_t ipv4Addr;
        if (!ip4addr_aton(str, &ipv4Addr))
            return false;
#else // LWIP_VERSION_MAJOR <= 1
        ip_addr_t ipv4Addr;
        if (!ipaddr_aton(str, &ipv4Addr))
            return false;
#endif // LWIP_VERSION_MAJOR <= 1
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
        struct in_addr ipv4Addr;
        if (inet_pton(AF_INET, str, &ipv4Addr) < 1)
            return false;
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP
        output = FromIPv4(ipv4Addr);
    }
    else
#endif // INET_CONFIG_ENABLE_IPV4
    {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        ip6_addr_t ipv6Addr;
        if (!ip6addr_aton(str, &ipv6Addr))
            return false;
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
        struct in6_addr ipv6Addr;
        if (inet_pton(AF_INET6, str, &ipv6Addr) < 1)
            return false;
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP
        output = FromIPv6(ipv6Addr);
    }

    return true;
}

bool IPAddress::FromString(const char *str, size_t strLen, IPAddress& output)
{
    bool res = false;

    if (strLen < INET6_ADDRSTRLEN)
    {
        char hostNameBuf[INET6_ADDRSTRLEN];
        memcpy(hostNameBuf, str, strLen);
        hostNameBuf[strLen] = 0;
        res = IPAddress::FromString(hostNameBuf, output);
    }

    return res;
}



} // namespace Inet
} // namespace nl
