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

#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Inet {

namespace PlatformNetworkInterface {

CHIP_ERROR GetInterfaceName(struct netif * interface, char * nameBuf, size_t nameBufSize)
{
    if (interface)
    {
        int status = snprintf(nameBuf, nameBufSize, "%c%c%d", interface->name[0], interface->name[1], interface->num);
        if (status >= static_cast<int>(nameBufSize))
            return CHIP_ERROR_BUFFER_TOO_SMALL;
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
    if (strlen(intfName) < 3)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }
    char * parseEnd;
    unsigned long intfNum = strtoul(intfName + 2, &parseEnd, 10);
    if (*parseEnd != 0 || intfNum > UINT8_MAX)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }
    struct netif * intf;
#if defined(NETIF_FOREACH)
    NETIF_FOREACH(intf)
#else
    for (intf = netif_list; intf != NULL; intf = intf->next)
#endif
    {
        if (intf->name[0] == intfName[0] && intf->name[1] == intfName[1] && intf->num == (uint8_t) intfNum)
        {
            interface = ToInterfaceId(intf);
            return CHIP_NO_ERROR;
        }
    }
    interface = InterfaceId::Null();
    return INET_ERROR_UNKNOWN_INTERFACE;
}

CHIP_ERROR GetLinkLocalAddr(struct netif * interface, IPAddress * llAddr)
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if !LWIP_IPV6
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif //! LWIP_IPV6

    for (struct netif * intf = netif_list; intf != nullptr; intf = intf->next)
    {
        if ((interface != nullptr) && (interface != intf))
            continue;
        for (int j = 0; j < LWIP_IPV6_NUM_ADDRESSES; ++j)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(intf, j)) && ip6_addr_islinklocal(netif_ip6_addr(intf, j)))
            {
                (*llAddr) = IPAddress(*netif_ip6_addr(intf, j));
                return CHIP_NO_ERROR;
            }
        }
        if (interface != nullptr)
        {
            return INET_ERROR_ADDRESS_NOT_FOUND;
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace PlatformNetworkInterface

bool InterfaceIteratorImplLwIP::Next()
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Verify the previous netif is still on the list if netifs.  If so,
    // advance to the next nextif.
    struct netif * prevNetif = mCurNetif;
#if defined(NETIF_FOREACH)
    NETIF_FOREACH(mCurNetif)
#else
    for (mCurNetif = netif_list; mCurNetif != NULL; mCurNetif = mCurNetif->next)
#endif
    {
        if (mCurNetif == prevNetif)
        {
            mCurNetif = mCurNetif->next;
            break;
        }
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return mCurNetif != NULL;
}

CHIP_ERROR InterfaceIteratorImplLwIP::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return PlatformNetworkInterface::GetInterfaceName(mCurNetif, nameBuf, nameBufSize);
}

bool InterfaceIteratorImplLwIP::IsUp()
{
    return HasCurrent() && netif_is_up(mCurNetif);
}

bool InterfaceIteratorImplLwIP::SupportsMulticast()
{
    return HasCurrent() && (mCurNetif->flags & (NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST)) != 0;
}

bool InterfaceIteratorImplLwIP::HasBroadcastAddress()
{
    return HasCurrent() && (mCurNetif->flags & NETIF_FLAG_BROADCAST) != 0;
}

CHIP_ERROR InterfaceIteratorImplLwIP::GetInterfaceType(InterfaceType & type)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR InterfaceIteratorImplLwIP::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize)
{
    VerifyOrReturnError(addressBuffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(addressBufferSize >= mCurNetif->hwaddr_len, CHIP_ERROR_BUFFER_TOO_SMALL);
    addressSize = mCurNetif->hwaddr_len;
    memcpy(addressBuffer, mCurNetif->hwaddr, addressSize);
    return CHIP_NO_ERROR;
}

bool InterfaceAddressIteratorImplLwIP::HasCurrent()
{
    return mIntfIter.HasCurrent() && ((mCurAddrIndex != kBeforeStartIndex) || Next());
}

bool InterfaceAddressIteratorImplLwIP::Next()
{
    mCurAddrIndex++;

    while (mIntfIter.HasCurrent())
    {
        struct netif * curIntf = PlatformNetworkInterface::FromInterfaceId(mIntfIter.GetInterfaceId());

        while (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(curIntf, mCurAddrIndex)))
            {
                return true;
            }
            mCurAddrIndex++;
        }

#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        if (mCurAddrIndex == LWIP_IPV6_NUM_ADDRESSES)
        {
            if (!ip4_addr_isany(netif_ip4_addr(curIntf)))
            {
                return true;
            }
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4

        mIntfIter.Next();
        mCurAddrIndex = 0;
    }

    return false;
}

CHIP_ERROR InterfaceAddressIteratorImplLwIP::GetAddress(IPAddress & outIPAddress)
{
    if (!HasCurrent())
    {
        return CHIP_ERROR_SENTINEL;
    }

    struct netif * curIntf = PlatformNetworkInterface::FromInterfaceId(mIntfIter.GetInterfaceId());

    if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
    {
        outIPAddress = IPAddress(*netif_ip6_addr(curIntf, mCurAddrIndex));
        return CHIP_NO_ERROR;
    }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    else
    {
        outIPAddress = IPAddress(*netif_ip4_addr(curIntf));
        return CHIP_NO_ERROR;
    }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    return CHIP_ERROR_INTERNAL;
}

uint8_t InterfaceAddressIteratorImplLwIP::GetPrefixLength()
{
    if (HasCurrent())
    {
        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return 64;
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            struct netif * curIntf = PlatformNetworkInterface::FromInterfaceId(mIntfIter.GetInterfaceId());
            return NetmaskToPrefixLength((const uint8_t *) netif_ip4_netmask(curIntf), 4);
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    }
    return 0;
}

InterfaceId InterfaceAddressIteratorImplLwIP::GetInterfaceId()
{
    return HasCurrent() ? mIntfIter.GetInterfaceId() : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIteratorImplLwIP::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIteratorImplLwIP::IsUp()
{
    return HasCurrent() && mIntfIter.IsUp();
}

bool InterfaceAddressIteratorImplLwIP::SupportsMulticast()
{
    return HasCurrent() && mIntfIter.SupportsMulticast();
}

bool InterfaceAddressIteratorImplLwIP::HasBroadcastAddress()
{
    return HasCurrent() && mIntfIter.HasBroadcastAddress();
}

} // namespace Inet
} // namespace chip
