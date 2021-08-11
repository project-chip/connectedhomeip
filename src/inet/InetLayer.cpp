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
 *      notification is handled via file descriptors, using System::WatchableSocket.
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

#include <system/SystemTimer.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

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

void InetLayer::UpdateSnapshot(chip::System::Stats::Snapshot & aSnapshot)
{
#if INET_CONFIG_ENABLE_DNS_RESOLVER
    DNSResolver::sPool.GetStatistics(aSnapshot.mResourcesInUse[chip::System::Stats::kInetLayer_NumDNSResolvers],
                                     aSnapshot.mHighWatermarks[chip::System::Stats::kInetLayer_NumDNSResolvers]);
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[chip::System::Stats::kInetLayer_NumTCPEps],
                                     aSnapshot.mHighWatermarks[chip::System::Stats::kInetLayer_NumTCPEps]);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    UDPEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[chip::System::Stats::kInetLayer_NumUDPEps],
                                     aSnapshot.mHighWatermarks[chip::System::Stats::kInetLayer_NumUDPEps]);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    RawEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[chip::System::Stats::kInetLayer_NumRawEps],
                                     aSnapshot.mHighWatermarks[chip::System::Stats::kInetLayer_NumRawEps]);
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
}

/**
 *  This is the InetLayer default constructor.
 *
 *  It performs some basic data member initialization; however, since
 *  InetLayer follows an explicit initializer design pattern, the InetLayer::Init
 *  method must be called successfully prior to using the object.
 *
 */
InetLayer::InetLayer()
{
    State = kState_NotInitialized;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (!sInetEventHandlerDelegate.IsInitialized())
        sInetEventHandlerDelegate.Init(HandleInetLayerEvent);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
chip::System::LwIPEventHandlerDelegate InetLayer::sInetEventHandlerDelegate;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

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
        return chip::System::MapErrorPOSIX(errno);
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
 *  Platforms may choose to assert
 *  #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS in their
 *  platform-specific configuration header and enable the
 *  Platform::InetLayer::WillInit and Platform::InetLayer::DidInit
 *  hooks to effect platform-specific customizations or data extensions
 *  to InetLayer.
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    Inet::RegisterLayerErrorFormatter();

    if (State != kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    // Platform-specific initialization may elect to set this data
    // member. Ensure it is set to a sane default value before
    // invoking platform-specific initialization.

    mPlatformData = nullptr;

    err = Platform::InetLayer::WillInit(this, aContext);
    SuccessOrExit(err);

    mSystemLayer = &aSystemLayer;
    mContext     = aContext;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    err = InitQueueLimiter();
    SuccessOrExit(err);

    mSystemLayer->AddEventHandlerDelegate(sInetEventHandlerDelegate);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    State = kState_Initialized;

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    err = mAsyncDNSResolver.Init(this);
    SuccessOrExit(err);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

exit:
    Platform::InetLayer::DidInit(this, mContext, err);
    return err;
}

/**
 *  This is the InetLayer explicit deinitializer and should be called
 *  prior to disposing of an instantiated InetLayer instance.
 *
 *  Platforms may choose to assert
 *  #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS in their
 *  platform-specific configuration header and enable the
 *  Platform::InetLayer::WillShutdown and
 *  Platform::InetLayer::DidShutdown hooks to effect clean-up of
 *  platform-specific customizations or data extensions to InetLayer.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
 *          the reason for shutdown failure.
 *
 */
CHIP_ERROR InetLayer::Shutdown()
{
    CHIP_ERROR err;

    err = Platform::InetLayer::WillShutdown(this, mContext);
    SuccessOrExit(err);

    if (State == kState_Initialized)
    {
#if INET_CONFIG_ENABLE_DNS_RESOLVER
        // Cancel all DNS resolution requests owned by this instance.
        for (size_t i = 0; i < DNSResolver::sPool.Size(); i++)
        {
            DNSResolver * lResolver = DNSResolver::sPool.Get(*mSystemLayer, i);
            if ((lResolver != nullptr) && lResolver->IsCreatedByInetLayer(*this))
            {
                lResolver->Cancel();
            }
        }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

        err = mAsyncDNSResolver.Shutdown();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        // Close all raw endpoints owned by this Inet layer instance.
        for (size_t i = 0; i < RawEndPoint::sPool.Size(); i++)
        {
            RawEndPoint * lEndPoint = RawEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != nullptr) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Close();
            }
        }
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        // Abort all TCP endpoints owned by this instance.
        for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
        {
            TCPEndPoint * lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != nullptr) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Abort();
            }
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        // Close all UDP endpoints owned by this instance.
        for (size_t i = 0; i < UDPEndPoint::sPool.Size(); i++)
        {
            UDPEndPoint * lEndPoint = UDPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != nullptr) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Close();
            }
        }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
    }

    State = kState_NotInitialized;

