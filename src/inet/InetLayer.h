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
 *      For LwIP, event readiness notification is handled via events /
 *      messages and platform- and system-specific hooks for the event
 *      / message system.
 *
 */

#ifndef INETLAYER_H
#define INETLAYER_H

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>

#include <InetLayer/InetConfig.h>

#include <SystemLayer/SystemLayer.h>
#include <SystemLayer/SystemStats.h>

#include <Weave/Support/NLDLLUtil.h>

#include <InetLayer/IANAConstants.h>
#include <InetLayer/InetError.h>
#include <InetLayer/IPAddress.h>
#include <InetLayer/IPPrefix.h>
#include <InetLayer/InetInterface.h>
#include <InetLayer/InetLayerBasis.h>
#include <InetLayer/InetLayerEvents.h>

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#include <InetLayer/DNSResolver.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
#include <InetLayer/RawEndPoint.h>
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <InetLayer/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <InetLayer/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
#include <InetLayer/TunEndPoint.h>
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#include <InetLayer/InetBuffer.h>
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#include <InetLayer/AsyncDNSResolverSockets.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if INET_CONFIG_MAX_DROPPABLE_EVENTS

#if WEAVE_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#include <semaphore.h>
#endif // WEAVE_SYSTEM_CONFIG_POSIX_LOCKING

#if WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
#include <FreeRTOS.h>
#include <semphr.h>
#endif // WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

#endif // INET_CONFIG_MAX_DROPPABLE_EVENTS

namespace nl {
namespace Inet {

// Forward Declarations

class InetLayer;

namespace Platform {
namespace InetLayer {

    extern INET_ERROR WillInit(Inet::InetLayer *aLayer, void *aContext);
    extern void       DidInit(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError);

    extern INET_ERROR WillShutdown(Inet::InetLayer *aLayer, void *aContext);
    extern void       DidShutdown(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError);

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    extern INET_ERROR PostEvent(Inet::InetLayer *aLayer, void *aContext, InetLayerBasis *aTarget, InetEventType aType,
        uintptr_t anArg);
    extern INET_ERROR DispatchEvents(Inet::InetLayer *aLayer, void *aContext);
    extern INET_ERROR DispatchEvent(Inet::InetLayer *aLayer, void *aContext, InetEvent anEvent);
    extern INET_ERROR StartTimer(Inet::InetLayer *aLayer, void *aContext, uint32_t aDurMS);

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

} // namespace InetLayer
} // namespace Platform

/**
 *  @class InetLayer
 *
 *  @brief
 *    This provides access to Internet services, including timers,
 *    Domain Name System (DNS) resolution, TCP network transport, UDP
 *    network transport, and raw network transport, for a single
 *    thread.
 *
 *    For BSD/POSIX Sockets, event readiness notification is handled
 *    via file descriptors and a traditional poll / select
 *    implementation on the platform adaptation.
 *
 *    For LwIP, event readiness notification is handle via events /
 *    messages and platform- and system-specific hooks for the event /
 *    message system.
 *
 */
class NL_DLL_EXPORT InetLayer
{
#if INET_CONFIG_ENABLE_DNS_RESOLVER
    friend class DNSResolver;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    friend class RawEndPoint;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    friend class TCPEndPoint;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    friend class UDPEndPoint;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
    friend class TunEndPoint;
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    friend class AsyncDNSResolverSockets;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

  public:
    /**
     *  The current state of the InetLayer object.
     *
     */
    volatile enum
    {
        kState_NotInitialized = 0,              /**< Not initialized state. */
        kState_Initialized = 1,                 /**< Initialized state. */
        kState_ShutdownInProgress = 2,          /**< State where Shutdown has been triggered. */
    } State;                                    /**< [READ-ONLY] Current state. */

    InetLayer(void);

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    INET_ERROR Init(void *aContext);
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    INET_ERROR Init(Weave::System::Layer& aSystemLayer, void* aContext);
    INET_ERROR Shutdown(void);

    Weave::System::Layer* SystemLayer(void) const;

    // End Points

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    INET_ERROR NewRawEndPoint(IPVersion ipVer, IPProtocol ipProto, RawEndPoint **retEndPoint);
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    INET_ERROR NewTCPEndPoint(TCPEndPoint **retEndPoint);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    INET_ERROR NewUDPEndPoint(UDPEndPoint **retEndPoint);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TUN_ENDPOINT
    INET_ERROR NewTunEndPoint(TunEndPoint **retEndPoint);
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT

