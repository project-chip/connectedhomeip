/*
 *
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
 *      For BSD/POSIX Sockets, event readiness notification is handled
 *      via file descriptors and a traditional poll / select
 *      implementation on the platform adaptation.
 *
 *      For LwIP, event readiness notification is handle via events /
 *      messages and platform- and system-specific hooks for the event
 *      / message system.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <InetLayer/InetLayer.h>
#include <InetLayer/InetFaultInjection.h>

#include <SystemLayer/SystemTimer.h>

#include <Weave/Support/CodeUtils.h>
#include <Weave/Support/logging/WeaveLogging.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <lwip/sys.h>
#include <lwip/netif.h>
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#ifdef __ANDROID__
#include <ifaddrs-android.h>
#else
#include <ifaddrs.h>
#endif // __ANDROID__
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#if WEAVE_SYSTEM_CONFIG_USE_LWIP && !INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS

// MARK: InetLayer platform- and system-specific functions for LwIP-native eventing.

struct LwIPInetEvent
{
    nl::Inet::InetEventType     Type;
    nl::Inet::InetLayerBasis*   Target;
    uintptr_t                   Arg;
};

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP && !INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

namespace nl {
namespace Inet {

void InetLayer::UpdateSnapshot(nl::Weave::System::Stats::Snapshot &aSnapshot)
{
#if INET_CONFIG_ENABLE_DNS_RESOLVER
    DNSResolver::sPool.GetStatistics(aSnapshot.mResourcesInUse[nl::Weave::System::Stats::kInetLayer_NumDNSResolvers],
                                     aSnapshot.mHighWatermarks[nl::Weave::System::Stats::kInetLayer_NumDNSResolvers]);
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[nl::Weave::System::Stats::kInetLayer_NumTCPEps],
                                     aSnapshot.mHighWatermarks[nl::Weave::System::Stats::kInetLayer_NumTCPEps]);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    UDPEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[nl::Weave::System::Stats::kInetLayer_NumUDPEps],
                                     aSnapshot.mHighWatermarks[nl::Weave::System::Stats::kInetLayer_NumUDPEps]);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    RawEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[nl::Weave::System::Stats::kInetLayer_NumRawEps],
                                     aSnapshot.mHighWatermarks[nl::Weave::System::Stats::kInetLayer_NumRawEps]);
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
#if INET_CONFIG_ENABLE_TUN_ENDPOINT
    TunEndPoint::sPool.GetStatistics(aSnapshot.mResourcesInUse[nl::Weave::System::Stats::kInetLayer_NumTunEps],
                                     aSnapshot.mHighWatermarks[nl::Weave::System::Stats::kInetLayer_NumTunEps]);
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT
}

/**
 *  This is the InetLayer default constructor.
 *
 *  It performs some basic data member initialization; however, since
 *  InetLayer follows an explicit initializer design pattern, the InetLayer::Init
 *  method must be called successfully prior to using the object.
 *
 */