exit:
    Platform::InetLayer::DidShutdown(this, mContext, err);

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

    // see if there are any TCP connections with the idle timer check in use.
    for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
    {
        TCPEndPoint * lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);

        if ((lEndPoint != nullptr) && (lEndPoint->mIdleTimeout != 0))
        {
            timerRunning = true;
            break;
        }
    }

    return timerRunning;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

/**
 *  Get the link local IPv6 address for a specified link or interface.
 *
 *  @param[in]    link    The interface for which the link local IPv6
 *                        address is being sought.
 *
 *  @param[out]   llAddr  The link local IPv6 address for the link.
 *
 *  @retval    #CHIP_ERROR_NOT_IMPLEMENTED      If IPv6 is not supported.
 *  @retval    #CHIP_ERROR_INVALID_ARGUMENT     If the link local address
 *                                              is NULL.
 *  @retval    #INET_ERROR_ADDRESS_NOT_FOUND    If the link does not have
 *                                              any address configured.
 *  @retval    #CHIP_NO_ERROR                   On success.
 *
 */
CHIP_ERROR InetLayer::GetLinkLocalAddr(InterfaceId link, IPAddress * llAddr)
{
    VerifyOrReturnError(llAddr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !LWIP_IPV6
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif //! LWIP_IPV6

    for (struct netif * intf = netif_list; intf != NULL; intf = intf->next)
    {
        if ((link != NULL) && (link != intf))
            continue;
        for (int j = 0; j < LWIP_IPV6_NUM_ADDRESSES; ++j)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(intf, j)) && ip6_addr_islinklocal(netif_ip6_addr(intf, j)))
            {
                (*llAddr) = IPAddress::FromIPv6(*netif_ip6_addr(intf, j));
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
                ((link == INET_NULL_INTERFACEID) || (if_nametoindex(ifaddr_iter->ifa_name) == link)))
            {
                struct in6_addr * sin6_addr = &(reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr;
                if (sin6_addr->s6_addr[0] == 0xfe && (sin6_addr->s6_addr[1] & 0xc0) == 0x80) // Link Local Address
                {
                    (*llAddr) = IPAddress::FromIPv6((reinterpret_cast<struct sockaddr_in6 *>(ifaddr_iter->ifa_addr))->sin6_addr);
                    break;
                }
            }
        }
    }
    freeifaddrs(ifaddr);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    net_if * const iface = (link == INET_NULL_INTERFACEID) ? net_if_get_default() : net_if_get_by_index(link);
    VerifyOrReturnError(iface != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    in6_addr * const ip6_addr = net_if_ipv6_get_ll(iface, NET_ADDR_PREFERRED);
    VerifyOrReturnError(ip6_addr != nullptr, INET_ERROR_ADDRESS_NOT_FOUND);

    *llAddr = IPAddress::FromIPv6(*ip6_addr);
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

    return CHIP_NO_ERROR;
}

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
/**
 *  Creates a new RawEndPoint object for a specific IP version and protocol.
 *
 *  @note
 *    This function gets a free RawEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[in]      ipVer          IPv4 or IPv6.
 *
 *  @param[in]      ipProto        A protocol within the IP family (e.g., ICMPv4 or ICMPv6).
 *
 *  @param[in,out]  retEndPoint    A pointer to a pointer of the RawEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #CHIP_ERROR_INCORRECT_STATE    If the InetLayer object is not initialized.
 *  @retval  #CHIP_ERROR_ENDPOINT_POOL_FULL If the InetLayer RawEndPoint pool is full and no new
 *                                          endpoints can be created.
 *  @retval  #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR InetLayer::NewRawEndPoint(IPVersion ipVer, IPProtocol ipProto, RawEndPoint ** retEndPoint)
{
    assertChipStackLockedByCurrentThread();

    *retEndPoint = nullptr;

    VerifyOrReturnError(State == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = RawEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "Raw");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    (*retEndPoint)->Inet::RawEndPoint::Init(this, ipVer, ipProto);
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumRawEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

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

    VerifyOrReturnError(State == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = TCPEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "TCP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    (*retEndPoint)->Init(this);
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

    VerifyOrReturnError(State == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = UDPEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "UDP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    (*retEndPoint)->Init(this);
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumUDPEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_DNS_RESOLVER
/**
 *  Perform an IP address resolution of a specified hostname.
 *
 *  @note
 *    This is an asynchronous operation and the result will be communicated back
 *    via the OnComplete() callback.
 *
 *  @param[in]  hostName    A pointer to a NULL-terminated C string representing
 *                          the host name to be queried.
 *
 *  @param[in]  maxAddrs    The maximum number of addresses to store in the DNS
 *                          table.
 *
 *  @param[in]  addrArray   A pointer to the DNS table.
 *
 *  @param[in]  onComplete  A pointer to the callback function when a DNS
 *                          request is complete.
 *
 *  @param[in]  appState    A pointer to the application state to be passed to
 *                          onComplete when a DNS request is complete.
 *
 *  @retval #CHIP_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #CHIP_ERROR_NO_MEMORY            if the Inet layer resolver pool
 *                                           is full.
 *  @retval #INET_ERROR_HOST_NAME_TOO_LONG   if a requested host name is too
 *                                           long.
 *  @retval #INET_ERROR_HOST_NOT_FOUND       if a request host name could not be
 *                                           resolved to an address.
 *  @retval #INET_ERROR_DNS_TRY_AGAIN        if a name server returned a
 *                                           temporary failure indication;
 *                                           try again later.
 *  @retval #INET_ERROR_DNS_NO_RECOVERY      if a name server returned an
 *                                           unrecoverable error.
 *  @retval #CHIP_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
CHIP_ERROR InetLayer::ResolveHostAddress(const char * hostName, uint8_t maxAddrs, IPAddress * addrArray,
                                         DNSResolveCompleteFunct onComplete, void * appState)
{
    size_t hostNameLength = strlen(hostName);
    if (hostNameLength > UINT16_MAX)
    {
        return INET_ERROR_HOST_NAME_TOO_LONG;
    }
    return ResolveHostAddress(hostName, static_cast<uint16_t>(hostNameLength), maxAddrs, addrArray, onComplete, appState);
}

/**
 *  Perform an IP address resolution of a specified hostname.
 *
 *  @param[in]  hostName    A pointer to a non NULL-terminated C string representing the host name
 *                          to be queried.
 *
 *  @param[in]  hostNameLen The string length of host name.
 *
 *  @param[in]  maxAddrs    The maximum number of addresses to store in the DNS
 *                          table.
 *
 *  @param[in]  addrArray   A pointer to the DNS table.
 *
 *  @param[in]  onComplete  A pointer to the callback function when a DNS
 *                          request is complete.
 *
 *  @param[in]  appState    A pointer to the application state to be passed to
 *                          onComplete when a DNS request is complete.
 *
 *  @retval #CHIP_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #CHIP_ERROR_NO_MEMORY            if the Inet layer resolver pool
 *                                           is full.
 *  @retval #INET_ERROR_HOST_NAME_TOO_LONG   if a requested host name is too
 *                                           long.
 *  @retval #INET_ERROR_HOST_NOT_FOUND       if a request host name could not be
 *                                           resolved to an address.
 *  @retval #INET_ERROR_DNS_TRY_AGAIN        if a name server returned a
 *                                           temporary failure indication;
 *                                           try again later.
 *  @retval #INET_ERROR_DNS_NO_RECOVERY      if a name server returned an
 *                                           unrecoverable error.
 *  @retval #CHIP_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
CHIP_ERROR InetLayer::ResolveHostAddress(const char * hostName, uint16_t hostNameLen, uint8_t maxAddrs, IPAddress * addrArray,
                                         DNSResolveCompleteFunct onComplete, void * appState)
{
    return ResolveHostAddress(hostName, hostNameLen, kDNSOption_Default, maxAddrs, addrArray, onComplete, appState);
}

/**
 *  Perform an IP address resolution of a specified hostname.
 *
 *  @param[in]  hostName    A pointer to a non NULL-terminated C string representing the host name
 *                          to be queried.
 *
 *  @param[in]  hostNameLen The string length of host name.
 *
 *  @param[in]  options     An integer value controlling how host name resolution is performed.
 *
 *                          Value should be one of the address family values from the
 *                          #DNSOptions enumeration:
 *
 *                          #kDNSOption_AddrFamily_Any
 *                          #kDNSOption_AddrFamily_IPv4Only
 *                          #kDNSOption_AddrFamily_IPv6Only
 *                          #kDNSOption_AddrFamily_IPv4Preferred
 *                          #kDNSOption_AddrFamily_IPv6Preferred
 *
 *  @param[in]  maxAddrs    The maximum number of addresses to store in the DNS
 *                          table.
 *
 *  @param[in]  addrArray   A pointer to the DNS table.
 *
 *  @param[in]  onComplete  A pointer to the callback function when a DNS
 *                          request is complete.
 *
 *  @param[in]  appState    A pointer to the application state to be passed to
 *                          onComplete when a DNS request is complete.
 *
 *  @retval #CHIP_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #CHIP_ERROR_NO_MEMORY            if the Inet layer resolver pool
 *                                           is full.
 *  @retval #INET_ERROR_HOST_NAME_TOO_LONG   if a requested host name is too
 *                                           long.
 *  @retval #INET_ERROR_HOST_NOT_FOUND       if a request host name could not be
 *                                           resolved to an address.
 *  @retval #INET_ERROR_DNS_TRY_AGAIN        if a name server returned a
 *                                           temporary failure indication;
 *                                           try again later.
 *  @retval #INET_ERROR_DNS_NO_RECOVERY      if a name server returned an
 *                                           unrecoverable error.
 *  @retval #CHIP_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
CHIP_ERROR InetLayer::ResolveHostAddress(const char * hostName, uint16_t hostNameLen, uint8_t options, uint8_t maxAddrs,
                                         IPAddress * addrArray, DNSResolveCompleteFunct onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR err         = CHIP_NO_ERROR;
    DNSResolver * resolver = nullptr;

    VerifyOrExit(State == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    INET_FAULT_INJECT(FaultInjection::kFault_DNSResolverNew, return CHIP_ERROR_NO_MEMORY);

    // Store context information and set the resolver state.
    VerifyOrExit(hostNameLen <= NL_DNS_HOSTNAME_MAX_LEN, err = INET_ERROR_HOST_NAME_TOO_LONG);
    VerifyOrExit(maxAddrs > 0, err = CHIP_ERROR_NO_MEMORY);

    resolver = DNSResolver::sPool.TryCreate(*mSystemLayer);
    if (resolver != nullptr)
    {
        resolver->InitInetLayerBasis(*this);
    }
    else
    {
        ChipLogError(Inet, "%s resolver pool FULL", "DNS");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }

    // Short-circuit full address resolution if the supplied host name is a text-form
    // IP address...
    if (IPAddress::FromString(hostName, hostNameLen, *addrArray))
    {
        uint8_t addrTypeOption = (options & kDNSOption_AddrFamily_Mask);
        IPAddressType addrType = addrArray->Type();

        if ((addrTypeOption == kDNSOption_AddrFamily_IPv6Only && addrType != kIPAddressType_IPv6)
#if INET_CONFIG_ENABLE_IPV4
            || (addrTypeOption == kDNSOption_AddrFamily_IPv4Only && addrType != kIPAddressType_IPv4)
#endif
        )
        {
            err = INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE;
        }

        if (onComplete)
        {
            onComplete(appState, err, (err == CHIP_NO_ERROR) ? 1 : 0, addrArray);
        }

        resolver->Release();
        resolver = nullptr;

        ExitNow(err = CHIP_NO_ERROR);
    }

    // After this point, the resolver will be released by:
    // - mAsyncDNSResolver (in case of ASYNC_DNS_SOCKETS)
    // - resolver->Resolve() (in case of synchronous resolving)
    // - the event handlers (in case of LwIP)

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

    err =
        mAsyncDNSResolver.PrepareDNSResolver(*resolver, hostName, hostNameLen, options, maxAddrs, addrArray, onComplete, appState);
    SuccessOrExit(err);

    mAsyncDNSResolver.EnqueueRequest(*resolver);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

#if !INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    err = resolver->Resolve(hostName, hostNameLen, options, maxAddrs, addrArray, onComplete, appState);
#endif // !INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
exit:

    return err;
}

/**
 *  Cancel any outstanding DNS query (for a matching completion callback and
 *  application state) that may still be active.
 *
 *  @note
 *    This situation can arise if the application initiates a connection
 *    to a peer using a hostname and then aborts/closes the connection
 *    before the hostname resolution completes.
 *
 *  @param[in]    onComplete   A pointer to the callback function when a DNS
 *                             request is complete.
 *
 *  @param[in]    appState     A pointer to an application state object to be passed
 *                             to the callback function as argument.
 *
 */
