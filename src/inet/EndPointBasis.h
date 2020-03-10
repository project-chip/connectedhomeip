/*
 *
 *    <COPYRIGHT>
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

#ifndef ENDPOINTBASIS_H
#define ENDPOINTBASIS_H

#include "InetConfig.h"
#include "InetError.h"
#include "IANAConstants.h"
#include "InetLayerBasis.h"
#include "InetLayerEvents.h"
#include "InetInterface.h"

#include "support/DLLUtil.h"

//--- Declaration of LWIP protocol control buffer structure names
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
struct raw_pcb;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
struct udp_pcb;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
struct tcp_pcb;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#if INET_CONFIG_ENABLE_TUN_ENDPOINT
struct netif;
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace Inet {

/**
 * @class EndPointBasis
 *
 * @brief Basis of internet transport endpoint classes
 */
class DLL_EXPORT EndPointBasis : public InetLayerBasis
{
public:
    /** Common state codes */
    enum {
        kBasisState_Closed = 0      /**< Encapsulated descriptor is not valid. */
    };

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    /** Test whether endpoint is a POSIX socket */
    bool IsSocketsEndPoint(void) const;
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /** Test whether endpoint is a LwIP protocol control buffer */
    bool IsLWIPEndPoint(void) const;
#endif

    /** Test whether endpoint has a valid descriptor. */
    bool IsOpenEndPoint(void) const;

protected:
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    int mSocket;                    /**< Encapsulated socket descriptor. */
    IPAddressType mAddrType;        /**< Protocol family, i.e. IPv4 or IPv6. */
    SocketEvents mPendingIO;        /**< Socket event masks */
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /** Encapsulated LwIP protocol control block */
    union
    {
        const void* mVoid;          /**< An untyped protocol control buffer reference */
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        raw_pcb* mRaw;              /**< Raw network interface protocol control */
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        udp_pcb* mUDP;              /**< User datagram protocol (UDP) control */
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        tcp_pcb* mTCP;              /**< Transmission control protocol (TCP) control */
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
#if INET_CONFIG_ENABLE_TUN_ENDPOINT
        netif* mNetIf;              /**< Tunnel interface control */
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT
    };

    enum
    {
        kLwIPEndPointType_Unknown = 0,

        kLwIPEndPointType_Raw     = 1,
        kLwIPEndPointType_UDP     = 2,
        kLwIPEndPointType_UCP     = 3,
        kLwIPEndPointType_TCP     = 4
    };

    uint8_t mLwIPEndPointType;

    void DeferredFree(chip::System::Object::ReleaseDeferralErrorTactic aTactic);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    void InitEndPointBasis(InetLayer& aInetLayer, void* aAppState = NULL);
};

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
inline bool EndPointBasis::IsSocketsEndPoint(void) const
{
    return mSocket >= 0;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
inline bool EndPointBasis::IsLWIPEndPoint(void) const
{
    return mVoid != NULL;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

inline bool EndPointBasis::IsOpenEndPoint(void) const
{
    bool lResult = false;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    lResult = (lResult || IsLWIPEndPoint());
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    lResult = (lResult || IsSocketsEndPoint());
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return lResult;
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
inline void EndPointBasis::DeferredFree(chip::System::Object::ReleaseDeferralErrorTactic aTactic)
{
    if (!CHIP_SYSTEM_CONFIG_USE_SOCKETS || IsLWIPEndPoint())
    {
        DeferredRelease(aTactic);
    }
    else
    {
        Release();
    }
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP


} // namespace Inet

#endif // !defined(ENDPOINTBASIS_H)
