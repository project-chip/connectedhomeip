/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the CHIP Connection object that maintains a UDP connection.
 *      It binds to any avaiable local addr and port and begins listening.
 *      TODO This class should be extended to support TCP as well...
 *
 */

#ifndef __UDPTRANSPORT_H__
#define __UDPTRANSPORT_H__

#include <utility>

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <transport/Base.h>

namespace chip {
namespace Transport {

class DLL_EXPORT UDP : public Base
{
    /**
     *  The State of the UDP connection
     *
     */
    enum class State
    {
        kNotReady    = 0, /**< State before initialization. */
        kInitialized = 1, /**< State after class is listening and ready. */
    };

public:
    virtual ~UDP();

    /**
     * Initialize a UDP transport on a given port.
     *
     * @param inetLayer    underlying communication channel
     * @param addrType     IP Type for listening. IPV6 except for testing ipv4;
     * @param sendPort     messages to other peers are sent at this port
     * @param receivePort  listening port to receive messages
     *
     * @details
     *   Generally send and receive ports should be the same and equal to CHIP_PORT.
     *   The class allows separate definitions to allow local execution of several
     *   Nodes.
     */
    CHIP_ERROR Init(Inet::InetLayer * inetLayer, Inet::IPAddressType addrType, uint16_t sendPort, uint16_t receivePort);

    /**
     * Convenience method to listen on IPv6 on chip standard ports
     */
    CHIP_ERROR Init(Inet::InetLayer * inetLayer) { return Init(inetLayer, kIPAddressType_IPv6, CHIP_PORT, CHIP_PORT); }

    Type GetType() override { return Type::kUdp; }
    CHIP_ERROR SendMessage(const MessageHeader & header, Inet::IPAddress address, System::PacketBuffer * msgBuf) override;

private:
    // UDP message receive handler.
    static void OnUdpReceive(Inet::IPEndPointBasis * endPoint, System::PacketBuffer * buffer, const IPPacketInfo * pktInfo);

    Inet::UDPEndPoint * mUDPEndPoint = nullptr;          ///< UDP socket used by the transport
    State mState                     = State::kNotReady; ///< State of the UDP transport
    uint16_t mSendPort               = 0;                ///< Port where packets are sent by default
};

} // namespace Transport
} // namespace chip

#endif // __UDPTRANSPORT_H__