InetLayer::InetLayer(void)
#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    : mImplicitSystemLayer()
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
{
    State = kState_NotInitialized;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    if (!sInetEventHandlerDelegate.IsInitialized())
        sInetEventHandlerDelegate.Init(HandleInetLayerEvent);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
Weave::System::LwIPEventHandlerDelegate InetLayer::sInetEventHandlerDelegate;
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if INET_CONFIG_MAX_DROPPABLE_EVENTS && WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_NO_LOCKING

INET_ERROR InetLayer::InitQueueLimiter(void)
{
    mDroppableEvents = 0;
    return INET_NO_ERROR;
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

#elif WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

INET_ERROR InetLayer::InitQueueLimiter(void)
{
    const unsigned portBASE_TYPE maximum = INET_CONFIG_MAX_DROPPABLE_EVENTS;
    const unsigned portBASE_TYPE initial = INET_CONFIG_MAX_DROPPABLE_EVENTS;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    mDroppableEvents = xSemaphoreCreateCountingStatic(maximum, initial, &mDroppableEventsObj);
#else
    mDroppableEvents = xSemaphoreCreateCounting(maximum, initial);
#endif

    if (mDroppableEvents != NULL)
        return INET_NO_ERROR;
    else
        return INET_ERROR_NO_MEMORY;
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

#else // !WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

INET_ERROR InetLayer::InitQueueLimiter(void)
{
    if (sem_init(&mDroppableEvents, 0, INET_CONFIG_MAX_DROPPABLE_EVENTS) != 0)
    {
        return Weave::System::MapErrorPOSIX(errno);
    }
    return INET_NO_ERROR;
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

#endif // !WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
#endif // INET_CONFIG_MAX_DROPPABLE_EVENTS && WEAVE_SYSTEM_CONFIG_USE_LWIP

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
 *  @param[in]  aSystemLayer  A required instance of the Weave System Layer
 *                            already successfully initialized.
 *
 *  @param[in]  aContext  An optional context argument which will be passed
 *                        back to the caller via any platform-specific hook
 *                        functions.
 *
 *  @retval   #INET_ERROR_INCORRECT_STATE        If the InetLayer is in an
 *                                               incorrect state.
 *  @retval   #INET_ERROR_NO_MEMORY              If the InetLayer runs out
 *                                               of resource for this
 *                                               request for a new timer.
 *  @retval   other Platform-specific errors indicating the reason for
 *            initialization failure.
 *  @retval   #INET_NO_ERROR                     On success.
 *
 */
INET_ERROR InetLayer::Init(Weave::System::Layer& aSystemLayer, void *aContext)
{
    INET_ERROR err = INET_NO_ERROR;

    RegisterInetLayerErrorFormatter();

    if (State != kState_NotInitialized)
        return INET_ERROR_INCORRECT_STATE;

    // Platform-specific initialization may elect to set this data
    // member. Ensure it is set to a sane default value before
    // invoking platform-specific initialization.

    mPlatformData = NULL;

    Platform::InetLayer::WillInit(this, aContext);
    SuccessOrExit(err);

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    if (&aSystemLayer == &mImplicitSystemLayer)
    {
        err = mImplicitSystemLayer.Init(aContext);
        SuccessOrExit(err);
    }
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    mSystemLayer = &aSystemLayer;
    mContext = aContext;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    err = InitQueueLimiter();
    SuccessOrExit(err);

    mSystemLayer->AddEventHandlerDelegate(sInetEventHandlerDelegate);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

    State = kState_Initialized;

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

    err = mAsyncDNSResolver.Init(this);
    SuccessOrExit(err);

#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

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
 *  @return #INET_NO_ERROR on success; otherwise, a specific error indicating
 *          the reason for shutdown failure.
 *
 */
INET_ERROR InetLayer::Shutdown(void)
{
    INET_ERROR err;

    err = Platform::InetLayer::WillShutdown(this, mContext);
    SuccessOrExit(err);

    if (State == kState_Initialized)
    {
#if INET_CONFIG_ENABLE_DNS_RESOLVER
        // Cancel all DNS resolution requests owned by this instance.
        for (size_t i = 0; i < DNSResolver::sPool.Size(); i++)
        {
            DNSResolver* lResolver = DNSResolver::sPool.Get(*mSystemLayer, i);
            if ((lResolver != NULL) && lResolver->IsCreatedByInetLayer(*this))
            {
                lResolver->Cancel();
            }
        }

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

        err = mAsyncDNSResolver.Shutdown();

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        // Close all raw endpoints owned by this Inet layer instance.
        for (size_t i = 0; i < RawEndPoint::sPool.Size(); i++)
        {
            RawEndPoint* lEndPoint = RawEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Close();
            }
        }
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        // Abort all TCP endpoints owned by this instance.
        for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
        {
            TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Abort();
            }
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        // Close all UDP endpoints owned by this instance.
        for (size_t i = 0; i < UDPEndPoint::sPool.Size(); i++)
        {
            UDPEndPoint* lEndPoint = UDPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->Close();
            }
        }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
        if (mSystemLayer == &mImplicitSystemLayer)
        {
            err = mImplicitSystemLayer.Shutdown();
            SuccessOrExit(err);
        }
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

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
void *InetLayer::GetPlatformData(void)
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
void InetLayer::SetPlatformData(void *aPlatformData)
{
    mPlatformData = aPlatformData;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
bool InetLayer::IsIdleTimerRunning(void)
{
    bool timerRunning = false;

    // see if there are any TCP connections with the idle timer check in use.
    for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
    {
        TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);

        if ((lEndPoint != NULL) && (lEndPoint->mIdleTimeout != 0))
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
 *  @retval    #INET_ERROR_NOT_IMPLEMENTED      If IPv6 is not supported.
 *  @retval    #INET_ERROR_BAD_ARGS             If the link local address
 *                                              is NULL.
 *  @retval    #INET_ERROR_ADDRESS_NOT_FOUND    If the link does not have
 *                                              any address configured.
 *  @retval    #INET_NO_ERROR                   On success.
 *
 */
INET_ERROR InetLayer::GetLinkLocalAddr(InterfaceId link, IPAddress *llAddr)
{
    INET_ERROR err = INET_NO_ERROR;
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if !LWIP_IPV6
    err = INET_ERROR_NOT_IMPLEMENTED;
    goto out;
#endif //!LWIP_IPV6
#endif //WEAVE_SYSTEM_CONFIG_USE_LWIP

    if ( llAddr == NULL )
    {
        err = INET_ERROR_BAD_ARGS;
        goto out;
    }

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    for (struct netif *intf = netif_list; intf != NULL; intf = intf->next)
    {
        if ( (link != NULL) && (link != intf) )    continue;
        int j; for (j = 0; j < LWIP_IPV6_NUM_ADDRESSES; ++j)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(intf, j)) && ip6_addr_islinklocal(netif_ip6_addr(intf, j)))
            {
                (*llAddr) = IPAddress::FromIPv6(*netif_ip6_addr(intf, j));
                goto out;
            }
        }
        if (link != NULL) {
            err = INET_ERROR_ADDRESS_NOT_FOUND;
            break;
        }
    }
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    struct ifaddrs *ifaddr;
    int rv;
    rv = getifaddrs(&ifaddr);
    if (rv != -1)
    {
        struct ifaddrs*ifaddr_iter = ifaddr;
        while (ifaddr_iter != NULL)
        {

            if (ifaddr_iter->ifa_addr != NULL)
            {
                if ((ifaddr_iter->ifa_addr->sa_family == AF_INET6) &&
                    ((link == INET_NULL_INTERFACEID) || (if_nametoindex(ifaddr_iter->ifa_name) == link)))
                {
                    struct in6_addr *sin6_addr = &((struct sockaddr_in6*)ifaddr_iter->ifa_addr)->sin6_addr;
                    if (sin6_addr->s6_addr[0] == 0xfe && (sin6_addr->s6_addr[1] & 0xc0) == 0x80)//Link Local Address
                    {
                        (*llAddr) = IPAddress::FromIPv6(((struct sockaddr_in6*)ifaddr_iter->ifa_addr)->sin6_addr);
                        break;
                    }
                }
            }
            ifaddr_iter = ifaddr_iter->ifa_next;
        }
        freeifaddrs(ifaddr);
    }
    else
    {
        err = INET_ERROR_ADDRESS_NOT_FOUND;
    }
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

out:
    return err;
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
 *  @param[inout]   retEndPoint    A pointer to a pointer of the RawEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #INET_ERROR_INCORRECT_STATE  If the InetLayer object is not initialized.
 *  @retval  #INET_ERROR_NO_ENDPOINTS     If the InetLayer RawEndPoint pool is full and no new
 *                                        endpoints can be created.
 *  @retval  #INET_NO_ERROR               On success.
 *
 */
INET_ERROR InetLayer::NewRawEndPoint(IPVersion ipVer, IPProtocol ipProto, RawEndPoint **retEndPoint)
{
    INET_ERROR err = INET_NO_ERROR;
    *retEndPoint = NULL;

    VerifyOrExit(State == kState_Initialized, err = INET_ERROR_INCORRECT_STATE);

    *retEndPoint = RawEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint != NULL)
    {
        (*retEndPoint)->nl::Inet::RawEndPoint::Init(this, ipVer, ipProto);
        SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kInetLayer_NumRawEps);
    }
    else
    {
        WeaveLogError(Inet, "%s endpoint pool FULL", "Raw");
        err = INET_ERROR_NO_ENDPOINTS;
    }

exit:
    return err;
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
 *  @param[inout]   retEndPoint    A pointer to a pointer of the TCPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #INET_ERROR_INCORRECT_STATE  If the InetLayer object is not initialized.
 *  @retval  #INET_ERROR_NO_ENDPOINTS     If the InetLayer TCPEndPoint pool is full and no new
 *                                        endpoints can be created.
 *  @retval  #INET_NO_ERROR               On success.
 *
 */
INET_ERROR InetLayer::NewTCPEndPoint(TCPEndPoint **retEndPoint)
{
    INET_ERROR err = INET_NO_ERROR;
    *retEndPoint = NULL;

    VerifyOrExit(State == kState_Initialized, err = INET_ERROR_INCORRECT_STATE);

    *retEndPoint = TCPEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint != NULL)
    {
        (*retEndPoint)->Init(this);
        SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kInetLayer_NumTCPEps);
    }
    else
    {
        WeaveLogError(Inet, "%s endpoint pool FULL", "TCP");
        err = INET_ERROR_NO_ENDPOINTS;
    }

exit:
    return err;
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
 *  @param[inout]   retEndPoint    A pointer to a pointer of the UDPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #INET_ERROR_INCORRECT_STATE  If the InetLayer object is not initialized.
 *  @retval  #INET_ERROR_NO_ENDPOINTS     If the InetLayer UDPEndPoint pool is full and no new
 *                                        endpoints can be created.
 *  @retval  #INET_NO_ERROR               On success.
 *
 */
INET_ERROR InetLayer::NewUDPEndPoint(UDPEndPoint **retEndPoint)
{
    INET_ERROR err = INET_NO_ERROR;
    *retEndPoint = NULL;

    VerifyOrExit(State == kState_Initialized, err = INET_ERROR_INCORRECT_STATE);

    *retEndPoint = UDPEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint != NULL)
    {
        (*retEndPoint)->Init(this);
        SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kInetLayer_NumUDPEps);
    }
    else
    {
        WeaveLogError(Inet, "%s endpoint pool FULL", "UDP");
        err = INET_ERROR_NO_ENDPOINTS;
    }

exit:
    return err;
}
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
/**
 *  Creates a new TunEndPoint object.
 *
 *  @note
 *    This function gets a free TunEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[inout]   retEndPoint    A pointer to a pointer of the TunEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #INET_ERROR_INCORRECT_STATE  If the InetLayer object is not initialized.
 *  @retval  #INET_ERROR_NO_ENDPOINTS     If the InetLayer TunEndPoint pool is full and no new
 *                                        ones can be created.
 *  @retval  #INET_NO_ERROR               On success.
 *
 */
INET_ERROR InetLayer::NewTunEndPoint(TunEndPoint **retEndPoint)
{
    INET_ERROR err = INET_NO_ERROR;
    *retEndPoint = NULL;

    VerifyOrExit(State == kState_Initialized, err = INET_ERROR_INCORRECT_STATE);

    *retEndPoint = TunEndPoint::sPool.TryCreate(*mSystemLayer);
    if (*retEndPoint != NULL)
    {
        (*retEndPoint)->Init(this);
        SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kInetLayer_NumTunEps);
    }
    else
    {
        WeaveLogError(Inet, "%s endpoint pool FULL", "Tun");
        err = INET_ERROR_NO_ENDPOINTS;
    }

exit:
    return err;
}
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

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
 *  @retval #INET_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #INET_ERROR_NO_MEMORY            if the Inet layer resolver pool
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
 *  @retval #INET_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