    // DNS Resolution

#if INET_CONFIG_ENABLE_DNS_RESOLVER

    typedef DNSResolver::OnResolveCompleteFunct DNSResolveCompleteFunct;

    INET_ERROR ResolveHostAddress(const char *hostName, uint16_t hostNameLen, uint8_t options,
            uint8_t maxAddrs, IPAddress *addrArray,
            DNSResolveCompleteFunct onComplete, void *appState);
    INET_ERROR ResolveHostAddress(const char *hostName, uint16_t hostNameLen,
            uint8_t maxAddrs, IPAddress *addrArray,
            DNSResolveCompleteFunct onComplete, void *appState);
    INET_ERROR ResolveHostAddress(const char *hostName, uint8_t maxAddrs, IPAddress *addrArray,
            DNSResolveCompleteFunct onComplete, void *appState);
    void CancelResolveHostAddress(DNSResolveCompleteFunct onComplete, void *appState);

#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    INET_ERROR GetInterfaceFromAddr(const IPAddress& addr, InterfaceId& intfId);

    INET_ERROR GetLinkLocalAddr(InterfaceId link, IPAddress *llAddr);
    bool MatchLocalIPv6Subnet(const IPAddress& addr);

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    /**
     *  @brief
     *    This function is the callback to be invoked when a one-shot timer fires.
     *
     *    @param[in]   inetLayer  A pointer to the InetLayer instance this timer was
     *                            started on.
     *
     *    @param[in]   appState   A pointer to an application state object registered
     *                            when this timer was started.
     *
     *    @param[in]   err        #INET_NO_ERROR unconditionally.
     */
    typedef void (*TimerCompleteFunct)(InetLayer *inetLayer, void *appState, INET_ERROR err);
    INET_ERROR StartTimer(uint32_t durMS, TimerCompleteFunct onComplete, void *appState);
    void CancelTimer(TimerCompleteFunct onComplete, void *appState);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    INET_ERROR PostEvent(InetLayerBasis *target, InetEventType type, uintptr_t arg);
    INET_ERROR DispatchEvents(void);
    INET_ERROR DispatchEvent(InetEvent aEvent);
    INET_ERROR HandleEvent(InetLayerBasis &target, InetEventType type, uintptr_t arg);

