/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <inet/InetInterface.h>

#include <inet/IPPrefix.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/SafeInt.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */
#include "InetInterfaceImpl.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <zephyr/net/net_if.h>
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#include <inet/UDPEndPointImplOpenThread.h>
#endif

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Inet {

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
CHIP_ERROR InterfaceId::GetInterfaceName(char * nameBuf, size_t nameBufSize) const
{
    if (mPlatformInterface && nameBufSize >= kMaxIfNameLength)
    {
        nameBuf[0] = 'o';
        nameBuf[1] = 't';
        nameBuf[2] = 0;
    }
    else
    {
        nameBuf[0] = 0;
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR InterfaceId::InterfaceNameToId(const char * intfName, InterfaceId & interface)
{
    if (strlen(intfName) < 3)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }
    char * parseEnd       = nullptr;
    unsigned long intfNum = strtoul(intfName + 2, &parseEnd, 10);
    if (*parseEnd != 0 || intfNum > UINT8_MAX)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }

    interface = InterfaceId(intfNum);
    if (intfNum == 0)
    {
        return INET_ERROR_UNKNOWN_INTERFACE;
    }
    return CHIP_NO_ERROR;
}

bool InterfaceIterator::Next()
{
    // TODO : Cleanup #17346
    return false;
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return InterfaceId(1).GetInterfaceName(nameBuf, nameBufSize);
}

InterfaceId InterfaceIterator::GetInterfaceId()
{
    // only 1 interface is supported
    return HasCurrent() ? InterfaceId(1) : InterfaceId::Null();
}

bool InterfaceIterator::IsUp()
{
    return HasCurrent() && (otThreadGetDeviceRole(Inet::globalOtInstance) != OT_DEVICE_ROLE_DISABLED);
}

InterfaceAddressIterator::InterfaceAddressIterator()
{
    mNetifAddrList = nullptr;
    mCurAddr       = nullptr;
}

bool InterfaceAddressIterator::HasCurrent()
{
    return (mNetifAddrList != nullptr) ? (mCurAddr != nullptr) : Next();
}

bool InterfaceAddressIterator::Next()
{
    if (mNetifAddrList == nullptr)
    {
        if (Inet::globalOtInstance == nullptr)
            return false;
        mNetifAddrList = otIp6GetUnicastAddresses(Inet::globalOtInstance);
        mCurAddr       = mNetifAddrList;
    }
    else if (mCurAddr != nullptr)
    {
        mCurAddr = mCurAddr->mNext;
    }

    return (mCurAddr != nullptr);
}
CHIP_ERROR InterfaceAddressIterator::GetAddress(IPAddress & outIPAddress)
{
    if (!HasCurrent())
    {
        return CHIP_ERROR_SENTINEL;
    }

    outIPAddress = IPAddress(mCurAddr->mAddress);
    return CHIP_NO_ERROR;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    // Only 64 bits prefix are supported
    return 64;
}

bool InterfaceAddressIterator::IsUp()
{
    return HasCurrent() && (otThreadGetDeviceRole(Inet::globalOtInstance) != OT_DEVICE_ROLE_DISABLED);
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    // only 1 interface is supported
    return HasCurrent() ? InterfaceId(1) : InterfaceId::Null();
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    return HasCurrent() && (otIp6GetMulticastAddresses(Inet::globalOtInstance) != nullptr);
}

#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

CHIP_ERROR InterfaceId::GetInterfaceName(char * nameBuf, size_t nameBufSize) const
{
    if (mPlatformInterface)
    {
        int status = snprintf(nameBuf, nameBufSize, "%c%c%d", mPlatformInterface->name[0], mPlatformInterface->name[1],
                              mPlatformInterface->num);
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

CHIP_ERROR InterfaceId::InterfaceNameToId(const char * intfName, InterfaceId & interface)
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
            interface = InterfaceId(intf);
            return CHIP_NO_ERROR;
        }
    }
    interface = InterfaceId::Null();
    return INET_ERROR_UNKNOWN_INTERFACE;
}

bool InterfaceIterator::Next()
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Verify the previous netif is still on the list if netifs.  If so,
    // advance to the next nextif.
    struct netif * prevNetif = mCurNetif;
#if defined(NETIF_FOREACH)
    NETIF_FOREACH(mCurNetif)
