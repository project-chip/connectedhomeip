/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *      Implementation of network interface abstraction layer.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdio.h>
#include <string.h>

#include <Weave/Support/CodeUtils.h>
#include <Weave/Support/NLDLLUtil.h>

#include <InetLayer/InetLayer.h>
#include <InetLayer/InetLayerEvents.h>
#include <InetLayer/IPPrefix.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#include <lwip/sys.h>
#include <lwip/netif.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */
#include <sys/ioctl.h>
#include <net/if.h>
#ifdef __ANDROID__
#include "ifaddrs-android.h"
#else // !defined(__ANDROID__)
#include <ifaddrs.h>
#endif // !defined(__ANDROID__)
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

namespace nl {
namespace Inet {


/**
 * @brief   Get the name of a network interface
 *
 * @param[in]   intfId      a network interface
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  INET_NO_ERROR           successful result, interface name written
 * @retval  INET_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf. The name of the unspecified network interface is the empty
 *     string.
 */
NL_DLL_EXPORT INET_ERROR GetInterfaceName(InterfaceId intfId, char *nameBuf, size_t nameBufSize)
{
    if (intfId != INET_NULL_INTERFACEID)
    {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        int status = snprintf(nameBuf, nameBufSize, "%c%c%d", intfId->name[0], intfId->name[1], intfId->num);
        if (status >= static_cast<int>(nameBufSize))
            return INET_ERROR_NO_MEMORY;
        return INET_NO_ERROR;
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        char intfName[IF_NAMESIZE];
        if (if_indextoname(intfId, intfName) == NULL)
            return Weave::System::MapErrorPOSIX(errno);
        if (strlen(intfName) >= nameBufSize)
            return INET_ERROR_NO_MEMORY;
        strcpy(nameBuf, intfName);
        return INET_NO_ERROR;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    }
    else
    {
        if (nameBufSize < 1)
            return INET_ERROR_NO_MEMORY;
        nameBuf[0] = 0;
        return INET_NO_ERROR;
    }
}

/**
 * @brief   Search the list of network interfaces for the indicated name.
 *
 * @param[in]   intfName    name of the network interface to find
 * @param[out]  intfId      indicator of the network interface to assign
 *
 * @retval  INET_NO_ERROR                 success, network interface indicated
 * @retval  INET_ERROR_UNKNOWN_INTERFACE  no network interface found
 * @retval  other                   another system or platform error
 *
 * @details
 *     On LwIP, this function must be called with the LwIP stack lock acquired.
 *
 *     The \c intfId parameter is not updated unless the value returned is
 *     \c INET_NO_ERROR. It should be initialized with \c INET_NULL_INTERFACEID
 *     before calling this function.
 */
NL_DLL_EXPORT INET_ERROR InterfaceNameToId(const char *intfName, InterfaceId& intfId)
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    if (strlen(intfName) < 3)
        return INET_ERROR_UNKNOWN_INTERFACE;
    char *parseEnd;
    unsigned long intfNum = strtoul(intfName + 2, &parseEnd, 10);
    if (*parseEnd != 0 || intfNum > UINT8_MAX)
        return INET_ERROR_UNKNOWN_INTERFACE;
    struct netif * intf;
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(intf)
#else
    for (intf = netif_list; intf != NULL; intf = intf->next)
#endif
    {
        if (intf->name[0] == intfName[0] && intf->name[1] == intfName[1] && intf->num == (uint8_t)intfNum)
        {
            intfId = intf;
            return INET_NO_ERROR;
        }
    }
    intfId = INET_NULL_INTERFACEID;
    return INET_ERROR_UNKNOWN_INTERFACE;
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    intfId = if_nametoindex(intfName);
    if (intfId == 0)
        return (errno == ENXIO) ? INET_ERROR_UNKNOWN_INTERFACE : Weave::System::MapErrorPOSIX(errno);
    return INET_NO_ERROR;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
}


#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

static int sIOCTLSocket = -1;

/**
 * @brief   Returns a global general purpose socket useful for invoking certain network IOCTLs.
 *
 * This function is thread-safe on all platforms.
 */
int GetIOCTLSocket(void)
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
void CloseIOCTLSocket(void)
{
    if (sIOCTLSocket == -1)
    {
        close(sIOCTLSocket);
        sIOCTLSocket = -1;
    }
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS


/**
 * @fn      InterfaceIterator::InterfaceIterator(void)
 *
 * @brief   Constructs an InterfaceIterator object.
 *
 * @details
 *     Starts the iterator at the first network interface. On some platforms,
 *     this constructor may allocate resources recycled by the destructor.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

InterfaceIterator::InterfaceIterator(void)
{
    mIntfArray = NULL;
    mCurIntf = 0;
    mIntfFlags = 0;
    mIntfFlagsCached = 0;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 * @fn      InterfaceIterator::~InterfaceIterator(void)
 *
 * @brief   Destroys an InterfaceIterator object.
 *
 * @details
 *     Recycles any resources allocated by the constructor.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

InterfaceIterator::~InterfaceIterator(void)
{
    if (mIntfArray != NULL)
    {
        if_freenameindex(mIntfArray);
        mIntfArray = NULL;
    }
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 * @fn      bool InterfaceIterator::HasCurrent(void)
 *
 * @brief   Test whether the iterator is positioned on an interface
 *
 * @return  \c true if the iterator is positioned on an interface;
 *          \c false if positioned beyond the end of the interface list.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

bool InterfaceIterator::HasCurrent(void)
{
    return (mIntfArray != NULL)
        ? mIntfArray[mCurIntf].if_index != 0
        : Next();
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 * @fn      bool InterfaceIterator::Next(void)
 *
 * @brief   Advance the iterator to the next network interface.
 *
 * @return  \c false if advanced beyond the end, else \c true.
 *
 * @details
 *     Advances the internal iterator to the next network interface or to a position
 *     beyond the end of the interface list.
 *
 *     On multi-threaded LwIP systems, this method is thread-safe relative to other
 *     threads accessing the global LwIP state provided that: 1) the other threads
 *     hold the LwIP core lock while mutating the list of netifs; and 2) netif objects
 *     themselves are never destroyed.
 *
 *     Iteration is stable in the face of changes to the underlying system's
 *     interfaces, *except* in the case of LwIP systems when the currently selected
 *     interface is removed from the list, which causes iteration to end immediately.
 */
bool InterfaceIterator::Next(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    if (mIntfArray == NULL)
    {
        mIntfArray = if_nameindex();
    }
    else if (mIntfArray[mCurIntf].if_index != 0)
    {
        mCurIntf++;
        mIntfFlags = 0;
        mIntfFlagsCached = false;
    }
    return (mIntfArray != NULL && mIntfArray[mCurIntf].if_index != 0);

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Verify the previous netif is still on the list if netifs.  If so,
    // advance to the next nextif.
    struct netif * prevNetif = mCurNetif;
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
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

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      InterfaceId InterfaceIterator::GetInterfaceId(void)
 *
 * @brief   Returns the network interface id at the current iterator position.
 *
 * @retval  INET_NULL_INTERFACEID   if advanced beyond the end of the list.
 * @retval  id                      the current network interface id.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return (HasCurrent())
        ? mIntfArray[mCurIntf].if_index
        : INET_NULL_INTERFACEID;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS


/**
 * @brief   Get the name of the current network interface
 *
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  INET_NO_ERROR           successful result, interface name written
 * @retval  INET_ERROR_INCORRECT_STATE
 *                                  iterator is positioned beyond the end of
 *                                  the list
 * @retval  INET_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf.
 */
INET_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    INET_ERROR err = INET_ERROR_NOT_IMPLEMENTED;

    VerifyOrExit(HasCurrent(), err = INET_ERROR_INCORRECT_STATE);

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    VerifyOrExit(strlen(mIntfArray[mCurIntf].if_name) < nameBufSize, err = INET_ERROR_NO_MEMORY);
    strncpy(nameBuf, mIntfArray[mCurIntf].if_name, nameBufSize);
    err = INET_NO_ERROR;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    err = ::nl::Inet::GetInterfaceName(mCurNetif, nameBuf, nameBufSize);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

exit:
    return err;
}

/**
 * @brief   Returns whether the current network interface is up.
 *
 * @return  \c true if current network interface is up, \c false if not
 *          or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::IsUp(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (GetFlags() & IFF_UP) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() && netif_is_up(mCurNetif);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @brief   Returns whether the current network interface supports multicast.
 *
 * @return  \c true if current network interface supports multicast, \c false
 *          if not, or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::SupportsMulticast(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (GetFlags() & IFF_MULTICAST) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() &&
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        (mCurNetif->flags & (NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST)) != 0;
#else
        (mCurNetif->flags & NETIF_FLAG_POINTTOPOINT) == 0;
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @brief   Returns whether the current network interface has a broadcast address.
 *
 * @return  \c true if current network interface has a broadcast address, \c false
 *          if not, or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::HasBroadcastAddress(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (GetFlags() & IFF_BROADCAST) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() && (mCurNetif->flags & NETIF_FLAG_BROADCAST) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      short InterfaceIterator::GetFlags(void)
 *
 * @brief   Returns the ifr_flags value for the current interface.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

short InterfaceIterator::GetFlags(void)
{
    struct ifreq intfData;

    if (!mIntfFlagsCached && HasCurrent())
    {
    	strncpy(intfData.ifr_name, mIntfArray[mCurIntf].if_name, IFNAMSIZ);
    	intfData.ifr_name[IFNAMSIZ-1] = '\0';

        int res = ioctl(GetIOCTLSocket(), SIOCGIFFLAGS, &intfData);
        if (res == 0)
        {
            mIntfFlags = intfData.ifr_flags;
            mIntfFlagsCached = true;
        }
    }

    return mIntfFlags;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 * @fn      InterfaceAddressIterator::InterfaceAddressIterator(void)
 *
 * @brief   Constructs an InterfaceAddressIterator object.
 *
 * @details
 *     Starts the iterator at the first network address. On some platforms,
 *     this constructor may allocate resources recycled by the destructor.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

InterfaceAddressIterator::InterfaceAddressIterator(void)
{
    mAddrsList = NULL;
    mCurAddr = NULL;
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS


/**
 * @fn      InterfaceAddressIterator::~InterfaceAddressIterator(void)
 *
 * @brief   Destroys an InterfaceAddressIterator object.
 *
 * @details
 *  Recycles any resources allocated by the constructor.
 */

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

InterfaceAddressIterator::~InterfaceAddressIterator(void)
{
    if (mAddrsList != NULL)
    {
        freeifaddrs(mAddrsList);
        mAddrsList = mCurAddr = NULL;
    }
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 * @fn      bool InterfaceIterator::HasCurrent(void)
 *
 * @brief   Test whether the iterator is positioned on an interface address
 *
 * @return  \c true if the iterator is positioned on an interface address;
 *          \c false if positioned beyond the end of the address list.
 */
bool InterfaceAddressIterator::HasCurrent(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    return (mAddrsList != NULL) ? (mCurAddr != NULL) : Next();

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    return mIntfIter.HasCurrent() && ((mCurAddrIndex != kBeforeStartIndex) || Next());

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      bool InterfaceAddressIterator::Next(void)
 *
 * @brief   Advance the iterator to the next interface address.
 *
 * @return  \c false if advanced beyond the end, else \c true.
 *
 * @details
 *     Advances the iterator to the next interface address or to a position
 *     beyond the end of the address list.
 *
 *     On LwIP, this method is thread-safe provided that: 1) other threads hold
 *     the LwIP core lock while mutating the netif list; and 2) netif objects
 *     themselves are never destroyed.  Additionally, iteration on LwIP systems
 *     will terminate early if the current interface is removed from the list.
 */
bool InterfaceAddressIterator::Next(void)
{
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    while (true)
    {
        if (mAddrsList == NULL)
        {
            int res = getifaddrs(&mAddrsList);
            if (res < 0)
            {
                return false;
            }
            mCurAddr = mAddrsList;
        }
        else if (mCurAddr != NULL)
        {
            mCurAddr = mCurAddr->ifa_next;
        }

        if (mCurAddr == NULL)
        {
            return false;
        }

        if (mCurAddr->ifa_addr != NULL &&
            (mCurAddr->ifa_addr->sa_family == AF_INET6
#if INET_CONFIG_ENABLE_IPV4
             || mCurAddr->ifa_addr->sa_family == AF_INET
#endif // INET_CONFIG_ENABLE_IPV4
             ))
        {
            return true;
        }
    }

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    mCurAddrIndex++;

    while (mIntfIter.HasCurrent())
    {
        struct netif * curIntf = mIntfIter.GetInterfaceId();

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

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      IPAddress InterfaceAddressIterator::GetAddress(void)
 *
 * @brief   Get the current interface address.
 *
 * @return  the current interface address or \c IPAddress::Any if the iterator
 *          is positioned beyond the end of the address list.
 */
IPAddress InterfaceAddressIterator::GetAddress(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        return IPAddress::FromSockAddr(*mCurAddr->ifa_addr);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        struct netif * curIntf = mIntfIter.GetInterfaceId();

        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return IPAddress::FromIPv6(*netif_ip6_addr(curIntf, mCurAddrIndex));
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            return IPAddress::FromIPv4(*netif_ip4_addr(curIntf));
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }

    return IPAddress::Any;
}

/**
 * @fn      uint8_t InterfaceAddressIterator::GetPrefixLength(void)
 *
 * @brief   Gets the network prefix associated with the current interface address.
 *
 * @return  the network prefix (in bits) or 0 if the iterator is positioned beyond
 *          the end of the address list.
 *
 * @details
 *     On LwIP, this method simply returns the hard-coded constant 64.
 *
 *     Note Well: the standard subnet prefix on all links other than PPP
 *     links is 64 bits. On PPP links and some non-broadcast multipoint access
 *     links, the convention is either 127 bits or 128 bits, but it might be
 *     something else. On most platforms, the system's interface address
 *     structure can represent arbitrary prefix lengths between 0 and 128.
 */
uint8_t InterfaceAddressIterator::GetPrefixLength(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    	if (mCurAddr->ifa_addr->sa_family == AF_INET6)
    	{
    		struct sockaddr_in6& netmask = *(struct sockaddr_in6 *)(mCurAddr->ifa_netmask);
    		return NetmaskToPrefixLength(netmask.sin6_addr.s6_addr, 16);
    	}
    	if (mCurAddr->ifa_addr->sa_family == AF_INET)
    	{
    		struct sockaddr_in& netmask = *(struct sockaddr_in *)(mCurAddr->ifa_netmask);
    		return NetmaskToPrefixLength((const uint8_t *)&netmask.sin_addr.s_addr, 4);
    	}
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return 64;
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            struct netif * curIntf = mIntfIter.GetInterfaceId();
            return NetmaskToPrefixLength((const uint8_t *)netif_ip4_netmask(curIntf), 4);
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }
    return 0;
}

/**
 * @fn      InterfaceId InterfaceAddressIterator::GetInterfaceId(void)
 *
 * @brief   Returns the network interface id associated with the current
 *          interface address.
 *
 * @return  the interface id or \c INET_NULL_INTERFACEID if the iterator
 *          is positioned beyond the end of the address list.
 */
InterfaceId InterfaceAddressIterator::GetInterfaceId(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        return if_nametoindex(mCurAddr->ifa_name);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        return mIntfIter.GetInterfaceId();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }
    return INET_NULL_INTERFACEID;
}

/**
 * @fn      INET_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
 *
 * @brief   Get the name of the network interface associated with the
 *          current interface address.
 *
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  INET_NO_ERROR           successful result, interface name written
 * @retval  INET_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  INET_ERROR_INCORRECT_STATE
 *                                  the iterator is not currently positioned on an
 *                                  interface address
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf.
 */
INET_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    INET_ERROR err = INET_ERROR_NOT_IMPLEMENTED;

