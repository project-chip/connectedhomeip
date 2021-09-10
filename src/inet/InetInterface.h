/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *  This file defines the <tt>Inet::InterfaceId</tt> type alias and related
 *  classes for iterating on the list of system network interfaces and the list
 *  of system interface addresses.
 */

#pragma once

#include <inet/InetConfig.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <lib/support/DLLUtil.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/netif.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
struct if_nameindex;
struct ifaddrs;
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <device.h>

struct net_if;
struct net_if_ipv4;
struct net_if_ipv6;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace Inet {

class IPAddress;
class IPPrefix;

/**
 * @typedef     InterfaceId
 *
 * @brief       Indicator for system network interfaces.
 *
 * @details
 *  Portability depends on never witnessing this alias. It may be replaced by a
 *  concrete opaque class in the future.
 *
 *  Note Well: The term "interface identifier" also conventionally refers to
 *  the lower 64 bits of an IPv6 address in all the relevant IETF standards
 *  documents, where the abbreviation "IID" is often used. In this text, the
 *  term "interface indicator" refers to values of this type alias.
 */

#if CHIP_SYSTEM_CONFIG_USE_LWIP
typedef struct netif * InterfaceId;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
typedef unsigned InterfaceId;
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
typedef int InterfaceId;
#endif

/**
 * @def     INET_NULL_INTERFACEID
 *
 * @brief   The distinguished value indicating no network interface.
 *
 * @details
 *  Note Well: This is not the indicator of a "null" network interface. This
 *  value can be used to indicate the absence of a specific network interface,
 *  or to specify that any applicable network interface is acceptable. Usage
 *  varies depending on context.
 */

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define INET_NULL_INTERFACEID NULL
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#define INET_NULL_INTERFACEID 0
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 * @brief   Test \c ID for inequivalence with \c INET_NULL_INTERFACEID
 *
 * @details
 *  This macro resolves to an expression that evaluates \c false if the
 *  argument is equivalent to \c INET_NULL_INTERFACEID and \c true otherwise.
 */
#define IsInterfaceIdPresent(intfId) ((intfId) != INET_NULL_INTERFACEID)

extern CHIP_ERROR GetInterfaceName(InterfaceId intfId, char * nameBuf, size_t nameBufSize);
extern CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & intfId);
extern uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen);

/**
 * @brief   Iterator for the list of system network interfaces.
 *
 * @details
 *  Use objects of this class to iterate the list of system network interfaces.
 *
 *  Methods on an individual instance of this class are *not* thread-safe;
 *  however separate instances may be used simultaneously by multiple threads.
 *
 *  On multi-threaded LwIP systems, instances are thread-safe relative to other
 *  threads accessing the global LwIP state provided that the other threads hold
 *  the LwIP core lock while mutating the list of netifs, and that netif object
 *  themselves are never destroyed.
 *
 *  On sockets-based systems, iteration is always stable in the face of changes
 *  to the underlying system's interfaces.
 *
 *  On LwIP systems, iteration is stable except in the case where the currently
 *  selected interface is removed from the list, in which case iteration ends
 *  immediately.
 */
class InterfaceIterator
{
public:
    InterfaceIterator();
    ~InterfaceIterator();

    bool HasCurrent();
    bool Next();
    InterfaceId GetInterface();
    InterfaceId GetInterfaceId();
    CHIP_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize);
    bool IsUp();
    bool SupportsMulticast();
    bool HasBroadcastAddress();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static constexpr size_t kMaxIfNameLength = 13; // Names are formatted as %c%c%d
#elif CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    static constexpr size_t kMaxIfNameLength = IF_NAMESIZE;
#elif CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    static constexpr size_t kMaxIfNameLength = Z_DEVICE_MAX_NAME_LEN;
#elif defined(IFNAMSIZ)
    static constexpr size_t kMaxIfNameLength = IFNAMSIZ;
#else
    // No constant available here - set some reasonable size
    static constexpr size_t kMaxIfNameLength = 33;
#endif

protected:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    struct netif * mCurNetif;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    struct if_nameindex * mIntfArray;
    size_t mCurIntf;
    short mIntfFlags;
    bool mIntfFlagsCached;

    short GetFlags();
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    InterfaceId mCurrentId     = 1;
    net_if * mCurrentInterface = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
};

/**
 * @brief   Iterator for the list of system network interface IP addresses.
 *
 * @details
 *  Use objects of this class to iterate the list of system network interface
 *  interface IP addresses.
 *
 *  Methods on an individual instance of this class are *not* thread-safe;
 *  however separate instances may be used simultaneously by multiple threads.
 *
 *  On multi-threaded LwIP systems, instances are thread-safe relative to other
 *  threads accessing the global LwIP state provided that: 1) other threads hold
 *  the LwIP core lock while mutating the list of netifs; and 2) netif object
 *  themselves are never destroyed.
 *
 *  On sockets-based systems, iteration is always stable in the face of changes
 *  to the underlying system's interfaces and/or addresses.
 *
 *  On LwIP systems, iteration is stable except in the case where the interface
 *  associated with the current address is removed, in which case iteration may
 *  end prematurely.
 */
class DLL_EXPORT InterfaceAddressIterator
{
public:
    InterfaceAddressIterator();
    ~InterfaceAddressIterator();

    bool HasCurrent();
    bool Next();
    IPAddress GetAddress();
    uint8_t GetPrefixLength();
    uint8_t GetIPv6PrefixLength();
    void GetAddressWithPrefix(IPPrefix & addrWithPrefix);
    InterfaceId GetInterface();
    InterfaceId GetInterfaceId();
    CHIP_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize);
    bool IsUp();
    bool SupportsMulticast();
    bool HasBroadcastAddress();

private:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    enum
    {
        kBeforeStartIndex = -1
    };

    InterfaceIterator mIntfIter;
    int mCurAddrIndex;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    struct ifaddrs * mAddrsList;
    struct ifaddrs * mCurAddr;
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    InterfaceIterator mIntfIter;
    net_if_ipv6 * mIpv6 = nullptr;
    int mCurAddrIndex   = -1;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
};

#if CHIP_SYSTEM_CONFIG_USE_LWIP

inline InterfaceIterator::InterfaceIterator(void)
{
    mCurNetif = netif_list;
}

inline InterfaceIterator::~InterfaceIterator(void) {}

inline bool InterfaceIterator::HasCurrent(void)
{
    return mCurNetif != NULL;
}

inline InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return mCurNetif;
}

inline InterfaceAddressIterator::InterfaceAddressIterator(void)
{
    mCurAddrIndex = kBeforeStartIndex;
}

inline InterfaceAddressIterator::~InterfaceAddressIterator(void) {}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
inline InterfaceIterator::~InterfaceIterator()               = default;
inline InterfaceAddressIterator::~InterfaceAddressIterator() = default;
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 * @brief    Deprecated alias for \c GetInterfaceId(void)
 */
inline InterfaceId InterfaceIterator::GetInterface()
{
    return GetInterfaceId();
}

/**
 * @brief    Deprecated alias for \c GetInterfaceId(void)
 */
inline InterfaceId InterfaceAddressIterator::GetInterface()
{
    return GetInterfaceId();
}

/**
 * @brief    Deprecated alias for \c GetPrefixLength(void)
 */
inline uint8_t InterfaceAddressIterator::GetIPv6PrefixLength()
{
    return GetPrefixLength();
}

} // namespace Inet
} // namespace chip
