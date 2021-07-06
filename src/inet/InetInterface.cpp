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

#include "InetInterface.h"

#include "InetLayer.h"
#include "InetLayerEvents.h"

#include <support/CHIPMemString.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
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
#include <net/if.h>
#include <sys/ioctl.h>
#ifdef __ANDROID__
#include "ifaddrs-android.h"
#else // !defined(__ANDROID__)
#include <ifaddrs.h>
#endif // !defined(__ANDROID__)
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <net/net_if.h>
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Inet {

/**
 * @brief   Get the name of a network interface
 *
 * @param[in]   intfId      a network interface
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  CHIP_NO_ERROR           successful result, interface name written
 * @retval  CHIP_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf. The name of the unspecified network interface is the empty
 *     string.
 */
DLL_EXPORT CHIP_ERROR GetInterfaceName(InterfaceId intfId, char * nameBuf, size_t nameBufSize)
{
    if (intfId != INET_NULL_INTERFACEID)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        int status = snprintf(nameBuf, nameBufSize, "%c%c%d", intfId->name[0], intfId->name[1], intfId->num);
        if (status >= static_cast<int>(nameBufSize))
            return CHIP_ERROR_NO_MEMORY;
        return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        char intfName[IF_NAMESIZE];
        if (if_indextoname(intfId, intfName) == nullptr)
            return chip::System::MapErrorPOSIX(errno);
        if (strlen(intfName) >= nameBufSize)
            return CHIP_ERROR_NO_MEMORY;
        strcpy(nameBuf, intfName);
        return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        net_if * currentInterface = net_if_get_by_index(intfId);
        if (!currentInterface)
            return CHIP_ERROR_INCORRECT_STATE;
        const char * name = net_if_get_device(currentInterface)->name;
        if (strlen(name) >= nameBufSize)
            return CHIP_ERROR_NO_MEMORY;
        strcpy(nameBuf, name);
        return CHIP_NO_ERROR;

#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    }

    if (nameBufSize < 1)
        return CHIP_ERROR_NO_MEMORY;

    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

/**
 * @brief   Search the list of network interfaces for the indicated name.
 *
 * @param[in]   intfName    name of the network interface to find
 * @param[out]  intfId      indicator of the network interface to assign
 *
 * @retval  CHIP_NO_ERROR                 success, network interface indicated
 * @retval  INET_ERROR_UNKNOWN_INTERFACE  no network interface found
 * @retval  other                   another system or platform error
 *
 * @details
 *     On LwIP, this function must be called with the LwIP stack lock acquired.
 *
 *     The \c intfId parameter is not updated unless the value returned is
 *     \c CHIP_NO_ERROR. It should be initialized with \c INET_NULL_INTERFACEID
 *     before calling this function.
 */
DLL_EXPORT CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & intfId)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (strlen(intfName) < 3)
        return INET_ERROR_UNKNOWN_INTERFACE;
    char * parseEnd;
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
        if (intf->name[0] == intfName[0] && intf->name[1] == intfName[1] && intf->num == (uint8_t) intfNum)
        {
            intfId = intf;
            return CHIP_NO_ERROR;
        }
    }
    intfId = INET_NULL_INTERFACEID;
    return INET_ERROR_UNKNOWN_INTERFACE;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    intfId = if_nametoindex(intfName);
    if (intfId == 0)
        return (errno == ENXIO) ? INET_ERROR_UNKNOWN_INTERFACE : chip::System::MapErrorPOSIX(errno);
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    int currentId = 0;
    net_if * currentInterface;

    while ((currentInterface = net_if_get_by_index(++currentId)) != nullptr)
    {
        if (strcmp(net_if_get_device(currentInterface)->name, intfName) == 0)
        {
            intfId = currentId;
            return CHIP_NO_ERROR;
        }
    }
    return INET_ERROR_UNKNOWN_INTERFACE;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
} // namespace Inet

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

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
    if (sIOCTLSocket == -1)
    {
        close(sIOCTLSocket);
        sIOCTLSocket = -1;
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

/**
 * @fn      InterfaceIterator::InterfaceIterator(void)
 *
 * @brief   Constructs an InterfaceIterator object.
 *
 * @details
 *     Starts the iterator at the first network interface. On some platforms,
 *     this constructor may allocate resources recycled by the destructor.
 */

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if __ANDROID__ && __ANDROID_API__ < 24

static struct if_nameindex * backport_if_nameindex(void);
static void backport_if_freenameindex(struct if_nameindex *);

static void backport_if_freenameindex(struct if_nameindex * inArray)
{
    if (inArray == NULL)
    {
        return;
    }

    for (size_t i = 0; inArray[i].if_index != 0; i++)
    {
        if (inArray[i].if_name != NULL)
        {
            free(inArray[i].if_name);
        }
    }

    free(inArray);
}

static struct if_nameindex * backport_if_nameindex(void)
{
    int err;
    unsigned index;
    size_t intfIter              = 0;
    size_t maxIntfNum            = 0;
    size_t numIntf               = 0;
    size_t numAddrs              = 0;
    struct if_nameindex * retval = NULL;
    struct if_nameindex * tmpval = NULL;
    struct ifaddrs * addrList    = NULL;
    struct ifaddrs * addrIter    = NULL;
    const char * lastIntfName    = "";

    err = getifaddrs(&addrList);
    VerifyOrExit(err >= 0, );

    // coalesce on consecutive interface names
    for (addrIter = addrList; addrIter != NULL; addrIter = addrIter->ifa_next)
    {
        numAddrs++;
        if (strcmp(addrIter->ifa_name, lastIntfName) == 0)
        {
            continue;
        }
        numIntf++;
        lastIntfName = addrIter->ifa_name;
    }

    tmpval = (struct if_nameindex *) malloc((numIntf + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(tmpval != NULL, );
    memset(tmpval, 0, (numIntf + 1) * sizeof(struct if_nameindex));

    lastIntfName = "";
    for (addrIter = addrList; addrIter != NULL; addrIter = addrIter->ifa_next)
    {
        if (strcmp(addrIter->ifa_name, lastIntfName) == 0)
        {
            continue;
        }

        index = if_nametoindex(addrIter->ifa_name);
        if (index != 0)
        {
            tmpval[intfIter].if_index = index;
            tmpval[intfIter].if_name  = strdup(addrIter->ifa_name);
            intfIter++;
        }
        lastIntfName = addrIter->ifa_name;
    }

    // coalesce on interface index
    maxIntfNum = 0;
    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        if (maxIntfNum < tmpval[i].if_index)
        {
            maxIntfNum = tmpval[i].if_index;
        }
    }

    retval = (struct if_nameindex *) malloc((maxIntfNum + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(retval != NULL, );
    memset(retval, 0, (maxIntfNum + 1) * sizeof(struct if_nameindex));

    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        struct if_nameindex * intf = &tmpval[i];
        if (retval[intf->if_index - 1].if_index == 0)
        {
            retval[intf->if_index - 1] = *intf;
        }
        else
        {
            free(intf->if_name);
            intf->if_index = 0;
            intf->if_name  = 0;
        }
    }

    intfIter = 0;

    // coalesce potential gaps between indeces
    for (size_t i = 0; i < maxIntfNum; i++)
    {
        if (retval[i].if_index != 0)
        {
            retval[intfIter] = retval[i];
            intfIter++;
        }
    }

    for (size_t i = intfIter; i < maxIntfNum; i++)
    {
        retval[i].if_index = 0;
        retval[i].if_name  = NULL;
    }

exit:
    if (tmpval != NULL)
    {
        free(tmpval);
    }

    if (addrList != NULL)
    {
        freeifaddrs(addrList);
    }

    return retval;
}

#endif // __ANDROID__ && __ANDROID_API__ < 24

InterfaceIterator::InterfaceIterator()
{
    mIntfArray       = nullptr;
    mCurIntf         = 0;
    mIntfFlags       = 0;
    mIntfFlagsCached = false;
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
InterfaceIterator::InterfaceIterator() : mCurrentInterface(net_if_get_by_index(mCurrentId)) {}
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 * @fn      InterfaceIterator::~InterfaceIterator(void)
 *
 * @brief   Destroys an InterfaceIterator object.
 *
 * @details
 *     Recycles any resources allocated by the constructor.
 */

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

InterfaceIterator::~InterfaceIterator()
{
    if (mIntfArray != nullptr)
    {
#if __ANDROID__ && __ANDROID_API__ < 24
        backport_if_freenameindex(mIntfArray);
#else
        if_freenameindex(mIntfArray);
#endif
        mIntfArray = nullptr;
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

/**
 * @fn      bool InterfaceIterator::HasCurrent(void)
 *
 * @brief   Test whether the iterator is positioned on an interface
 *
 * @return  \c true if the iterator is positioned on an interface;
 *          \c false if positioned beyond the end of the interface list.
 */

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
bool InterfaceIterator::HasCurrent()
{
    return (mIntfArray != nullptr) ? mIntfArray[mCurIntf].if_index != 0 : Next();
}
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
bool InterfaceIterator::HasCurrent(void)
{
    return mCurrentInterface != nullptr;
}
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

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
bool InterfaceIterator::Next()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

    if (mIntfArray == nullptr)
    {
#if __ANDROID__ && __ANDROID_API__ < 24
        mIntfArray = backport_if_nameindex();
#else
        mIntfArray = if_nameindex();
#endif
    }
    else if (mIntfArray[mCurIntf].if_index != 0)
    {
        mCurIntf++;
        mIntfFlags       = 0;
        mIntfFlagsCached = false;
    }
    return (mIntfArray != nullptr && mIntfArray[mCurIntf].if_index != 0);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    mCurrentInterface = net_if_get_by_index(++mCurrentId);
    return HasCurrent();
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP

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

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      InterfaceId InterfaceIterator::GetInterfaceId(void)
 *
 * @brief   Returns the network interface id at the current iterator position.
 *
 * @retval  INET_NULL_INTERFACEID   if advanced beyond the end of the list.
 * @retval  id                      the current network interface id.
 */

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
InterfaceId InterfaceIterator::GetInterfaceId()
{
    return (HasCurrent()) ? mIntfArray[mCurIntf].if_index : INET_NULL_INTERFACEID;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return HasCurrent() ? mCurrentId : INET_NULL_INTERFACEID;
}
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 * @brief   Get the name of the current network interface
 *
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  CHIP_NO_ERROR           successful result, interface name written
 * @retval  CHIP_ERROR_INCORRECT_STATE
 *                                  iterator is positioned beyond the end of
 *                                  the list
 * @retval  CHIP_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf.
 */
CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    VerifyOrReturnError(strlen(mIntfArray[mCurIntf].if_name) < nameBufSize, CHIP_ERROR_NO_MEMORY);
    strncpy(nameBuf, mIntfArray[mCurIntf].if_name, nameBufSize);
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    return ::chip::Inet::GetInterfaceName(mCurrentId, nameBuf, nameBufSize);
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return ::chip::Inet::GetInterfaceName(mCurNetif, nameBuf, nameBufSize);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief   Returns whether the current network interface is up.
 *
 * @return  \c true if current network interface is up, \c false if not
 *          or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::IsUp()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    return (GetFlags() & IFF_UP) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    return HasCurrent() && net_if_is_up(mCurrentInterface);
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() && netif_is_up(mCurNetif);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @brief   Returns whether the current network interface supports multicast.
 *
 * @return  \c true if current network interface supports multicast, \c false
 *          if not, or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::SupportsMulticast()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    return (GetFlags() & IFF_MULTICAST) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    return HasCurrent() && NET_IF_MAX_IPV6_MADDR > 0;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() &&
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        (mCurNetif->flags & (NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST)) != 0;
#else
        (mCurNetif->flags & NETIF_FLAG_POINTTOPOINT) == 0;
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @brief   Returns whether the current network interface has a broadcast address.
 *
 * @return  \c true if current network interface has a broadcast address, \c false
 *          if not, or if the iterator is positioned beyond the end of the list.
 */
bool InterfaceIterator::HasBroadcastAddress()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    return (GetFlags() & IFF_BROADCAST) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    // Zephyr seems to handle broadcast address for IPv4 implicitly
    return HasCurrent() && INET_CONFIG_ENABLE_IPV4;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return HasCurrent() && (mCurNetif->flags & NETIF_FLAG_BROADCAST) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

/**
 * @fn      short InterfaceIterator::GetFlags(void)
 *
 * @brief   Returns the ifr_flags value for the current interface.
 */
short InterfaceIterator::GetFlags()
{
    struct ifreq intfData;

    if (!mIntfFlagsCached && HasCurrent())
    {
        strncpy(intfData.ifr_name, mIntfArray[mCurIntf].if_name, IFNAMSIZ);
        intfData.ifr_name[IFNAMSIZ - 1] = '\0';

        int res = ioctl(GetIOCTLSocket(), SIOCGIFFLAGS, &intfData);
        if (res == 0)
        {
            mIntfFlags       = intfData.ifr_flags;
            mIntfFlagsCached = true;
        }
    }

    return mIntfFlags;
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
/**
 * @fn      InterfaceAddressIterator::InterfaceAddressIterator(void)
 *
 * @brief   Constructs an InterfaceAddressIterator object.
 *
 * @details
 *     Starts the iterator at the first network address. On some platforms,
 *     this constructor may allocate resources recycled by the destructor.
 */
InterfaceAddressIterator::InterfaceAddressIterator()
{
    mAddrsList = nullptr;
    mCurAddr   = nullptr;
}
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
InterfaceAddressIterator::InterfaceAddressIterator() = default;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 * @fn      InterfaceAddressIterator::~InterfaceAddressIterator(void)
 *
 * @brief   Destroys an InterfaceAddressIterator object.
 *
 * @details
 *  Recycles any resources allocated by the constructor.
 */

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
InterfaceAddressIterator::~InterfaceAddressIterator()
{
    if (mAddrsList != nullptr)
    {
        freeifaddrs(mAddrsList);
        mAddrsList = mCurAddr = nullptr;
    }
}
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

/**
 * @fn      bool InterfaceIterator::HasCurrent(void)
 *
 * @brief   Test whether the iterator is positioned on an interface address
 *
 * @return  \c true if the iterator is positioned on an interface address;
 *          \c false if positioned beyond the end of the address list.
 */
bool InterfaceAddressIterator::HasCurrent()
{
#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    return (mAddrsList != nullptr) ? (mCurAddr != nullptr) : Next();
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    return mIntfIter.HasCurrent() && (mCurAddrIndex >= 0 || Next());
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return mIntfIter.HasCurrent() && ((mCurAddrIndex != kBeforeStartIndex) || Next());
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
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
bool InterfaceAddressIterator::Next()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
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
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    while (mIntfIter.HasCurrent())
    {
        if (mCurAddrIndex == -1) // first address for the current interface
        {
            const net_if_config * config = net_if_get_config(net_if_get_by_index(mIntfIter.GetInterfaceId()));
            mIpv6                        = config->ip.ipv6;
        }

        while (++mCurAddrIndex < NET_IF_MAX_IPV6_ADDR)
            if (mIpv6->unicast[mCurAddrIndex].is_used)
                return true;

        mCurAddrIndex = -1;
        mIntfIter.Next();
    }

    return false;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
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
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @fn      IPAddress InterfaceAddressIterator::GetAddress(void)
 *
 * @brief   Get the current interface address.
 *
 * @return  the current interface address or \c IPAddress::Any if the iterator
 *          is positioned beyond the end of the address list.
 */
IPAddress InterfaceAddressIterator::GetAddress()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        return IPAddress::FromSockAddr(*mCurAddr->ifa_addr);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        return IPAddress::FromIPv6(mIpv6->unicast[mCurAddrIndex].address.in6_addr);
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
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
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
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
uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        if (mCurAddr->ifa_addr->sa_family == AF_INET6)
        {
#if !__MBED__
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
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        net_if * const iface              = net_if_get_by_index(mIntfIter.GetInterfaceId());
        net_if_ipv6_prefix * const prefix = net_if_ipv6_prefix_get(iface, &mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return prefix ? prefix->len : 128;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return 64;
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            struct netif * curIntf = mIntfIter.GetInterfaceId();
            return NetmaskToPrefixLength((const uint8_t *) netif_ip4_netmask(curIntf), 4);
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
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
InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        return if_nametoindex(mCurAddr->ifa_name);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        return mIntfIter.GetInterfaceId();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    }
    return INET_NULL_INTERFACEID;
}

/**
 * @fn      CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
 *
 * @brief   Get the name of the network interface associated with the
 *          current interface address.
 *
 * @param[in]   nameBuf     region of memory to write the interface name
 * @param[in]   nameBufSize size of the region denoted by \c nameBuf
 *
 * @retval  CHIP_NO_ERROR           successful result, interface name written
 * @retval  CHIP_ERROR_NO_MEMORY    name is too large to be written in buffer
 * @retval  CHIP_ERROR_INCORRECT_STATE
 *                                  the iterator is not currently positioned on an
 *                                  interface address
 * @retval  other                   another system or platform error
 *
 * @details
 *     Writes the name of the network interface as \c NUL terminated text string
 *     at \c nameBuf.
 */
CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    VerifyOrReturnError(strlen(mCurAddr->ifa_name) < nameBufSize, CHIP_ERROR_NO_MEMORY);
    strncpy(nameBuf, mCurAddr->ifa_name, nameBufSize);
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

    return CHIP_ERROR_NOT_IMPLEMENTED;
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
bool InterfaceAddressIterator::IsUp()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        return (mCurAddr->ifa_flags & IFF_UP) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        return mIntfIter.IsUp();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
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
bool InterfaceAddressIterator::SupportsMulticast()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        return (mCurAddr->ifa_flags & IFF_MULTICAST) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        return mIntfIter.SupportsMulticast();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
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
bool InterfaceAddressIterator::HasBroadcastAddress()
{
    if (HasCurrent())
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
        return (mCurAddr->ifa_flags & IFF_BROADCAST) != 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
        return mIntfIter.HasBroadcastAddress();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
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
