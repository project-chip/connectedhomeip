/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines classes for abstracting access to and
 *      interactions with a platform- and system-specific Internet
 *      Protocol stack which, as of this implementation, may be either
 *      BSD/POSIX Sockets or LwIP.
 *
 *      Major abstractions provided are:
 *
 *        * Timers
 *        * Domain Name System (DNS) resolution
 *        * TCP network transport
 *        * UDP network transport
 *        * Raw network transport
 *
 *      For BSD/POSIX Sockets (CHIP_SYSTEM_CONFIG_USE_SOCKETS), event readiness
 *      notification is handled via file descriptors, using a System::Layer API.
 *
 *      For LwIP (CHIP_SYSTEM_CONFIG_USE_LWIP), event readiness notification is handled
 *      via events / messages and platform- and system-specific hooks for the event
 *      / message system.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include "InetLayer.h"

#include "InetFaultInjection.h"

#include <platform/LockTracker.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utility>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/netif.h>
#include <lwip/sys.h>
#else // !CHIP_SYSTEM_CONFIG_USE_LWIP
#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>
#ifdef __ANDROID__
#include <ifaddrs-android.h>
#elif CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <ifaddrs.h>
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <net/net_if.h>
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

namespace chip {
namespace Inet {

/**
 *  This is the InetLayer default constructor.
 *
 *  It performs some basic data member initialization; however, since
 *  InetLayer follows an explicit initializer design pattern, the InetLayer::Init
 *  method must be called successfully prior to using the object.
 *
 */
InetLayer::InetLayer() {}

#if INET_CONFIG_MAX_DROPPABLE_EVENTS && CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_NO_LOCKING

CHIP_ERROR InetLayer::InitQueueLimiter(void)
{
    mDroppableEvents = 0;
    return CHIP_NO_ERROR;
}

bool InetLayer::CanEnqueueDroppableEvent(void)
{
    if (__sync_add_and_fetch(&mDroppableEvents, 1) <= INET_CONFIG_MAX_DROPPABLE_EVENTS)
    {
        return true;
    }
    else
    {
        __sync_add_and_fetch(&mDroppableEvents, -1);
        return false;
    }
}

void InetLayer::DroppableEventDequeued(void)
{
    __sync_add_and_fetch(&mDroppableEvents, -1);
}

#elif CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

CHIP_ERROR InetLayer::InitQueueLimiter(void)
{
    const unsigned portBASE_TYPE maximum = INET_CONFIG_MAX_DROPPABLE_EVENTS;
    const unsigned portBASE_TYPE initial = INET_CONFIG_MAX_DROPPABLE_EVENTS;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    mDroppableEvents                     = xSemaphoreCreateCountingStatic(maximum, initial, &mDroppableEventsObj);
#else
    mDroppableEvents = xSemaphoreCreateCounting(maximum, initial);
#endif

    if (mDroppableEvents != NULL)
        return CHIP_NO_ERROR;
    else
        return CHIP_ERROR_NO_MEMORY;
}

bool InetLayer::CanEnqueueDroppableEvent(void)
{
    if (xSemaphoreTake(mDroppableEvents, 0) != pdTRUE)
    {
        return false;
    }
    return true;
}

void InetLayer::DroppableEventDequeued(void)
{
    xSemaphoreGive(mDroppableEvents);
}

#else // !CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

CHIP_ERROR InetLayer::InitQueueLimiter(void)
{
    if (sem_init(&mDroppableEvents, 0, INET_CONFIG_MAX_DROPPABLE_EVENTS) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
}

bool InetLayer::CanEnqueueDroppableEvent(void)
{
    // Striclty speaking, we should check for EAGAIN.  But, other
    // errno values probably should signal that that we should drop
    // the packet: EINVAL means that the semaphore is not valid (we
    // failed initialization), and EINTR should probably also lead to
    // dropping a packet.

    if (sem_trywait(&mDroppableEvents) != 0)
    {
        return false;
    }
    return true;
}

void InetLayer::DroppableEventDequeued(void)
{
    sem_post(&mDroppableEvents);
}

#endif // !CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#endif // INET_CONFIG_MAX_DROPPABLE_EVENTS && CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  This is the InetLayer explicit initializer. This must be called
 *  and complete successfully before the InetLayer may be used.
 *
 *  The caller may provide an optional context argument which will be
 *  passed back via any platform-specific hook functions. For
 *  LwIP-based adaptations, this will typically be a pointer to the
 *  event queue associated with the InetLayer instance.
 *
 *  @param[in]  aSystemLayer  A required instance of the chip System Layer
 *                            already successfully initialized.
 *
 *  @param[in]  aContext  An optional context argument which will be passed
 *                        back to the caller via any platform-specific hook
 *                        functions.
 *
 *  @retval   #CHIP_ERROR_INCORRECT_STATE        If the InetLayer is in an
 *                                               incorrect state.
 *  @retval   #CHIP_ERROR_NO_MEMORY              If the InetLayer runs out
 *                                               of resource for this
 *                                               request for a new timer.
 *  @retval   other Platform-specific errors indicating the reason for
 *            initialization failure.
 *  @retval   #CHIP_NO_ERROR                     On success.
 *
 */
CHIP_ERROR InetLayer::Init(chip::System::Layer & aSystemLayer, void * aContext)
{
    Inet::RegisterLayerErrorFormatter();
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    // Platform-specific initialization may elect to set this data
    // member. Ensure it is set to a sane default value before
    // invoking platform-specific initialization.

    mPlatformData = nullptr;

    mSystemLayer = &aSystemLayer;
    mContext     = aContext;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    ReturnErrorOnFailure(InitQueueLimiter());
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    mLayerState.SetInitialized();

    return CHIP_NO_ERROR;
}

/**
 *  This is the InetLayer explicit deinitializer and should be called
 *  prior to disposing of an instantiated InetLayer instance.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
 *          the reason for shutdown failure.
 *
 */
CHIP_ERROR InetLayer::Shutdown()
{
    VerifyOrReturnError(mLayerState.SetShuttingDown(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Abort all TCP endpoints owned by this instance.
    TCPEndPoint::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && &lEndPoint->Layer() == this)
        {
            lEndPoint->Abort();
        }
        return true;
    });
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    // Close all UDP endpoints owned by this instance.
    UDPEndPoint::sPool.ForEachActiveObject([&](UDPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && &lEndPoint->Layer() == this)
        {
            lEndPoint->Close();
        }
        return true;
    });
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