    VerifyOrExit(HasCurrent(), err = INET_ERROR_INCORRECT_STATE);

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    VerifyOrExit(strlen(mCurAddr->ifa_name) < nameBufSize, err = INET_ERROR_NO_MEMORY);
    strncpy(nameBuf, mCurAddr->ifa_name, nameBufSize);
    err = INET_NO_ERROR;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    err = mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

exit:
    return err;
}

/**
 * @fn      bool InterfaceAddressIterator::IsUp(void)
 *
 * @brief   Returns whether the network interface associated with the current
 *          interface address is up.
 *
 * @return  \c true if current network interface is up, \c false if not, or
 *          if the iterator is not positioned on an interface address.
 */
bool InterfaceAddressIterator::IsUp(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        return (mCurAddr->ifa_flags & IFF_UP) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        return mIntfIter.IsUp();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }
    return false;
}

/**
 * @fn      bool InterfaceAddressIterator::SupportsMulticast(void)
 *
 * @brief   Returns whether the network interface associated with the current
 *          interface address supports multicast.
 *
 * @return  \c true if multicast is supported, \c false if not, or
 *          if the iterator is not positioned on an interface address.
 */
bool InterfaceAddressIterator::SupportsMulticast(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        return (mCurAddr->ifa_flags & IFF_MULTICAST) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        return mIntfIter.SupportsMulticast();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }
    return false;
}