#else
    for (mCurNetif = netif_list; mCurNetif != nullptr; mCurNetif = mCurNetif->next)
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

    return mCurNetif != nullptr;
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return InterfaceId(mCurNetif).GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceIterator::IsUp()
{
    return HasCurrent() && netif_is_up(mCurNetif);
}

bool InterfaceIterator::SupportsMulticast()
{
    return HasCurrent() && (mCurNetif->flags & (NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST)) != 0;
}

bool InterfaceIterator::HasBroadcastAddress()
{
    return HasCurrent() && (mCurNetif->flags & NETIF_FLAG_BROADCAST) != 0;
}

CHIP_ERROR InterfaceIterator::GetInterfaceType(InterfaceType & type)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR InterfaceIterator::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize)
{
    VerifyOrReturnError(addressBuffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(addressBufferSize >= mCurNetif->hwaddr_len, CHIP_ERROR_BUFFER_TOO_SMALL);
    addressSize = mCurNetif->hwaddr_len;
    memcpy(addressBuffer, mCurNetif->hwaddr, addressSize);
    return CHIP_NO_ERROR;
}

bool InterfaceAddressIterator::HasCurrent()
{
    return mIntfIter.HasCurrent() && ((mCurAddrIndex != kBeforeStartIndex) || Next());
}

bool InterfaceAddressIterator::Next()
{
    mCurAddrIndex++;

    while (mIntfIter.HasCurrent())
    {
        struct netif * curIntf = mIntfIter.GetInterfaceId().GetPlatformInterface();

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

CHIP_ERROR InterfaceAddressIterator::GetAddress(IPAddress & outIPAddress)
{
    if (!HasCurrent())
    {
        return CHIP_ERROR_SENTINEL;
    }

    struct netif * curIntf = mIntfIter.GetInterfaceId().GetPlatformInterface();

    if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
    {
        outIPAddress = IPAddress(*netif_ip6_addr(curIntf, mCurAddrIndex));
        return CHIP_NO_ERROR;
    }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    outIPAddress = IPAddress(*netif_ip4_addr(curIntf));
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_INTERNAL;
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return 64;
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        struct netif * curIntf = mIntfIter.GetInterfaceId().GetPlatformInterface();
        return NetmaskToPrefixLength((const uint8_t *) netif_ip4_netmask(curIntf), 4);
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    return HasCurrent() ? mIntfIter.GetInterfaceId() : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIterator::IsUp()
{
    return HasCurrent() && mIntfIter.IsUp();
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    return HasCurrent() && mIntfIter.SupportsMulticast();
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    return HasCurrent() && mIntfIter.HasBroadcastAddress();
}

CHIP_ERROR InterfaceId::GetLinkLocalAddr(IPAddress * llAddr) const
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if !LWIP_IPV6
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif //! LWIP_IPV6

    for (struct netif * intf = netif_list; intf != nullptr; intf = intf->next)
    {
        if ((mPlatformInterface != nullptr) && (mPlatformInterface != intf))
            continue;
        for (int j = 0; j < LWIP_IPV6_NUM_ADDRESSES; ++j)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(intf, j)) && ip6_addr_islinklocal(netif_ip6_addr(intf, j)))
            {
                (*llAddr) = IPAddress(*netif_ip6_addr(intf, j));
                return CHIP_NO_ERROR;
            }
        }
        if (mPlatformInterface != nullptr)
        {
            return INET_ERROR_ADDRESS_NOT_FOUND;
        }
    }

    return CHIP_NO_ERROR;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

CHIP_ERROR InterfaceId::GetInterfaceName(char * nameBuf, size_t nameBufSize) const
{
    if (mPlatformInterface)
    {
        char intfName[IF_NAMESIZE];
        if (if_indextoname(mPlatformInterface, intfName) == nullptr)
        {
            return CHIP_ERROR_POSIX(errno);
        }
        size_t nameLength = strlen(intfName);
        if (nameLength >= nameBufSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        Platform::CopyString(nameBuf, nameBufSize, intfName);
        return CHIP_NO_ERROR;
    }
    if (nameBufSize < 1)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InterfaceId::InterfaceNameToId(const char * intfName, InterfaceId & interface)
{
    // First attempt to parse as a numeric ID:
    char * parseEnd;
    unsigned long intfNum = strtoul(intfName, &parseEnd, 10);
    if (*parseEnd == 0)
    {
        if (intfNum > 0 && intfNum < UINT8_MAX && CanCastTo<InterfaceId::PlatformType>(intfNum))
        {
            interface = InterfaceId(static_cast<InterfaceId::PlatformType>(intfNum));
            return CHIP_NO_ERROR;
        }

        return INET_ERROR_UNKNOWN_INTERFACE;
    }

    // Falling back to name -> ID lookup otherwise (e.g. wlan0)
    unsigned int intfId = if_nametoindex(intfName);
    interface           = InterfaceId(intfId);
    if (intfId == 0)
    {
        return (errno == ENXIO) ? INET_ERROR_UNKNOWN_INTERFACE : CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
}

static int sIOCTLSocket = -1;

/**
 * @brief   Returns a global general purpose socket useful for invoking certain network IOCTLs.
 *
 * This function is thread-safe on all platforms.
 */
int GetIOCTLSocket()
{
    if (sIOCTLSocket == -1)
    {
        int s;
#ifdef SOCK_CLOEXEC
        s = socket(AF_INET, SOCK_STREAM, SOCK_CLOEXEC);
        if (s < 0)
#endif
        {
            s = socket(AF_INET, SOCK_STREAM, 0);
            fcntl(s, O_CLOEXEC);
        }

        if (!__sync_bool_compare_and_swap(&sIOCTLSocket, -1, s))
        {
            close(s);
        }
    }
    return sIOCTLSocket;
}

/**
 * @brief   Close the global socket created by \c GetIOCTLSocket.
 *
 * @details
 *   This function is provided for cases were leaving the global IOCTL socket
 *   open would register as a leak.
 *
 *   NB: This function is NOT thread-safe with respect to \c GetIOCTLSocket.
 */
void CloseIOCTLSocket()
{
    if (sIOCTLSocket != -1)
    {
        close(sIOCTLSocket);
        sIOCTLSocket = -1;
    }
}

InterfaceIterator::InterfaceIterator()
{
    mIntfArray       = nullptr;
    mCurIntf         = 0;
    mIntfFlags       = 0;
    mIntfFlagsCached = false;
}

InterfaceIterator::~InterfaceIterator()
{
    if (mIntfArray != nullptr)
    {
        if_freenameindexImpl(mIntfArray);
        mIntfArray = nullptr;
    }
}

bool InterfaceIterator::HasCurrent()
{
    return (mIntfArray != nullptr) ? mIntfArray[mCurIntf].if_index != 0 : Next();
}

bool InterfaceIterator::Next()
{
    if (mIntfArray == nullptr)
    {
        mIntfArray = if_nameindexImpl();
    }
    else if (mIntfArray[mCurIntf].if_index != 0)
    {
        mCurIntf++;
        mIntfFlags       = 0;
        mIntfFlagsCached = false;
    }
    return (mIntfArray != nullptr && mIntfArray[mCurIntf].if_index != 0);
}

InterfaceId InterfaceIterator::GetInterfaceId()
{
    return HasCurrent() ? InterfaceId(mIntfArray[mCurIntf].if_index) : InterfaceId::Null();
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mIntfArray[mCurIntf].if_name) < nameBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    Platform::CopyString(nameBuf, nameBufSize, mIntfArray[mCurIntf].if_name);
    return CHIP_NO_ERROR;
}

bool InterfaceIterator::IsUp()
{
    return (GetFlags() & IFF_UP) != 0;
}

bool InterfaceIterator::IsLoopback()
{
    return (GetFlags() & IFF_LOOPBACK) != 0;
}

bool InterfaceIterator::SupportsMulticast()
{
    return (GetFlags() & IFF_MULTICAST) != 0;
}

bool InterfaceIterator::HasBroadcastAddress()
{
    return (GetFlags() & IFF_BROADCAST) != 0;
}

short InterfaceIterator::GetFlags()
{
    struct ifreq intfData;

    if (!mIntfFlagsCached && HasCurrent())
    {
        Platform::CopyString(intfData.ifr_name, mIntfArray[mCurIntf].if_name);

        int res = ioctl(GetIOCTLSocket(), SIOCGIFFLAGS, &intfData);
        if (res == 0)
        {
            mIntfFlags       = intfData.ifr_flags;
            mIntfFlagsCached = true;
        }
#ifdef __MBED__
        CloseIOCTLSocket();
#endif
    }

    return mIntfFlags;
}

CHIP_ERROR InterfaceIterator::GetInterfaceType(InterfaceType & type)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR InterfaceIterator::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

InterfaceAddressIterator::InterfaceAddressIterator()
{
    mAddrsList = nullptr;
    mCurAddr   = nullptr;
}

InterfaceAddressIterator::~InterfaceAddressIterator()
{
    if (mAddrsList != nullptr)
    {
        freeifaddrs(mAddrsList);
        mAddrsList = mCurAddr = nullptr;
    }
}

bool InterfaceAddressIterator::HasCurrent()
{
    return (mAddrsList != nullptr) ? (mCurAddr != nullptr) : Next();
}

bool InterfaceAddressIterator::Next()
{
    while (true)
    {
        if (mAddrsList == nullptr)
        {
            int res = getifaddrs(&mAddrsList);
            if (res < 0)
            {
                return false;
            }
            mCurAddr = mAddrsList;
        }
        else if (mCurAddr != nullptr)
        {
            mCurAddr = mCurAddr->ifa_next;
        }

        if (mCurAddr == nullptr)
        {
            return false;
        }

        if (mCurAddr->ifa_addr != nullptr &&
            (mCurAddr->ifa_addr->sa_family == AF_INET6
#if INET_CONFIG_ENABLE_IPV4
             || mCurAddr->ifa_addr->sa_family == AF_INET
#endif // INET_CONFIG_ENABLE_IPV4
             ))
        {
            return true;
        }
    }
}

CHIP_ERROR InterfaceAddressIterator::GetAddress(IPAddress & outIPAddress)
{
    return HasCurrent() ? IPAddress::GetIPAddressFromSockAddr(*mCurAddr->ifa_addr, outIPAddress) : CHIP_ERROR_SENTINEL;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        if (mCurAddr->ifa_addr->sa_family == AF_INET6)
        {
#ifndef __MBED__
            struct sockaddr_in6 & netmask = *reinterpret_cast<struct sockaddr_in6 *>(mCurAddr->ifa_netmask);
            return NetmaskToPrefixLength(netmask.sin6_addr.s6_addr, 16);
#else  // __MBED__
       // netmask is not available through an API for IPv6 interface in Mbed.
       // Default prefix length to 64.
            return 64;
#endif // !__MBED__
        }
        if (mCurAddr->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in & netmask = *reinterpret_cast<struct sockaddr_in *>(mCurAddr->ifa_netmask);
            return NetmaskToPrefixLength(reinterpret_cast<const uint8_t *>(&netmask.sin_addr.s_addr), 4);
        }
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    return HasCurrent() ? InterfaceId(if_nametoindex(mCurAddr->ifa_name)) : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mCurAddr->ifa_name) < nameBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    Platform::CopyString(nameBuf, nameBufSize, mCurAddr->ifa_name);
    return CHIP_NO_ERROR;
}

bool InterfaceAddressIterator::IsUp()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_UP) != 0;
}