void InetLayer::CancelResolveHostAddress(DNSResolveCompleteFunct onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    if (State != kState_Initialized)
        return;

    for (size_t i = 0; i < DNSResolver::sPool.Size(); i++)
    {
        DNSResolver * lResolver = DNSResolver::sPool.Get(*mSystemLayer, i);

        if (lResolver == nullptr)
        {
            continue;
        }

        if (!lResolver->IsCreatedByInetLayer(*this))
        {
            continue;
        }

        if (lResolver->OnComplete != onComplete)
        {
            continue;
        }

        if (lResolver->AppState != appState)
        {
            continue;
        }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
        if (lResolver->mState == DNSResolver::kState_Canceled)
        {
            continue;
        }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

        lResolver->Cancel();
        break;
    }
}

#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

/**
 *  Get the interface identifier for the specified IP address. If the
 *  interface identifier cannot be derived it is set to the
 *  #INET_NULL_INTERFACEID.
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
            intfId = addrIter.GetInterface();
            return CHIP_NO_ERROR;
        }
    }

    intfId = INET_NULL_INTERFACEID;

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
        addrPrefix.Length = ifAddrIter.GetIPv6PrefixLength();
        if (addrPrefix.MatchAddress(addr))
            return true;
    }

    return false;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
void InetLayer::HandleTCPInactivityTimer(chip::System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    InetLayer & lInetLayer = *reinterpret_cast<InetLayer *>(aAppState);
    bool lTimerRequired    = lInetLayer.IsIdleTimerRunning();

    for (size_t i = 0; i < INET_CONFIG_NUM_TCP_ENDPOINTS; i++)
    {
        TCPEndPoint * lEndPoint = TCPEndPoint::sPool.Get(*aSystemLayer, i);

        if (lEndPoint == nullptr)
            continue;
        if (!lEndPoint->IsCreatedByInetLayer(lInetLayer))
            continue;
        if (!lEndPoint->IsConnected())
            continue;
        if (lEndPoint->mIdleTimeout == 0)
            continue;

        if (lEndPoint->mRemainingIdleTime == 0)
        {
            lEndPoint->DoClose(INET_ERROR_IDLE_TIMEOUT, false);
        }
        else
        {
            --lEndPoint->mRemainingIdleTime;
        }
    }

    if (lTimerRequired)
    {
        aSystemLayer->StartTimer(INET_TCP_IDLE_CHECK_INTERVAL, HandleTCPInactivityTimer, &lInetLayer);
    }
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

#if CHIP_SYSTEM_CONFIG_USE_LWIP
CHIP_ERROR InetLayer::HandleInetLayerEvent(chip::System::Object & aTarget, chip::System::EventType aEventType, uintptr_t aArgument)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(INET_IsInetEvent(aEventType), CHIP_ERROR_UNEXPECTED_EVENT);

    // Dispatch the event according to its type.
    switch (aEventType)
    {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    case kInetEvent_TCPConnectComplete:
        static_cast<TCPEndPoint &>(aTarget).HandleConnectComplete(static_cast<CHIP_ERROR>(aArgument));
        break;

    case kInetEvent_TCPConnectionReceived:
        static_cast<TCPEndPoint &>(aTarget).HandleIncomingConnection(reinterpret_cast<TCPEndPoint *>(aArgument));
        break;

    case kInetEvent_TCPDataReceived:
        static_cast<TCPEndPoint &>(aTarget).HandleDataReceived(
            System::PacketBufferHandle::Adopt(reinterpret_cast<chip::System::PacketBuffer *>(aArgument)));
        break;

    case kInetEvent_TCPDataSent:
        static_cast<TCPEndPoint &>(aTarget).HandleDataSent(static_cast<uint16_t>(aArgument));
        break;

    case kInetEvent_TCPError:
        static_cast<TCPEndPoint &>(aTarget).HandleError(static_cast<CHIP_ERROR>(aArgument));
        break;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    case kInetEvent_RawDataReceived:
        static_cast<RawEndPoint &>(aTarget).HandleDataReceived(
            System::PacketBufferHandle::Adopt(reinterpret_cast<chip::System::PacketBuffer *>(aArgument)));
        break;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    case kInetEvent_UDPDataReceived:
        static_cast<UDPEndPoint &>(aTarget).HandleDataReceived(
            System::PacketBufferHandle::Adopt(reinterpret_cast<chip::System::PacketBuffer *>(aArgument)));
        break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_DNS_RESOLVER
    case kInetEvent_DNSResolveComplete:
        static_cast<DNSResolver &>(aTarget).HandleResolveComplete();
        break;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    default:
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }

    // If the event was droppable, record the fact that it has been dequeued.
    if (IsDroppableEvent(aEventType))
    {
        InetLayerBasis & lBasis = static_cast<InetLayerBasis &>(aTarget);
        InetLayer & lInetLayer  = lBasis.Layer();

        lInetLayer.DroppableEventDequeued();
    }

    return CHIP_NO_ERROR;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  Reset the members of the IPPacketInfo object.
 *
 */
