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
 *      BSD/POSIX Sockets, LwIP or Network.framework.
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

#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inet/InetConfig.h>

#include <inet/IANAConstants.h>
#include <inet/IPAddress.h>
#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <inet/InetInterface.h>
#include <inet/InetLayerBasis.h>
#include <inet/InetLayerEvents.h>

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#include <inet/DNSResolver.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
#include <inet/RawEndPoint.h>
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <inet/TCPEndPoint.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
#include <inet/UDPEndPoint.h>
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#include <inet/AsyncDNSResolverSockets.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#include <lib/support/DLLUtil.h>

#if INET_CONFIG_MAX_DROPPABLE_EVENTS

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#include <semaphore.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#else
#include <FreeRTOS.h>
#endif
#include <semphr.h>
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#endif // INET_CONFIG_MAX_DROPPABLE_EVENTS

#include <stdint.h>

namespace chip {
namespace Inet {

// Forward Declarations

class InetLayer;

namespace Platform {
namespace InetLayer {

extern CHIP_ERROR WillInit(Inet::InetLayer * aLayer, void * aContext);
extern void DidInit(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError);

extern CHIP_ERROR WillShutdown(Inet::InetLayer * aLayer, void * aContext);
extern void DidShutdown(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError);

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
class DLL_EXPORT InetLayer
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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    friend class AsyncDNSResolverSockets;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

public:
    /**
     *  The current state of the InetLayer object.
     *
     */
    volatile enum {
        kState_NotInitialized     = 0, /**< Not initialized state. */
        kState_Initialized        = 1, /**< Initialized state. */
        kState_ShutdownInProgress = 2, /**< State where Shutdown has been triggered. */
    } State;                           /**< [READ-ONLY] Current state. */

    InetLayer();

    CHIP_ERROR Init(chip::System::Layer & aSystemLayer, void * aContext);

    // Must be called before System::Layer::Shutdown(), since this holds a pointer to that.
    CHIP_ERROR Shutdown();

    chip::System::Layer * SystemLayer() const;

    // End Points

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    CHIP_ERROR NewRawEndPoint(IPVersion ipVer, IPProtocol ipProto, RawEndPoint ** retEndPoint);
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    CHIP_ERROR NewTCPEndPoint(TCPEndPoint ** retEndPoint);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    CHIP_ERROR NewUDPEndPoint(UDPEndPoint ** retEndPoint);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

    // DNS Resolution

#if INET_CONFIG_ENABLE_DNS_RESOLVER

    typedef DNSResolver::OnResolveCompleteFunct DNSResolveCompleteFunct;

    CHIP_ERROR ResolveHostAddress(const char * hostName, uint16_t hostNameLen, uint8_t options, uint8_t maxAddrs,
                                  IPAddress * addrArray, DNSResolveCompleteFunct onComplete, void * appState);
    CHIP_ERROR ResolveHostAddress(const char * hostName, uint16_t hostNameLen, uint8_t maxAddrs, IPAddress * addrArray,
                                  DNSResolveCompleteFunct onComplete, void * appState);
    CHIP_ERROR ResolveHostAddress(const char * hostName, uint8_t maxAddrs, IPAddress * addrArray,
                                  DNSResolveCompleteFunct onComplete, void * appState);
    void CancelResolveHostAddress(DNSResolveCompleteFunct onComplete, void * appState);

#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    CHIP_ERROR GetInterfaceFromAddr(const IPAddress & addr, InterfaceId & intfId);

    CHIP_ERROR GetLinkLocalAddr(InterfaceId link, IPAddress * llAddr);
    bool MatchLocalIPv6Subnet(const IPAddress & addr);

    static void UpdateSnapshot(chip::System::Stats::Snapshot & aSnapshot);

    void * GetPlatformData();
    void SetPlatformData(void * aPlatformData);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static CHIP_ERROR HandleInetLayerEvent(chip::System::Object & aTarget, chip::System::EventType aEventType, uintptr_t aArgument);

    static chip::System::LwIPEventHandlerDelegate sInetEventHandlerDelegate;

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
    inline static bool IsDroppableEvent(chip::System::EventType type)
    {
        return
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
            type == kInetEvent_UDPDataReceived ||
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
            type == kInetEvent_RawDataReceived ||
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
            false;
    }

    CHIP_ERROR InitQueueLimiter(void);
    bool CanEnqueueDroppableEvent(void);
    void DroppableEventDequeued(void);

#if CHIP_SYSTEM_CONFIG_NO_LOCKING
    volatile int32_t mDroppableEvents;
#elif CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    sem_t mDroppableEvents;
#elif CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t mDroppableEventsObj;
#endif // (configSUPPORT_STATIC_ALLOCATION == 1)
    SemaphoreHandle_t mDroppableEvents;
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#else  // !INET_CONFIG_MAX_DROPPABLE_EVENTS

    inline static bool IsDroppableEvent(chip::System::EventType aType) { return false; }

    inline CHIP_ERROR InitQueueLimiter(void) { return CHIP_NO_ERROR; }
    inline bool CanEnqueueDroppableEvent(void) { return true; }
    inline void DroppableEventDequeued(void) { return; }
#endif // !INET_CONFIG_MAX_DROPPABLE_EVENTS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
    static void HandleTCPInactivityTimer(chip::System::Layer * aSystemLayer, void * aAppState);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

private:
    void * mContext;
    void * mPlatformData;
    chip::System::Layer * mSystemLayer;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    AsyncDNSResolverSockets mAsyncDNSResolver;
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    friend CHIP_ERROR Platform::InetLayer::WillInit(Inet::InetLayer * aLayer, void * aContext);
    friend void Platform::InetLayer::DidInit(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError);

    friend CHIP_ERROR Platform::InetLayer::WillShutdown(Inet::InetLayer * aLayer, void * aContext);
    friend void Platform::InetLayer::DidShutdown(Inet::InetLayer * aLayer, void * aContext, CHIP_ERROR anError);

    bool IsIdleTimerRunning();
};

inline chip::System::Layer * InetLayer::SystemLayer() const
{
    return mSystemLayer;
}

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
    IPAddress SrcAddress;  /**< The source IPAddress in the packet. */
    IPAddress DestAddress; /**< The destination IPAddress in the packet. */
    InterfaceId Interface; /**< The interface identifier for the connection. */
    uint16_t SrcPort;      /**< The source port in the packet. */
    uint16_t DestPort;     /**< The destination port in the packet. */

    void Clear();
};

extern CHIP_ERROR ParseHostAndPort(const char * aString, uint16_t aStringLen, const char *& aHost, uint16_t & aHostLen,
                                   uint16_t & aPort);

extern CHIP_ERROR ParseHostPortAndInterface(const char * aString, uint16_t aStringLen, const char *& aHost, uint16_t & aHostLen,
                                            uint16_t & aPort, const char *& aInterface, uint16_t & aInterfaceLen);

} // namespace Inet
} // namespace chip