    // Timer Management
    INET_ERROR StartPlatformTimer(uint32_t inDurMS);
    INET_ERROR HandlePlatformTimer(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    void WakeSelect(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    void PrepareSelect(int& nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval& sleepTime);
    void HandleSelectResult(int selectRes, fd_set *readfds, fd_set *writefds, fd_set *exceptfds);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    static void UpdateSnapshot(nl::Weave::System::Stats::Snapshot &aSnapshot);

    void *GetPlatformData(void);
    void SetPlatformData(void *aPlatformData);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    static Weave::System::Error HandleInetLayerEvent(Weave::System::Object& aTarget, Weave::System::EventType aEventType,
        uintptr_t aArgument);

    static Weave::System::LwIPEventHandlerDelegate sInetEventHandlerDelegate;

    // In some implementations, there may be a shared event / message
    // queue for the InetLayer used by other system events / messages.
    //
    // If the length of that queue is considerably longer than the
    // number of packet buffers available, it may lead to buffer
    // exhaustion. As a result, using the queue itself to implement
    // backpressure is insufficient, and we need an external mechanism
    // to prevent buffer starvation in the rest of the system and
    // getting into deadlock situations.

    // For both UDP and raw network transport traffic we can easily
    // drop incoming packets without impacting the correctness of
    // higher level protocols.

#if INET_CONFIG_MAX_DROPPABLE_EVENTS
    inline static bool       IsDroppableEvent(Weave::System::EventType type)
    {
        return
#if INET_CONFIG_ENABLE_TUN_ENDPOINT
            type == kInetEvent_TunDataReceived ||
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
            type == kInetEvent_UDPDataReceived ||
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
            type == kInetEvent_RawDataReceived ||
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
            false;
    }

    INET_ERROR        InitQueueLimiter(void);
    bool              CanEnqueueDroppableEvent(void);
    void              DroppableEventDequeued(void);

#if WEAVE_SYSTEM_CONFIG_NO_LOCKING
    volatile int32_t mDroppableEvents;
#elif WEAVE_SYSTEM_CONFIG_POSIX_LOCKING
    sem_t mDroppableEvents;
#elif WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t mDroppableEventsObj;
#endif // (configSUPPORT_STATIC_ALLOCATION == 1)
    SemaphoreHandle_t mDroppableEvents;
#endif // WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

#else // !INET_CONFIG_MAX_DROPPABLE_EVENTS

    inline static bool IsDroppableEvent(Weave::System::EventType aType)      { return false; }

    inline INET_ERROR InitQueueLimiter(void)                      { return INET_NO_ERROR; }
    inline bool       CanEnqueueDroppableEvent(void)              { return true; }
    inline void       DroppableEventDequeued(void)                { return; }
#endif // !INET_CONFIG_MAX_DROPPABLE_EVENTS
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    Weave::System::Layer    mImplicitSystemLayer;
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
    static void HandleTCPInactivityTimer(Weave::System::Layer* aSystemLayer, void* aAppState, Weave::System::Error aError);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

private:
    void*                   mContext;
    void*                   mPlatformData;
    Weave::System::Layer*   mSystemLayer;

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    AsyncDNSResolverSockets mAsyncDNSResolver;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS


#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    friend INET_ERROR Platform::InetLayer::WillInit(Inet::InetLayer *aLayer, void *aContext);
    friend void       Platform::InetLayer::DidInit(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError);

    friend INET_ERROR Platform::InetLayer::WillShutdown(Inet::InetLayer *aLayer, void *aContext);
    friend void       Platform::InetLayer::DidShutdown(Inet::InetLayer *aLayer, void *aContext, INET_ERROR anError);

    bool IsIdleTimerRunning(void);

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    friend INET_ERROR Platform::InetLayer::PostEvent(Inet::InetLayer *aLayer, void *aContext, InetLayerBasis *aTarget,
        InetEventType aType, uintptr_t anArg);
    friend INET_ERROR Platform::InetLayer::DispatchEvents(Inet::InetLayer *aLayer, void *aContext);
    friend INET_ERROR Platform::InetLayer::DispatchEvent(Inet::InetLayer *aLayer, void *aContext, InetEvent anEvent);
    friend INET_ERROR Platform::InetLayer::StartTimer(Inet::InetLayer *aLayer, void *aContext, uint32_t aDurMS);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
};

inline Weave::System::Layer* InetLayer::SystemLayer(void) const
{
    return mSystemLayer;
}

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
inline INET_ERROR InetLayer::Init(void* aContext)
{
    return Init(mImplicitSystemLayer, aContext);
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
inline INET_ERROR InetLayer::DispatchEvent(InetEvent aEvent)
{
    return mSystemLayer->DispatchEvent(aEvent);
}

inline INET_ERROR InetLayer::HandleEvent(InetLayerBasis& target, InetEventType type, uintptr_t arg)
{
    return mSystemLayer->HandleEvent(target, type, arg);
}
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
inline void InetLayer::WakeSelect(void)
{
    mSystemLayer->WakeSelect();
}
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  @class IPPacketInfo
 *
 *  @brief
 *     Information about an incoming/outgoing message/connection.
 *
 *   @warning
 *     Do not alter the contents of this class without first reading and understanding
 *     the code/comments in IPEndPointBasis::GetPacketInfo().
 */
class IPPacketInfo
{
public:
    IPAddress SrcAddress;                /**< The source IPAddress in the packet. */
    IPAddress DestAddress;               /**< The destination IPAddress in the packet. */
    InterfaceId Interface;               /**< The interface identifier for the connection. */
    uint16_t SrcPort;                    /**< The source port in the packet. */
    uint16_t DestPort;                   /**< The destination port in the packet. */

    void Clear(void);
};


extern INET_ERROR ParseHostAndPort(const char *aString, uint16_t aStringLen, const char *&aHost, uint16_t &aHostLen, uint16_t &aPort);

extern INET_ERROR ParseHostPortAndInterface(const char *aString, uint16_t aStringLen, const char *&aHost, uint16_t &aHostLen, uint16_t &aPort, const char *&aInterface, uint16_t &aInterfaceLen);

} // namespace Inet
} // namespace nl

#endif // !defined(INETLAYER_H)
