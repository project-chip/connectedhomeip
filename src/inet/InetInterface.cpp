/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *      Implementation of network interface abstraction layer.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inet/InetInterface.h>

#include <inet/IPPrefix.h>

namespace chip {
namespace Inet {

// static
InterfaceId InterfaceId::FromIPAddress(const IPAddress & addr)
{
    InterfaceAddressIterator addrIter;

    for (; addrIter.HasCurrent(); addrIter.Next())
    {
        IPAddress curAddr;
        if ((addrIter.GetAddress(curAddr) == CHIP_NO_ERROR) && (addr == curAddr))
        {
            return addrIter.GetInterfaceId();
        }
    }

    return InterfaceId::Null();
}

// static
bool InterfaceId::MatchLocalIPv6Subnet(const IPAddress & addr)
{
    if (addr.IsIPv6LinkLocal())
        return true;

    InterfaceAddressIterator ifAddrIter;
    for (; ifAddrIter.HasCurrent(); ifAddrIter.Next())
    {
        IPPrefix addrPrefix;
        if (ifAddrIter.GetAddress(addrPrefix.IPAddr) != CHIP_NO_ERROR)
            continue;
#if INET_CONFIG_ENABLE_IPV4
        if (addrPrefix.IPAddr.IsIPv4())
            continue;
#endif // INET_CONFIG_ENABLE_IPV4
        if (addrPrefix.IPAddr.IsIPv6LinkLocal())
            continue;
        addrPrefix.Length = ifAddrIter.GetPrefixLength();
        if (addrPrefix.MatchAddress(addr))
            return true;
    }

    return false;
}

uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen)
{
    uint8_t prefixLen = 0;

    for (uint16_t i = 0; i < netmaskLen; i++, prefixLen = static_cast<uint8_t>(prefixLen + 8u))
    {
        uint8_t b = netmask[i];
        if (b != 0xFF)
        {
            if ((b & 0xF0) == 0xF0)
                prefixLen = static_cast<uint8_t>(prefixLen + 4u);
            else
                b = static_cast<uint8_t>(b >> 4);

            if ((b & 0x0C) == 0x0C)
                prefixLen = static_cast<uint8_t>(prefixLen + 2u);
            else
                b = static_cast<uint8_t>(b >> 2);

            if ((b & 0x02) == 0x02)
                prefixLen++;

            break;
        }
    }

    return prefixLen;
}

} // namespace Inet
} // namespace chip