INET_ERROR InetLayer::ResolveHostAddress(const char *hostName, uint8_t maxAddrs,
                                         IPAddress *addrArray,
                                         DNSResolveCompleteFunct onComplete, void *appState)
{
    return ResolveHostAddress(hostName, strlen(hostName), maxAddrs, addrArray, onComplete, appState);
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
 *  @retval #INET_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #INET_ERROR_NO_MEMORY            if the Inet layer resolver pool
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
 *  @retval #INET_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
INET_ERROR InetLayer::ResolveHostAddress(const char *hostName, uint16_t hostNameLen,
                                         uint8_t maxAddrs, IPAddress *addrArray,
                                         DNSResolveCompleteFunct onComplete, void *appState)
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
 *  @retval #INET_NO_ERROR                   if a DNS request is handled
 *                                           successfully.
 *  @retval #INET_ERROR_NO_MEMORY            if the Inet layer resolver pool
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
 *  @retval #INET_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                           the underlying platform.
 *  @retval other POSIX network or OS error returned by the underlying DNS
 *          resolver implementation.
 *
 */
INET_ERROR InetLayer::ResolveHostAddress(const char *hostName, uint16_t hostNameLen,
                                         uint8_t options,
                                         uint8_t maxAddrs, IPAddress *addrArray,
                                         DNSResolveCompleteFunct onComplete, void *appState)
{
    INET_ERROR err = INET_NO_ERROR;
    DNSResolver *resolver = NULL;

    VerifyOrExit(State == kState_Initialized, err = INET_ERROR_INCORRECT_STATE);

    INET_FAULT_INJECT(FaultInjection::kFault_DNSResolverNew, return INET_ERROR_NO_MEMORY);

    // Store context information and set the resolver state.
    VerifyOrExit(hostNameLen <= NL_DNS_HOSTNAME_MAX_LEN, err = INET_ERROR_HOST_NAME_TOO_LONG);
    VerifyOrExit(maxAddrs > 0, err = INET_ERROR_NO_MEMORY);

    resolver = DNSResolver::sPool.TryCreate(*mSystemLayer);
    if (resolver != NULL)
    {
        resolver->InitInetLayerBasis(*this);
    }
    else
    {
        WeaveLogError(Inet, "%s resolver pool FULL", "DNS");
        ExitNow(err = INET_ERROR_NO_MEMORY);
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
            onComplete(appState, err, (err == INET_NO_ERROR) ? 1 : 0, addrArray);
        }

        resolver->Release();
        resolver = NULL;

        ExitNow(err = INET_NO_ERROR);
    }

    // After this point, the resolver will be released by:
    // - mAsyncDNSResolver (in case of ASYNC_DNS_SOCKETS)
    // - resolver->Resolve() (in case of synchronous resolving)
    // - the event handlers (in case of LwIP)

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

    err = mAsyncDNSResolver.PrepareDNSResolver(*resolver, hostName, hostNameLen, options,
                                               maxAddrs, addrArray, onComplete, appState);
    SuccessOrExit(err);

    mAsyncDNSResolver.EnqueueRequest(*resolver);

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

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
void InetLayer::CancelResolveHostAddress(DNSResolveCompleteFunct onComplete, void *appState)
{
    if (State != kState_Initialized)
        return;

    for (size_t i = 0; i < DNSResolver::sPool.Size(); i++)
    {
        DNSResolver* lResolver = DNSResolver::sPool.Get(*mSystemLayer, i);

        if (lResolver == NULL)
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

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
        if (lResolver->mState == DNSResolver::kState_Canceled)
        {
            continue;
        }
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

        lResolver->Cancel();
        break;
    }
}

#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
static void SystemTimerComplete(Weave::System::Layer* aLayer, void* aAppState, Weave::System::Error aError)
{
    Weave::System::Timer& lTimer = *reinterpret_cast<Weave::System::Timer*>(aAppState);
    InetLayer& lInetLayer = *lTimer.InetLayer();
    InetLayer::TimerCompleteFunct lComplete = reinterpret_cast<InetLayer::TimerCompleteFunct>(lTimer.OnCompleteInetLayer());
    void* lAppState = lTimer.AppStateInetLayer();

    lComplete(&lInetLayer, lAppState, static_cast<INET_ERROR>(aError));
}

/**
*  @brief
*    This method registers a one-shot timer(to fire at a specified offset relative to the time
*    of registration) with the underlying timer mechanism, through this InetLayer instance.
*
*  @note
*    Each InetLayer instance could have its own set of timers. This might not be
*    significant to applications, as each application usually works with one singleton
*    InetLayer instance.
*
*  @param[in]    aMilliseconds  Number of milliseconds before this timer should fire.
*
*  @param[in]    aComplete      A pointer to a callback function to be called when this
*                               timer fires.
*
*  @param[in]    aAppState      A pointer to an application state object to be passed
*                               to the callback function as argument.
*
*  @retval #INET_ERROR_INCORRECT_STATE  If the InetLayer instance is not initialized.
*  @retval #INET_ERROR_NO_MEMORY        If the InetLayer runs out of resource for this
*                                           request for a new timer.
*  @retval #INET_NO_ERROR               On success.
*
*/
INET_ERROR InetLayer::StartTimer(uint32_t aMilliseconds, TimerCompleteFunct aComplete, void* aAppState)
{
    INET_ERROR lReturn;

    if (State != kState_Initialized)
    {
        return INET_ERROR_INCORRECT_STATE;
    }

    Weave::System::Timer* lTimer;

    lReturn = mSystemLayer->NewTimer(lTimer);
    SuccessOrExit(lReturn);

    lTimer->AttachInetLayer(*this, reinterpret_cast<void*>(aComplete), aAppState);

    lReturn = lTimer->Start(aMilliseconds, SystemTimerComplete, lTimer);

    if (lReturn != WEAVE_SYSTEM_NO_ERROR)
    {
        lTimer->Cancel();
    }

exit:
    switch (lReturn)
    {
    case WEAVE_SYSTEM_ERROR_NO_MEMORY: lReturn = INET_ERROR_NO_MEMORY; break;
    case WEAVE_SYSTEM_NO_ERROR: lReturn = INET_NO_ERROR; break;
    }

    return lReturn;
}

/**
*  @brief
*    This method cancels an one-shot timer, started earlier through @p StartTimer().
*
*    @note
*      The cancellation could fail silently in two different ways. If the timer
*      specified by the combination of the callback function and application state object
*      couldn't be found, cancellation could fail. If the timer has fired, but not yet
*      removed from memory, cancellation could also fail.
*
*  @param[in]   aComplete   A pointer to the callback function used in calling @p StartTimer().
*  @param[in]   aAppState   A pointer to the application state object used in calling
*                            @p StartTimer().
*
*/
void InetLayer::CancelTimer(TimerCompleteFunct aComplete, void* aAppState)
{
    if (State == kState_Initialized)
    {
        mSystemLayer->CancelAllMatchingInetTimers(*this, reinterpret_cast<void*>(aComplete), aAppState);
    }
}
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

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
 *  @return  #INET_NO_ERROR unconditionally.
 *
 */
INET_ERROR InetLayer::GetInterfaceFromAddr(const IPAddress& addr, InterfaceId& intfId)
{
    InterfaceAddressIterator addrIter;

    for (; addrIter.HasCurrent(); addrIter.Next())
    {
        IPAddress curAddr = addrIter.GetAddress();
        if (addr == curAddr)
        {
            intfId = addrIter.GetInterface();
            return INET_NO_ERROR;
        }
    }

    intfId = INET_NULL_INTERFACEID;

    return INET_NO_ERROR;
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
bool InetLayer::MatchLocalIPv6Subnet(const IPAddress& addr)
{
    if (addr.IsIPv6LinkLocal())
        return true;

    InterfaceAddressIterator ifAddrIter;
    for ( ; ifAddrIter.HasCurrent(); ifAddrIter.Next())
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
void InetLayer::HandleTCPInactivityTimer(Weave::System::Layer* aSystemLayer, void* aAppState, Weave::System::Error aError)
{
    InetLayer& lInetLayer = *reinterpret_cast<InetLayer*>(aAppState);
    bool lTimerRequired = lInetLayer.IsIdleTimerRunning();

    for (size_t i = 0; i < INET_CONFIG_NUM_TCP_ENDPOINTS; i++)
    {
        TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*aSystemLayer, i);

        if (lEndPoint == NULL) continue;
        if (!lEndPoint->IsCreatedByInetLayer(lInetLayer)) continue;
        if (!lEndPoint->IsConnected()) continue;
        if (lEndPoint->mIdleTimeout == 0) continue;

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

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
Weave::System::Error InetLayer::HandleInetLayerEvent(Weave::System::Object& aTarget, Weave::System::EventType aEventType,
    uintptr_t aArgument)
{
    Weave::System::Error lReturn = WEAVE_SYSTEM_NO_ERROR;
    InetLayerBasis& lBasis = static_cast<InetLayerBasis&>(aTarget);

    VerifyOrExit(INET_IsInetEvent(aEventType), lReturn = WEAVE_SYSTEM_ERROR_UNEXPECTED_EVENT);

    // Dispatch the event according to its type.
    switch (aEventType)
    {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    case kInetEvent_TCPConnectComplete:
        static_cast<TCPEndPoint &>(aTarget).HandleConnectComplete(static_cast<INET_ERROR>(aArgument));
        break;

    case kInetEvent_TCPConnectionReceived:
        static_cast<TCPEndPoint &>(aTarget).HandleIncomingConnection(reinterpret_cast<TCPEndPoint*>(aArgument));
        break;

    case kInetEvent_TCPDataReceived:
        static_cast<TCPEndPoint &>(aTarget).HandleDataReceived(reinterpret_cast<Weave::System::PacketBuffer*>(aArgument));
        break;

    case kInetEvent_TCPDataSent:
        static_cast<TCPEndPoint &>(aTarget).HandleDataSent(static_cast<uint16_t>(aArgument));
        break;

    case kInetEvent_TCPError:
        static_cast<TCPEndPoint &>(aTarget).HandleError(static_cast<INET_ERROR>(aArgument));
        break;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    case kInetEvent_RawDataReceived:
        static_cast<RawEndPoint &>(aTarget).HandleDataReceived(reinterpret_cast<Weave::System::PacketBuffer*>(aArgument));
        break;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    case kInetEvent_UDPDataReceived:
        static_cast<UDPEndPoint &>(aTarget).HandleDataReceived(reinterpret_cast<Weave::System::PacketBuffer*>(aArgument));
        break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
    case kInetEvent_TunDataReceived:
        static_cast<TunEndPoint &>(aTarget).HandleDataReceived(reinterpret_cast<Weave::System::PacketBuffer*>(aArgument));
        break;
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

#if INET_CONFIG_ENABLE_DNS_RESOLVER
    case kInetEvent_DNSResolveComplete:
        static_cast<DNSResolver &>(aTarget).HandleResolveComplete();
        break;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    default:
        lReturn = WEAVE_SYSTEM_ERROR_UNEXPECTED_EVENT;
        ExitNow();
    }

    // If the event was droppable, record the fact that it has been dequeued.
    if (IsDroppableEvent(aEventType))
    {
        InetLayer& lInetLayer = lBasis.Layer();

        lInetLayer.DroppableEventDequeued();
    }

exit:
    return lReturn;
}

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  This posts an event / message of the specified type with the
 *  provided argument to this instance's platform-specific event /
 *  message queue.
 *
 *  @param[inout]  target  A pointer to the InetLayer object making the
 *                         post request.
 *
 *  @param[in]     type    The type of event to post.
 *
 *  @param[inout]  arg     The argument associated with the event to post.
 *
 *  @retval    #INET_ERROR_INCORRECT_STATE      If the state of the InetLayer
 *                                              object is incorrect.
 *  @retval    #INET_ERROR_BAD_ARGS             If #target is NULL.
 *  @retval    #INET_ERROR_NO_MEMORY            If the EventQueue is already
 *                                              full.
 *  @retval    other platform-specific errors generated indicating the reason
 *             for failure.
 *  @retval    #INET_NO_ERROR                   On success.
 *
 */
INET_ERROR InetLayer::PostEvent(InetLayerBasis *target, InetEventType type, uintptr_t arg)
{
    Weave::System::Layer& lSystemLayer = *mSystemLayer;
    INET_ERROR retval = INET_NO_ERROR;

    VerifyOrExit(State == kState_Initialized, retval = INET_ERROR_INCORRECT_STATE);
    VerifyOrExit(target != NULL, retval = INET_ERROR_BAD_ARGS);

    {
        Weave::System::Layer& lTargetSystemLayer = target->SystemLayer();

        VerifyOrDieWithMsg(target->IsRetained(lSystemLayer), Inet, "wrong system layer! [target %p != instance %p]",
            &lTargetSystemLayer, &lSystemLayer);
    }

    // Sanity check that this instance and the target layer haven't
    // been "crossed".

    {
        InetLayer& lTargetInetLayer = target->Layer();

        VerifyOrDieWithMsg(this == &lTargetInetLayer, Inet, "target layer %p != instance layer %p", &lTargetInetLayer, this);
    }

    if (IsDroppableEvent(type) && !CanEnqueueDroppableEvent())
    {
        WeaveLogProgress(Inet, "Dropping incoming packet (type %d)", (int)type);
        ExitNow(retval = INET_ERROR_NO_MEMORY);
    }

    retval = Platform::InetLayer::PostEvent(this, mContext, target, type, arg);
    if (retval != INET_NO_ERROR)
    {
        WeaveLogError(Inet, "Failed to queue InetLayer event (type %d): %s", (int)type, ErrorStr(retval));
    }
    SuccessOrExit(retval);

exit:
    return retval;
}

/**
 *  This is a syntactic wrapper around a platform-specific hook that
 *  effects an event loop, waiting on a queue that services this
 *  instance, pulling events off of that queue, and then dispatching
 *  them for handling.
 *
 *  @return #INET_NO_ERROR on success; otherwise, a specific error
 *          indicating the reason for initialization failure.
 *
 */
INET_ERROR InetLayer::DispatchEvents(void)
{
    INET_ERROR retval = INET_NO_ERROR;

    VerifyOrExit(State == kState_Initialized, retval = INET_ERROR_INCORRECT_STATE);

    retval = Platform::InetLayer::DispatchEvents(this, mContext);
    SuccessOrExit(retval);

 exit:
    return retval;
}

/**
 *  Start the platform timer with specified msec duration.
 *
 *  @brief
 *    Calls the Platform specific API to start a platform timer.
 *    This API is called by the Weave::System::Timer class when one or more
 *    system timers are active and require deferred execution.
 *
 *  @param[in]  inDurMS  The timer duration in milliseconds.
 *
 *  @return INET_NO_ERROR on success, error code otherwise.
 *
 */
INET_ERROR InetLayer::StartPlatformTimer(uint32_t inDurMS)
{
    INET_ERROR retval;

    VerifyOrExit(State == kState_Initialized, retval = INET_ERROR_INCORRECT_STATE);

    retval = Platform::InetLayer::StartTimer(this, mContext, inDurMS);

 exit:
    return retval;
}

/**
 *  Handle the platform timer expiration event.
 *
 *  @brief
 *    Calls Weave::System::Timer::HandleExpiredTimers to handle any expired
 *    system timers.  It is assumed that this API is called only while
 *    on the thread which owns the InetLayer object.
 *
 *  @return INET_NO_ERROR on success, error code otherwise.
 *
 */
INET_ERROR InetLayer::HandlePlatformTimer(void)
{
    INET_ERROR lReturn;
    Weave::System::Error lSystemError;

    VerifyOrExit(State == kState_Initialized, lReturn = INET_ERROR_INCORRECT_STATE);

    lSystemError = mSystemLayer->HandlePlatformTimer();
    lReturn = static_cast<INET_ERROR>(lSystemError);

 exit:
    return lReturn;
}

#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
/**
 *  Prepare the sets of file descriptors for @p select() to work with.
 *
 *  @param[out]    nfds       The range of file descriptors in the file
 *                            descriptor set.
 *
 *  @param[in]     readfds    A pointer to the set of readable file descriptors.
 *
 *  @param[in]     writefds   A pointer to the set of writable file descriptors.
 *
 *  @param[in]     exceptfds  A pointer to the set of file descriptors with errors.
 *
 * @param[in]      sleepTimeTV A pointer to a structure specifying how long the select should sleep
 *
 */
void InetLayer::PrepareSelect(int& nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
        struct timeval& sleepTimeTV)
{
    if (State != kState_Initialized)
        return;

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    for (size_t i = 0; i < RawEndPoint::sPool.Size(); i++)
    {
        RawEndPoint* lEndPoint = RawEndPoint::sPool.Get(*mSystemLayer, i);
        if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            lEndPoint->PrepareIO().SetFDs(lEndPoint->mSocket, nfds, readfds, writefds, exceptfds);
    }
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
    {
        TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);
        if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            lEndPoint->PrepareIO().SetFDs(lEndPoint->mSocket, nfds, readfds, writefds, exceptfds);
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    for (size_t i = 0; i < UDPEndPoint::sPool.Size(); i++)
    {
        UDPEndPoint* lEndPoint = UDPEndPoint::sPool.Get(*mSystemLayer, i);
        if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            lEndPoint->PrepareIO().SetFDs(lEndPoint->mSocket, nfds, readfds, writefds, exceptfds);
    }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
    for (size_t i = 0; i < TunEndPoint::sPool.Size(); i++)
    {
        TunEndPoint* lEndPoint = TunEndPoint::sPool.Get(*mSystemLayer, i);
        if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            lEndPoint->PrepareIO().SetFDs(lEndPoint->mSocket, nfds, readfds, writefds, exceptfds);
    }
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    if (mSystemLayer == &mImplicitSystemLayer)
    {
        mSystemLayer->PrepareSelect(nfds, readfds, writefds, exceptfds, sleepTimeTV);
    }
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
}

/**
 *  Handle I/O from a select call. This method registers the pending I/O
 *  event in each active endpoint and then invokes the respective I/O
 *  handling functions for those endpoints.
 *
 *  @note
 *    It is important to set the pending I/O fields for all endpoints
 *    *before* making any callbacks. This avoids the case where an
 *    endpoint is closed and then re-opened within the callback for
 *    another endpoint. When this happens the new endpoint is likely
 *    to be assigned the same file descriptor as the old endpoint.
 *    However, any pending I/O for that file descriptor number represents
 *    I/O related to the old incarnation of the endpoint, not the current
 *    one. Saving the pending I/O state in each endpoint before acting
 *    on it allows the endpoint code to clear the I/O flags in the event
 *    of a close, thus avoiding any confusion.
 *
 *  @param[in]    selectRes    The return value of the select call.
 *
 *  @param[in]    readfds      A pointer to the set of read file descriptors.
 *
 *  @param[in]    writefds     A pointer to the set of write file descriptors.
 *
 *  @param[in]    exceptfds    A pointer to the set of file descriptors with
 *                             errors.
 *
 */
void InetLayer::HandleSelectResult(int selectRes, fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
    if (State != kState_Initialized)
        return;

    if (selectRes < 0)
        return;

    if (selectRes > 0)
    {
        // Set the pending I/O field for each active endpoint based on the value returned by select.
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        for (size_t i = 0; i < RawEndPoint::sPool.Size(); i++)
        {
            RawEndPoint* lEndPoint = RawEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->mPendingIO = SocketEvents::FromFDs(lEndPoint->mSocket, readfds, writefds, exceptfds);
            }
        }
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
        {
            TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->mPendingIO = SocketEvents::FromFDs(lEndPoint->mSocket, readfds, writefds, exceptfds);
            }
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        for (size_t i = 0; i < UDPEndPoint::sPool.Size(); i++)
        {
            UDPEndPoint* lEndPoint = UDPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->mPendingIO = SocketEvents::FromFDs(lEndPoint->mSocket, readfds, writefds, exceptfds);
            }
        }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
        for (size_t i = 0; i < TunEndPoint::sPool.Size(); i++)
        {
            TunEndPoint* lEndPoint = TunEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->mPendingIO = SocketEvents::FromFDs(lEndPoint->mSocket, readfds, writefds, exceptfds);
            }
        }
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

        // Now call each active endpoint to handle its pending I/O.
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        for (size_t i = 0; i < RawEndPoint::sPool.Size(); i++)
        {
            RawEndPoint* lEndPoint = RawEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->HandlePendingIO();
            }
        }
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        for (size_t i = 0; i < TCPEndPoint::sPool.Size(); i++)
        {
            TCPEndPoint* lEndPoint = TCPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->HandlePendingIO();
            }
        }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        for (size_t i = 0; i < UDPEndPoint::sPool.Size(); i++)
        {
            UDPEndPoint* lEndPoint = UDPEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->HandlePendingIO();
            }
        }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
        for (size_t i = 0; i < TunEndPoint::sPool.Size(); i++)
        {
            TunEndPoint* lEndPoint = TunEndPoint::sPool.Get(*mSystemLayer, i);
            if ((lEndPoint != NULL) && lEndPoint->IsCreatedByInetLayer(*this))
            {
                lEndPoint->HandlePendingIO();
            }
        }
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT
    }

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    if (mSystemLayer == &mImplicitSystemLayer)
    {
        mSystemLayer->HandleSelectResult(selectRes, readfds, writefds, exceptfds);
    }
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
}

