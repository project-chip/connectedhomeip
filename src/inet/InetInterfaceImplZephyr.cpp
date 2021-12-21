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

#include <lib/support/CodeUtils.h>

#include <net/net_if.h>

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Inet {

namespace PlatformNetworkInterface {

CHIP_ERROR GetInterfaceName(int interface, char * nameBuf, size_t nameBufSize)
{
    if (interface)
    {
        net_if * currentInterface = net_if_get_by_index(interface);
        if (!currentInterface)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        const char * name = net_if_get_device(currentInterface)->name;
        size_t nameLength = strlen(name);
        if (nameLength >= nameBufSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        strncpy(nameBuf, name, nameLength + 1);
        return CHIP_NO_ERROR;
    }
    if (nameBufSize < 1)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & interface)
{
    int currentId = 0;
    net_if * currentInterface;

    while ((currentInterface = net_if_get_by_index(++currentId)) != nullptr)
    {
        if (strcmp(net_if_get_device(currentInterface)->name, intfName) == 0)
        {
            interface = PlatformNetworkInterface::ToInterfaceId(currentId);
            return CHIP_NO_ERROR;
        }
    }
    interface = InterfaceId::Null();
    return INET_ERROR_UNKNOWN_INTERFACE;
}

CHIP_ERROR GetLinkLocalAddr(int interface, IPAddress * llAddr)
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    net_if * const iface = interface ? net_if_get_by_index(interface) : net_if_get_default();
    VerifyOrReturnError(iface != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    in6_addr * const ip6_addr = net_if_ipv6_get_ll(iface, NET_ADDR_PREFERRED);
    VerifyOrReturnError(ip6_addr != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    *llAddr = IPAddress(*ip6_addr);

    return CHIP_NO_ERROR;
}

} // namespace PlatformNetworkInterface

InterfaceIteratorImplZephyr::InterfaceIteratorImplZephyr() : mCurrentInterface(net_if_get_by_index(mCurrentId)) {}

bool InterfaceIteratorImplZephyr::HasCurrent(void)
{
    return mCurrentInterface != nullptr;
}

bool InterfaceIteratorImplZephyr::Next()
{
    mCurrentInterface = net_if_get_by_index(++mCurrentId);
    return HasCurrent();
}

InterfaceId InterfaceIteratorImplZephyr::GetInterfaceId(void)
{
    return HasCurrent() ? PlatformNetworkInterface::ToInterfaceId(mCurrentId) : InterfaceId::Null();
}

CHIP_ERROR InterfaceIteratorImplZephyr::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return PlatformNetworkInterface::GetInterfaceName(mCurrentId, nameBuf, nameBufSize);
}

bool InterfaceIteratorImplZephyr::IsUp()
{
    return HasCurrent() && net_if_is_up(mCurrentInterface);
}

bool InterfaceIteratorImplZephyr::SupportsMulticast()
{
    return HasCurrent() && NET_IF_MAX_IPV6_MADDR > 0;
}

bool InterfaceIteratorImplZephyr::HasBroadcastAddress()
{
    // Zephyr seems to handle broadcast address for IPv4 implicitly
    return HasCurrent() && INET_CONFIG_ENABLE_IPV4;
}

CHIP_ERROR InterfaceIteratorImplZephyr::GetInterfaceType(InterfaceType & type)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);

    const net_linkaddr * linkAddr = net_if_get_link_addr(mCurrentInterface);
    if (!linkAddr)
        return CHIP_ERROR_INCORRECT_STATE;

    // Do not consider other than WiFi and Thread for now.
    if (linkAddr->type == NET_LINK_IEEE802154)
    {
        type = InterfaceType::Thread;
    }
    // Zephyr doesn't define WiFi address type, so it shares the same type as Ethernet.
    else if (linkAddr->type == NET_LINK_ETHERNET)
    {
        type = InterfaceType::WiFi;
    }
    else
    {
        type = InterfaceType::Unknown;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR InterfaceIteratorImplZephyr::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize,
                                                           uint8_t addressBufferSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);

    if (!addressBuffer)
        return CHIP_ERROR_INVALID_ARGUMENT;

    const net_linkaddr * linkAddr = net_if_get_link_addr(mCurrentInterface);
    if (!linkAddr)
        return CHIP_ERROR_INCORRECT_STATE;

    if (linkAddr->len > addressBufferSize)
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    addressSize = linkAddr->len;
    memcpy(addressBuffer, linkAddr->addr, linkAddr->len);

    return CHIP_NO_ERROR;
}

InterfaceAddressIteratorImplZephyr::InterfaceAddressIteratorImplZephyr() = default;

bool InterfaceAddressIteratorImplZephyr::HasCurrent()
{
    return mIntfIter.HasCurrent() && (mCurAddrIndex >= 0 || Next());
}

bool InterfaceAddressIteratorImplZephyr::Next()
{
    while (mIntfIter.HasCurrent())
    {
        if (mCurAddrIndex == -1) // first address for the current interface
        {
            const net_if_config * config =
                net_if_get_config(net_if_get_by_index(PlatformNetworkInterface::FromInterfaceId(mIntfIter.GetInterfaceId())));
            mIpv6 = config->ip.ipv6;
        }

        while (++mCurAddrIndex < NET_IF_MAX_IPV6_ADDR)
            if (mIpv6->unicast[mCurAddrIndex].is_used)
                return true;

        mCurAddrIndex = -1;
        mIntfIter.Next();
    }

    return false;
}

CHIP_ERROR InterfaceAddressIteratorImplZephyr::GetAddress(IPAddress & outIPAddress)
{
    if (HasCurrent())
    {
        outIPAddress = IPAddress(mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_SENTINEL;
}

uint8_t InterfaceAddressIteratorImplZephyr::GetPrefixLength()
{
    if (HasCurrent())
    {
        net_if * const iface = net_if_get_by_index(PlatformNetworkInterface::FromInterfaceId(mIntfIter.GetInterfaceId()));
        net_if_ipv6_prefix * const prefix = net_if_ipv6_prefix_get(iface, &mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return prefix ? prefix->len : 128;
    }
    return 0;
}

InterfaceId InterfaceAddressIteratorImplZephyr::GetInterfaceId()
{
    return HasCurrent() ? mIntfIter.GetInterfaceId() : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIteratorImplZephyr::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIteratorImplZephyr::IsUp()
{
    return HasCurrent() && mIntfIter.IsUp();
}

bool InterfaceAddressIteratorImplZephyr::SupportsMulticast()
{
    return HasCurrent() && mIntfIter.SupportsMulticast();
}

bool InterfaceAddressIteratorImplZephyr::HasBroadcastAddress()
{
    return HasCurrent() && mIntfIter.HasBroadcastAddress();
}

} // namespace Inet
} // namespace chip