bool InterfaceAddressIterator::IsLoopback()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_LOOPBACK) != 0;
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_MULTICAST) != 0;
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_BROADCAST) != 0;
}

CHIP_ERROR InterfaceId::GetLinkLocalAddr(IPAddress * llAddr) const
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    struct ifaddrs * ifaddr;
    const int rv = getifaddrs(&ifaddr);
    bool found   = false;

    if (rv == -1)
    {
        return INET_ERROR_ADDRESS_NOT_FOUND;
    }

    for (struct ifaddrs * ifaddr_iter = ifaddr; ifaddr_iter != nullptr; ifaddr_iter = ifaddr_iter->ifa_next)
    {
        if (ifaddr_iter->ifa_addr != nullptr)
        {
            if ((ifaddr_iter->ifa_addr->sa_family == AF_INET6) &&
                ((mPlatformInterface == 0) || (mPlatformInterface == if_nametoindex(ifaddr_iter->ifa_name))))
            {
                struct in6_addr * sin6_addr = &(reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr;
                if ((sin6_addr->s6_addr[0] == 0xfe) && ((sin6_addr->s6_addr[1] & 0xc0) == 0x80)) // Link Local Address
                {
                    (*llAddr) = IPAddress((reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr);
                    found     = true;
                    break;
                }
            }
        }
    }
    freeifaddrs(ifaddr);

    return (found) ? CHIP_NO_ERROR : INET_ERROR_ADDRESS_NOT_FOUND;
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

CHIP_ERROR InterfaceId::GetInterfaceName(char * nameBuf, size_t nameBufSize) const
{
    if (mPlatformInterface)
    {
        net_if * currentInterface = net_if_get_by_index(mPlatformInterface);
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
        Platform::CopyString(nameBuf, nameBufSize, name);
        return CHIP_NO_ERROR;
    }
    if (nameBufSize < 1)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InterfaceId::InterfaceNameToId(const char * intfName, InterfaceId & interface)
{
    int currentId = 0;
    net_if * currentInterface;

    while ((currentInterface = net_if_get_by_index(++currentId)) != nullptr)
    {
        if (strcmp(net_if_get_device(currentInterface)->name, intfName) == 0)
        {
            interface = InterfaceId(currentId);
            return CHIP_NO_ERROR;
        }
    }
    interface = InterfaceId::Null();
    return INET_ERROR_UNKNOWN_INTERFACE;
}

InterfaceIterator::InterfaceIterator() : mCurrentInterface(net_if_get_by_index(mCurrentId)) {}

bool InterfaceIterator::HasCurrent(void)
{
    return mCurrentInterface != nullptr;
}

bool InterfaceIterator::Next()
{
    mCurrentInterface = net_if_get_by_index(++mCurrentId);
    return HasCurrent();
}

InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return HasCurrent() ? InterfaceId(mCurrentId) : InterfaceId::Null();
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return InterfaceId(mCurrentId).GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceIterator::IsUp()
{
    return HasCurrent() && net_if_is_up(mCurrentInterface);
}

bool InterfaceIterator::SupportsMulticast()
{
    return HasCurrent() && NET_IF_MAX_IPV6_MADDR > 0;
}

bool InterfaceIterator::HasBroadcastAddress()
{
    // Zephyr seems to handle broadcast address for IPv4 implicitly
    return HasCurrent() && INET_CONFIG_ENABLE_IPV4;
}

CHIP_ERROR InterfaceIterator::GetInterfaceType(InterfaceType & type)
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

CHIP_ERROR InterfaceIterator::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize)
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

InterfaceAddressIterator::InterfaceAddressIterator() = default;

bool InterfaceAddressIterator::HasCurrent()
{
    return mIntfIter.HasCurrent() && (mCurAddrIndex >= 0 || Next());
}

bool InterfaceAddressIterator::Next()
{
    while (mIntfIter.HasCurrent())
    {
        if (mCurAddrIndex == -1) // first address for the current interface
        {
            const net_if_config * config =
                net_if_get_config(net_if_get_by_index(mIntfIter.GetInterfaceId().GetPlatformInterface()));
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

CHIP_ERROR InterfaceAddressIterator::GetAddress(IPAddress & outIPAddress)
{
    if (HasCurrent())
    {
        outIPAddress = IPAddress(mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_SENTINEL;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        net_if * const iface              = net_if_get_by_index(mIntfIter.GetInterfaceId().GetPlatformInterface());
        net_if_ipv6_prefix * const prefix = net_if_ipv6_prefix_get(iface, &mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return prefix ? prefix->len : 128;
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    return HasCurrent() ? mIntfIter.GetInterfaceId() : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIterator::IsUp()
{
    return HasCurrent() && mIntfIter.IsUp();
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    return HasCurrent() && mIntfIter.SupportsMulticast();
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    return HasCurrent() && mIntfIter.HasBroadcastAddress();
}

CHIP_ERROR InterfaceId::GetLinkLocalAddr(IPAddress * llAddr) const
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    net_if * const iface = mPlatformInterface ? net_if_get_by_index(mPlatformInterface) : net_if_get_default();
    VerifyOrReturnError(iface != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    in6_addr * const ip6_addr = net_if_ipv6_get_ll(iface, NET_ADDR_PREFERRED);
    VerifyOrReturnError(ip6_addr != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    *llAddr = IPAddress(*ip6_addr);

    return CHIP_NO_ERROR;
}

#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

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