    mLayerState.SetShutdown();
    mLayerState.Reset(); // Return to uninitialized state to permit re-initialization.
    return err;
}

/**
 * This returns any client-specific platform data assigned to the
 * instance, if it has been previously set.
 *
 * @return Client-specific platform data, if is has been previously set;
 *         otherwise, NULL.
 *
 */
void * InetLayer::GetPlatformData()
{
    return mPlatformData;
}

/**
 * This sets the specified client-specific platform data to the
 * instance for later retrieval by the client platform.
 *
 * @param[in]  aPlatformData  The client-specific platform data to set.
 *
 */
void InetLayer::SetPlatformData(void * aPlatformData)
{
    mPlatformData = aPlatformData;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
bool InetLayer::IsIdleTimerRunning()
{
    bool timerRunning = false;

    // See if there are any TCP connections with the idle timer check in use.
    TCPEndPoint::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && (lEndPoint->mIdleTimeout != 0))
        {
            timerRunning = true;
            return false;
        }
        return true;
    });

    return timerRunning;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

/**
 *  Get the link local IPv6 address for a specified link or interface.
 *
 *  @param[in]    interface The interface for which the link local IPv6
 *                          address is being sought.
 *
 *  @param[out]   llAddr  The link local IPv6 address for the link.
 *
 *  @retval    #CHIP_ERROR_NOT_IMPLEMENTED      If IPv6 is not supported.
 *  @retval    #CHIP_ERROR_INVALID_ARGUMENT     If the link local address
 *                                              is nullptr.
 *  @retval    #INET_ERROR_ADDRESS_NOT_FOUND    If the link does not have
 *                                              any address configured.
 *  @retval    #CHIP_NO_ERROR                   On success.
 *
 */
