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
 *  Shared state for LwIP implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>

struct udp_pcb;
struct tcp_pcb;

namespace chip {
namespace Inet {

class DLL_EXPORT EndPointStateLwIP
{
protected:
    EndPointStateLwIP() : mLwIPEndPointType(LwIPEndPointType::Unknown) {}

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
        UDP     = 1,
        TCP     = 2
    } mLwIPEndPointType;
};

} // namespace Inet
} // namespace chip