void IPPacketInfo::Clear()
{
    SrcAddress  = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface   = INET_NULL_INTERFACEID;
    SrcPort     = 0;
    DestPort    = 0;
}

#if !INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS

// MARK: InetLayer platform- and system-specific functions for InetLayer
//       construction and destruction.

namespace Platform {
namespace InetLayer {

/**
 * This is a platform-specific InetLayer pre-initialization hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[in,out] aLayer    A pointer to the InetLayer instance being
 *                          initialized.
 *
 * @param[in,out] aContext  Platform-specific context data passed to
 *                          the layer initialization method, \::Init.
 *
 * @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
 *         the reason for initialization failure. Returning non-successful
 *         status will abort initialization.
 *
 */
DLL_EXPORT CHIP_ERROR WillInit(Inet::InetLayer * aLayer, void * aContext)
{
    (void) aLayer;
    (void) aContext;

    return CHIP_NO_ERROR;
}

/**
 * This is a platform-specific InetLayer post-initialization hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[in,out] aLayer    A pointer to the InetLayer instance being
 *                          initialized.
 *
 * @param[in,out] aContext  Platform-specific context data passed to
 *                          the layer initialization method, \::Init.
 *
 * @param[in]     anError   The overall status being returned via the
 *                          InetLayer \::Init method.
 *
 */
DLL_EXPORT void DidInit(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError)
{
    (void) aLayer;
    (void) aContext;
    (void) anError;
}

/**
 * This is a platform-specific InetLayer pre-shutdown hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[in,out] aLayer    A pointer to the InetLayer instance being
 *                          shutdown.
 *
 * @param[in,out] aContext  Platform-specific context data passed to
 *                          the layer initialization method, \::Init.
 *
 * @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
 *         the reason for shutdown failure. Returning non-successful
 *         status will abort shutdown.
 *
 */
DLL_EXPORT CHIP_ERROR WillShutdown(Inet::InetLayer * aLayer, void * aContext)
{
    (void) aLayer;
    (void) aContext;

    return CHIP_NO_ERROR;
}

/**
 * This is a platform-specific InetLayer post-shutdown hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[in,out] aLayer    A pointer to the InetLayer instance being
 *                          shutdown.
 *
 * @param[in,out] aContext  Platform-specific context data passed to
 *                          the layer initialization method, \::Init.
 *
 * @param[in]     anError   The overall status being returned via the
 *                          InetLayer \::Shutdown method.
 *
 */
DLL_EXPORT void DidShutdown(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError)
{
    (void) aLayer;
    (void) aContext;
    (void) anError;
}

} // namespace InetLayer
} // namespace Platform

#endif // !INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS

} // namespace Inet
} // namespace chip