CHIP_ERROR InetLayer::GetLinkLocalAddr(InterfaceId interface, IPAddress * llAddr)
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !LWIP_IPV6
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif //! LWIP_IPV6

    struct netif * link = interface.GetPlatformInterface();
    for (struct netif * intf = netif_list; intf != NULL; intf = intf->next)
    {
        if ((link != NULL) && (link != intf))
            continue;
        for (int j = 0; j < LWIP_IPV6_NUM_ADDRESSES; ++j)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(intf, j)) && ip6_addr_islinklocal(netif_ip6_addr(intf, j)))
            {
                (*llAddr) = IPAddress(*netif_ip6_addr(intf, j));
                return CHIP_NO_ERROR;
            }
        }
        if (link != NULL)
        {
            return INET_ERROR_ADDRESS_NOT_FOUND;
        }
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
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
                (!interface.IsPresent() || (if_nametoindex(ifaddr_iter->ifa_name) == interface.GetPlatformInterface())))
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
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    net_if * const iface = interface.IsPresent() ? net_if_get_by_index(interface.GetPlatformInterface()) : net_if_get_default();
    VerifyOrReturnError(iface != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    in6_addr * const ip6_addr = net_if_ipv6_get_ll(iface, NET_ADDR_PREFERRED);
    VerifyOrReturnError(ip6_addr != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    *llAddr = IPAddress(*ip6_addr);
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

    return CHIP_NO_ERROR;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
/**
 *  Creates a new TCPEndPoint object.
 *
 *  @note
 *    This function gets a free TCPEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[in,out]  retEndPoint    A pointer to a pointer of the TCPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #CHIP_ERROR_INCORRECT_STATE    If the InetLayer object is not initialized.
 *  @retval  #CHIP_ERROR_ENDPOINT_POOL_FULL If the InetLayer TCPEndPoint pool is full and no new
 *                                          endpoints can be created.
 *  @retval  #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR InetLayer::NewTCPEndPoint(TCPEndPoint ** retEndPoint)
{
    assertChipStackLockedByCurrentThread();

    *retEndPoint = nullptr;

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = TCPEndPoint::sPool.CreateObject(*this);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "TCP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumTCPEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
/**
 *  Creates a new UDPEndPoint object.
 *
 *  @note
 *    This function gets a free UDPEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[in,out]  retEndPoint    A pointer to a pointer of the UDPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #CHIP_ERROR_INCORRECT_STATE    If the InetLayer object is not initialized.
 *  @retval  #CHIP_ERROR_ENDPOINT_POOL_FULL If the InetLayer UDPEndPoint pool is full and no new
 *                                          endpoints can be created.
 *  @retval  #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR InetLayer::NewUDPEndPoint(UDPEndPoint ** retEndPoint)
{
    assertChipStackLockedByCurrentThread();

    *retEndPoint = nullptr;

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = UDPEndPoint::sPool.CreateObject(*this);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "UDP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumUDPEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

/**
 *  Get the interface identifier for the specified IP address. If the
 *  interface identifier cannot be derived it is set to the default
 *  InterfaceId.
 *
 *  @note
 *    This function fetches the first interface (from the configured list
 *    of interfaces) that matches the specified IP address.
 *
 *  @param[in]    addr      A reference to the IPAddress object.
 *
 *  @param[out]   intfId    A reference to the InterfaceId object.
 *
 *  @return  #CHIP_NO_ERROR unconditionally.
 *
 */
CHIP_ERROR InetLayer::GetInterfaceFromAddr(const IPAddress & addr, InterfaceId & intfId)
{
    InterfaceAddressIterator addrIter;

    for (; addrIter.HasCurrent(); addrIter.Next())
    {
        IPAddress curAddr = addrIter.GetAddress();
        if (addr == curAddr)
        {
            intfId = addrIter.GetInterfaceId();
            return CHIP_NO_ERROR;
        }
    }

    intfId = InterfaceId::Null();

    return CHIP_NO_ERROR;
}

/**
 *  Check if there is a prefix match between the specified IPv6 address and any of
 *  the locally configured IPv6 addresses.
 *
 *  @param[in]    addr    The IPv6 address to check for the prefix-match.
 *
 *  @return true if a successful match is found, otherwise false.
 *
 */
bool InetLayer::MatchLocalIPv6Subnet(const IPAddress & addr)
{
    if (addr.IsIPv6LinkLocal())
        return true;

    InterfaceAddressIterator ifAddrIter;
    for (; ifAddrIter.HasCurrent(); ifAddrIter.Next())
    {
        IPPrefix addrPrefix;
        addrPrefix.IPAddr = ifAddrIter.GetAddress();
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

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
void InetLayer::HandleTCPInactivityTimer(chip::System::Layer * aSystemLayer, void * aAppState)
{
    InetLayer & lInetLayer = *reinterpret_cast<InetLayer *>(aAppState);
    bool lTimerRequired    = lInetLayer.IsIdleTimerRunning();

    TCPEndPoint::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if (&lEndPoint->Layer() != &lInetLayer)
            return true;
        if (!lEndPoint->IsConnected())
            return true;
        if (lEndPoint->mIdleTimeout == 0)
            return true;

        if (lEndPoint->mRemainingIdleTime == 0)
        {
            lEndPoint->DoClose(INET_ERROR_IDLE_TIMEOUT, false);
        }
        else
        {
            --lEndPoint->mRemainingIdleTime;
        }

        return true;
    });

    if (lTimerRequired)
    {
        aSystemLayer->StartTimer(System::Clock::Milliseconds32(INET_TCP_IDLE_CHECK_INTERVAL), HandleTCPInactivityTimer,
                                 &lInetLayer);
    }
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

/**
 *  Reset the members of the IPPacketInfo object.
 *
 */
void IPPacketInfo::Clear()
{
    SrcAddress  = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface   = InterfaceId::Null();
    SrcPort     = 0;
    DestPort    = 0;
}

} // namespace Inet
} // namespace chip