/**
 * @fn      bool InterfaceAddressIterator::HasBroadcastAddress(void)
 *
 * @brief   Returns whether the network interface associated with the current
 *          interface address has an IPv4 broadcast address.
 *
 * @return  \c true if the interface has a broadcast address, \c false if not, or
 *          if the iterator is not positioned on an interface address.
 */
bool InterfaceAddressIterator::HasBroadcastAddress(void)
{
    if (HasCurrent())
    {
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    	return (mCurAddr->ifa_flags & IFF_BROADCAST) != 0;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    	return mIntfIter.HasBroadcastAddress();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    }
    return false;
}

/**
 * @fn       void InterfaceAddressIterator::GetAddressWithPrefix(IPPrefix & addrWithPrefix)
 *
 * @brief    Returns an IPPrefix containing the address and prefix length
 *           for the current address.
 */
void InterfaceAddressIterator::GetAddressWithPrefix(IPPrefix & addrWithPrefix)
{
	if (HasCurrent())
	{
		addrWithPrefix.IPAddr = GetAddress();
		addrWithPrefix.Length = GetPrefixLength();
	}
	else
	{
		addrWithPrefix = IPPrefix::Zero;
	}
}

/**
 * @fn       uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen)
 *
 * @brief    Compute a prefix length from a variable-length netmask.
 */
uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen)
{
	uint8_t prefixLen = 0;

	for (uint8_t i = 0; i < netmaskLen; i++, prefixLen += 8)
    {
        uint8_t b = netmask[i];
        if (b != 0xFF)
        {
            if ((b & 0xF0) == 0xF0)
                prefixLen += 4;
            else
                b = b >> 4;

            if ((b & 0x0C) == 0x0C)
                prefixLen += 2;
            else
                b = b >> 2;

            if ((b & 0x02) == 0x02)
                prefixLen++;

            break;
        }
    }

	return prefixLen;
}

} // namespace Inet
} // namespace nl
