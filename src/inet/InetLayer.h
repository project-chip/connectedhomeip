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

#include <inet/EndPointBasis.h>
#include <inet/IANAConstants.h>
#include <inet/IPAddress.h>
#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <inet/InetInterface.h>
#include <inet/TCPEndPoint.h>
#include <inet/UDPEndPoint.h>

#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#include <lib/support/DLLUtil.h>
#include <lib/support/ObjectLifeCycle.h>

#include <stdint.h>

namespace chip {
namespace Inet {

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
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    friend class TCPEndPoint;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    friend class UDPEndPoint;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

public:
    CHIP_ERROR Init(chip::System::Layer & aSystemLayer, void * aContext);

    // Must be called before System::Layer::Shutdown(), since this holds a pointer to that.
    CHIP_ERROR Shutdown();

    chip::System::Layer * SystemLayer() const { return mSystemLayer; }

    // End Points

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    CHIP_ERROR NewTCPEndPoint(TCPEndPoint ** retEndPoint);
#if INET_TCP_IDLE_CHECK_INTERVAL > 0
    static void HandleTCPInactivityTimer(chip::System::Layer * aSystemLayer, void * aAppState);
    bool IsIdleTimerRunning();
#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    CHIP_ERROR NewUDPEndPoint(UDPEndPoint ** retEndPoint);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

    void * GetPlatformData();
    void SetPlatformData(void * aPlatformData);

protected:
    InetLayer();
    virtual ~InetLayer() {}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    friend class TCPEndPointDeletor;

    virtual TCPEndPoint * CreateTCPEndPoint(InetLayer & layer) = 0;
    virtual void DeleteTCPEndPoint(TCPEndPoint * endPoint)     = 0;

    using TCPEndPointVisitor                                          = bool (*)(TCPEndPoint *);
    virtual bool ForEachTCPEndPoint(const TCPEndPointVisitor visitor) = 0;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    friend class UDPEndPointDeletor;
    virtual UDPEndPoint * CreateUDPEndPoint(InetLayer & layer) = 0;
    virtual void DeleteUDPEndPoint(UDPEndPoint * endPoint)     = 0;

    using UDPEndPointVisitor                                         = bool (*)(UDPEndPoint *);
    virtual bool ForEachUDPEndPoint(const UDPEndPointVisitor vistor) = 0;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

private:
    ObjectLifeCycle mLayerState;
    void * mContext;
    void * mPlatformData;
    chip::System::Layer * mSystemLayer;
};

template <typename T, typename U>
class InetLayerPoolImpl : public InetLayer
{
    using TCPEndPointType = T;
    using UDPEndPointType = U;

public:
protected:
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint * CreateTCPEndPoint(InetLayer & layer) override { return sTCPEndPointPool.CreateObject(layer); }
    void DeleteTCPEndPoint(TCPEndPoint * endPoint) override
    {
        sTCPEndPointPool.ReleaseObject(static_cast<TCPEndPointImpl *>(endPoint));
    }
    bool ForEachTCPEndPoint(const TCPEndPointVisitor visitor) override
    {
        return sTCPEndPointPool.ForEachActiveObject(
            [&](TCPEndPoint * endPoint) -> bool { return (&endPoint->Layer() == this) ? visitor(endPoint) : true; });
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    UDPEndPoint * CreateUDPEndPoint(InetLayer & layer) override { return sUDPEndPointPool.CreateObject(layer); }
    void DeleteUDPEndPoint(UDPEndPoint * endPoint) override
    {
        sUDPEndPointPool.ReleaseObject(static_cast<UDPEndPointImpl *>(endPoint));
    }
    bool ForEachUDPEndPoint(const UDPEndPointVisitor visitor) override
    {
        return sUDPEndPointPool.ForEachActiveObject(
            [&](UDPEndPoint * endPoint) -> bool { return (&endPoint->Layer() == this) ? visitor(endPoint) : true; });
    }
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

private:
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    static ObjectPool<TCPEndPointType, INET_CONFIG_NUM_TCP_ENDPOINTS> sTCPEndPointPool;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    static ObjectPool<UDPEndPointType, INET_CONFIG_NUM_UDP_ENDPOINTS> sUDPEndPointPool;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
};

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
template <typename T, typename U>
ObjectPool<T, INET_CONFIG_NUM_TCP_ENDPOINTS> InetLayerPoolImpl<T, U>::sTCPEndPointPool;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
template <typename T, typename U>
ObjectPool<U, INET_CONFIG_NUM_UDP_ENDPOINTS> InetLayerPoolImpl<T, U>::sUDPEndPointPool;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

using InetLayerImpl = InetLayerPoolImpl<TCPEndPointImpl, UDPEndPointImpl>;

/**
 *  @class IPPacketInfo
 *
 *  @brief
 *     Information about an incoming/outgoing message/connection.
 *
 *   @warning
 *     Do not alter the contents of this class without first reading and understanding
 *     the code/comments in UDPEndPoint::GetPacketInfo().
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

} // namespace Inet
} // namespace chip
