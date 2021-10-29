/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file contains the basis class for all the various transport
 *      endpoint classes in the Inet layer, i.e. TCP, UDP, Raw and Tun.
 */

#pragma once

#include <inet/InetConfig.h>

#include <inet/IPAddress.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemObject.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SocketEvents.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#include <Network/Network.h>
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_LWIP
struct udp_pcb;
struct tcp_pcb;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace Inet {

class InetLayer;

/**
 * Basis of internet transport endpoint classes.
 */
class DLL_EXPORT EndPointBasis : public System::Object
{
public:
    /**
     *  Returns a reference to the Inet layer object that owns this basis object.
     */
    InetLayer & Layer() const { return *mInetLayer; }

    /**
     *  Returns \c true if the basis object was obtained by the specified Inet layer instance.
     *
     *  @note
     *      Does not check whether the object is actually obtained by the system layer instance associated with the Inet layer
     *      instance. It merely tests whether \c aInetLayer is the Inet layer instance that was provided to \c InitInetLayerBasis.
     */
    bool IsCreatedByInetLayer(const InetLayer & aInetLayer) const { return mInetLayer == &aInetLayer; }

private:
    InetLayer * mInetLayer; /**< Pointer to the InetLayer object that owns this object. */

protected:
    void InitEndPointBasis(InetLayer & aInetLayer, void * aAppState = nullptr);
    void DeferredFree(System::Object::ReleaseDeferralErrorTactic aTactic);

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    nw_parameters_t mParameters;
    IPAddressType mAddrType; /**< Protocol family, i.e. IPv4 or IPv6. */
#endif

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    static constexpr int kInvalidSocketFd = -1;
    int mSocket;                     /**< Encapsulated socket descriptor. */
    IPAddressType mAddrType;         /**< Protocol family, i.e. IPv4 or IPv6. */
    System::SocketWatchToken mWatch; /**< Socket event watcher */
#endif                               // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /** Encapsulated LwIP protocol control block */
    union
    {
        const void * mVoid; /**< An untyped protocol control buffer reference */
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        udp_pcb * mUDP; /**< User datagram protocol (UDP) control */
#endif                  // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        tcp_pcb * mTCP; /**< Transmission control protocol (TCP) control */
#endif                  // INET_CONFIG_ENABLE_TCP_ENDPOINT
    };

    enum class LwIPEndPointType : uint8_t
    {
        Unknown = 0,
        Raw     = 1,
        UDP     = 2,
        UCP     = 3,
        TCP     = 4
    } mLwIPEndPointType;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
};

} // namespace Inet
} // namespace chip