#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 *  Reset the members of the IPPacketInfo object.
 *
 */
void IPPacketInfo::Clear()
{
    SrcAddress = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface = INET_NULL_INTERFACEID;
    SrcPort = 0;
    DestPort = 0;
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
 * @param[inout]  aLayer    A pointer to the InetLayer instance being
 *                          initialized.
 *
 * @param[inout]  aContext  Platform-specific context data passed to
 *                          the layer initialization method, ::Init.
 *
 * @return #INET_NO_ERROR on success; otherwise, a specific error indicating
 *         the reason for initialization failure. Returning non-successful
 *         status will abort initialization.
 *
 */
NL_DLL_EXPORT INET_ERROR WillInit(Inet::InetLayer *aLayer, void *aContext)
{
    (void)aLayer;
    (void)aContext;

    return INET_NO_ERROR;
}

/**
 * This is a platform-specific InetLayer post-initialization hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[inout]  aLayer    A pointer to the InetLayer instance being
 *                          initialized.
 *
 * @param[inout]  aContext  Platform-specific context data passed to
 *                          the layer initialization method, ::Init.
 *
 * @param[in]     anError   The overall status being returned via the
 *                          InetLayer ::Init method.
 *
 */
NL_DLL_EXPORT void DidInit(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError)
{
    (void)aLayer;
    (void)aContext;
    (void)anError;

    return;
}

/**
 * This is a platform-specific InetLayer pre-shutdown hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[inout]  aLayer    A pointer to the InetLayer instance being
 *                          shutdown.
 *
 * @param[inout]  aContext  Platform-specific context data passed to
 *                          the layer initialization method, ::Init.
 *
 * @return #INET_NO_ERROR on success; otherwise, a specific error indicating
 *         the reason for shutdown failure. Returning non-successful
 *         status will abort shutdown.
 *
 */
NL_DLL_EXPORT INET_ERROR WillShutdown(Inet::InetLayer *aLayer, void *aContext)
{
    (void)aLayer;
    (void)aContext;

    return INET_NO_ERROR;
}

/**
 * This is a platform-specific InetLayer post-shutdown hook. This
 * may be overridden by assserting the preprocessor definition,
 * #INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS.
 *
 * @param[inout]  aLayer    A pointer to the InetLayer instance being
 *                          shutdown.
 *
 * @param[inout]  aContext  Platform-specific context data passed to
 *                          the layer initialization method, ::Init.
 *
 * @param[in]     anError   The overall status being returned via the
 *                          InetLayer ::Shutdown method.
 *
 */
NL_DLL_EXPORT void DidShutdown(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError)
{
    (void)aLayer;
    (void)aContext;
    (void)anError;

    return;
}

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#if WEAVE_SYSTEM_CONFIG_USE_LWIP && !INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS
/**
 *  This is a platform-specific event / message post hook. This may be
 *  overridden by assserting the preprocessor definition,
 *  #INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS.
 *
 *  This posts an event / message of the specified type with the
 *  provided argument to this instance's platform-specific event /
 *  message queue.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[inout]  aLayer    A pointer to the layer instance to which the
 *                           event / message is being posted.
 *
 *  @param[inout]  aContext  Platform-specific context data passed to
 *                           the layer initialization method, ::Init.
 *
 *  @param[inout]  aTarget   A pointer to the InetLayer object making the
 *                           post request.
 *
 *  @param[in]     aType     The type of event to post.
 *
 *  @param[inout]  anArg     The argument associated with the event to post.
 *
 *  @return #INET_NO_ERROR on success; otherwise, a specific error indicating
 *          the reason for initialization failure.
 *
 */
INET_ERROR PostEvent(Inet::InetLayer* aInetLayer, void* aContext, InetLayerBasis* aTarget, InetEventType aEventType,
    uintptr_t aArgument)
{
    Weave::System::Layer& lSystemLayer = *aInetLayer->mSystemLayer;
    Weave::System::Object& lObject = *aTarget;
    Weave::System::Error lReturn = Weave::System::Platform::Layer::PostEvent(lSystemLayer, aContext, lObject, aEventType,
        aArgument);

    return static_cast<INET_ERROR>(lReturn);
}

/**
 *  This is a platform-specific event / message dispatch hook. This may
 *  be overridden by assserting the preprocessor definition,
 *  INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS.
 *
 *  This effects an event loop, waiting on a queue that services this
 *  instance, pulling events off of that queue, and then dispatching
 *  them for handling.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[inout]  aInetLayer   A pointer to the layer instance for which
 *                              events / messages are being dispatched.
 *
 *  @param[inout]  aContext  Platform-specific context data passed to
 *                           the layer initialization method, ::Init.
 *
 *  @retval   #INET_ERROR_BAD_ARGS          If #aLayer or #aContext is NULL.
 *  @retval   #INET_ERROR_INCORRECT_STATE   If the state of the InetLayer
 *                                          object is incorrect.
 *  @retval   #INET_ERROR_UNEXPECTED_EVENT  If an event type is unrecognized.
 *  @retval   #INET_NO_ERROR                On success.
 *
 */
INET_ERROR DispatchEvents(Inet::InetLayer* aInetLayer, void* aContext)
{
    Weave::System::Layer& lSystemLayer = *aInetLayer->mSystemLayer;
    Weave::System::Error lReturn = Weave::System::Platform::Layer::DispatchEvents(lSystemLayer, aContext);

    return static_cast<INET_ERROR>(lReturn);
}

/**
 *  This is a platform-specific event / message dispatch hook. This may
 *  be overridden by assserting the preprocessor definition,
 *  INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS.
 *
 *  This dispatches the specified event for handling, unmarshalling the
 *  type and arguments from the event for hand off to
 *  InetLayer::HandleEvent for the actual dispatch.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[inout]  aInetLayer   A pointer to the layer instance for which
 *                              events / messages are being dispatched.
 *
 *  @param[inout]  aContext  Platform-specific context data passed to
 *                           the layer initialization method, ::Init.
 *
 *  @param[in]     aEvent    The platform-specific event object to
 *                           dispatch for handling.
 *
 *  @retval   #INET_ERROR_BAD_ARGS          If #aLayer or the event target is
 *                                          NULL.
 *  @retval   #INET_ERROR_UNEXPECTED_EVENT  If the event type is unrecognized.
 *  @retval   #INET_ERROR_INCORRECT_STATE   If the state of the InetLayer
 *                                          object is incorrect.
 *  @retval   #INET_NO_ERROR                On success.
 *
 */
INET_ERROR DispatchEvent(Inet::InetLayer* aInetLayer, void* aContext, InetEvent aEvent)
{
    Weave::System::Layer& lSystemLayer = *aInetLayer->mSystemLayer;
    Weave::System::Error lReturn = Weave::System::Platform::Layer::DispatchEvent(lSystemLayer, aContext, aEvent);

    return static_cast<INET_ERROR>(lReturn);
}

/**
 *  This is a platform-specific event / message dispatch hook. This may be overridden by assserting the preprocessor definition,
 *  #INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[inout]  aInetLayer           A reference to the layer instance for which events / messages are being dispatched.
 *  @param[inout]  aContext             Platform-specific context data passed to the layer initialization method, ::Init.
 *  @param[in]     aMilliseconds        The number of milliseconds to set for the timer.
 *
 *  @retval   #INET_NO_ERROR    Always succeeds unless overridden.
 */
INET_ERROR StartTimer(Inet::InetLayer* aInetLayer, void* aContext, uint32_t aMilliseconds)
{
    Weave::System::Layer& lSystemLayer = *aInetLayer->mSystemLayer;
    Weave::System::Error lReturn = Weave::System::Platform::Layer::StartTimer(lSystemLayer, aContext, aMilliseconds);

    return static_cast<INET_ERROR>(lReturn);
}

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP && !INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

} // namespace InetLayer
} // namespace Platform

#endif // !INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS

} // namespace Inet
} // namespace nl
