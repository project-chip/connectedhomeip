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

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */

namespace chip {
namespace Inet {

namespace PlatformNetworkInterface {

CHIP_ERROR GetInterfaceName(unsigned int interface, char * nameBuf, size_t nameBufSize)
{
    if (interface)
    {
        char intfName[IF_NAMESIZE];
        if (if_indextoname(interface, intfName) == nullptr)
        {
            return CHIP_ERROR_POSIX(errno);
        }
        size_t nameLength = strlen(intfName);
        if (nameLength >= nameBufSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        strncpy(nameBuf, intfName, nameLength + 1);
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
    unsigned int intfId = if_nametoindex(intfName);
    if (intfId == 0)
    {
        interface = InterfaceId::Null();
        return (errno == ENXIO) ? INET_ERROR_UNKNOWN_INTERFACE : CHIP_ERROR_POSIX(errno);
    }
    interface = ToInterfaceId(intfId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetLinkLocalAddr(unsigned int interface, IPAddress * llAddr)
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    struct ifaddrs * ifaddr;
    const int rv = getifaddrs(&ifaddr);
    if (rv == -1)
    {
        return INET_ERROR_ADDRESS_NOT_FOUND;
    }

    for (struct ifaddrs * ifaddr_iter = ifaddr; ifaddr_iter != nullptr; ifaddr_iter = ifaddr_iter->ifa_next)
    {
        if (ifaddr_iter->ifa_addr != nullptr)
        {
            if ((ifaddr_iter->ifa_addr->sa_family == AF_INET6) &&
                ((interface == 0) || (interface == if_nametoindex(ifaddr_iter->ifa_name))))
            {
                struct in6_addr * sin6_addr = &(reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr;
                if (sin6_addr->s6_addr[0] == 0xfe && (sin6_addr->s6_addr[1] & 0xc0) == 0x80) // Link Local Address
                {
                    (*llAddr) = IPAddress((reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr);
                    break;
                }
            }
        }
    }
    freeifaddrs(ifaddr);

    return CHIP_NO_ERROR;
}

} // namespace PlatformNetworkInterface

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

InterfaceIteratorImplIFAddrs::InterfaceIteratorImplIFAddrs()
{
    mIntfArray       = nullptr;
    mCurIntf         = 0;
    mIntfFlags       = 0;
    mIntfFlagsCached = false;
}

InterfaceIteratorImplIFAddrs::~InterfaceIteratorImplIFAddrs()
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

bool InterfaceIteratorImplIFAddrs::HasCurrent()
{
    return (mIntfArray != nullptr) ? mIntfArray[mCurIntf].if_index != 0 : Next();
}

bool InterfaceIteratorImplIFAddrs::Next()
{
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
}

InterfaceId InterfaceIteratorImplIFAddrs::GetInterfaceId()
{
    return HasCurrent() ? PlatformNetworkInterface::ToInterfaceId(mIntfArray[mCurIntf].if_index) : InterfaceId::Null();
}

CHIP_ERROR InterfaceIteratorImplIFAddrs::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mIntfArray[mCurIntf].if_name) < nameBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    strncpy(nameBuf, mIntfArray[mCurIntf].if_name, nameBufSize);
    return CHIP_NO_ERROR;
}

bool InterfaceIteratorImplIFAddrs::IsUp()
{
    return (GetFlags() & IFF_UP) != 0;
}

bool InterfaceIteratorImplIFAddrs::SupportsMulticast()
{
    return (GetFlags() & IFF_MULTICAST) != 0;
}

bool InterfaceIteratorImplIFAddrs::HasBroadcastAddress()
{
    return (GetFlags() & IFF_BROADCAST) != 0;
}

short InterfaceIteratorImplIFAddrs::GetFlags()
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
#if __MBED__
        CloseIOCTLSocket();
#endif
    }

    return mIntfFlags;
}

CHIP_ERROR InterfaceIteratorImplIFAddrs::GetInterfaceType(InterfaceType & type)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR InterfaceIteratorImplIFAddrs::GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize,
                                                            uint8_t addressBufferSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

InterfaceAddressIteratorImplIFAddrs::InterfaceAddressIteratorImplIFAddrs()
{
    mAddrsList = nullptr;
    mCurAddr   = nullptr;
}

InterfaceAddressIteratorImplIFAddrs::~InterfaceAddressIteratorImplIFAddrs()
{
    if (mAddrsList != nullptr)
    {
        freeifaddrs(mAddrsList);
        mAddrsList = mCurAddr = nullptr;
    }
}

bool InterfaceAddressIteratorImplIFAddrs::HasCurrent()
{
    return (mAddrsList != nullptr) ? (mCurAddr != nullptr) : Next();
}

bool InterfaceAddressIteratorImplIFAddrs::Next()
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

CHIP_ERROR InterfaceAddressIteratorImplIFAddrs::GetAddress(IPAddress & outIPAddress)
{
    return HasCurrent() ? IPAddress::GetIPAddressFromSockAddr(*mCurAddr->ifa_addr, outIPAddress) : CHIP_ERROR_SENTINEL;
}

uint8_t InterfaceAddressIteratorImplIFAddrs::GetPrefixLength()
{
    if (HasCurrent())
    {
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
    }
    return 0;
}

InterfaceId InterfaceAddressIteratorImplIFAddrs::GetInterfaceId()
{
    return HasCurrent() ? PlatformNetworkInterface::ToInterfaceId(if_nametoindex(mCurAddr->ifa_name)) : InterfaceId::Null();
}

CHIP_ERROR InterfaceAddressIteratorImplIFAddrs::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mCurAddr->ifa_name) < nameBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    strncpy(nameBuf, mCurAddr->ifa_name, nameBufSize);
    return CHIP_NO_ERROR;
}

bool InterfaceAddressIteratorImplIFAddrs::IsUp()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_UP) != 0;
}

bool InterfaceAddressIteratorImplIFAddrs::SupportsMulticast()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_MULTICAST) != 0;
}

bool InterfaceAddressIteratorImplIFAddrs::HasBroadcastAddress()
{
    return HasCurrent() && (mCurAddr->ifa_flags & IFF_BROADCAST) != 0;
}

} // namespace Inet
} // namespace chip
