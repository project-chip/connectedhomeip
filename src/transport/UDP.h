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
#include <inet/InetInterface.h>
#include <transport/Base.h>

namespace chip {
namespace Transport {

/** Defines listening parameters for setting up a UDP transport */
class UdpListenParameters
{
public:
    explicit UdpListenParameters(Inet::InetLayer * layer) : mLayer(layer) {}
    UdpListenParameters(const UdpListenParameters &) = default;
    UdpListenParameters(UdpListenParameters &&)      = default;

    Inet::InetLayer * GetInetLayer() { return mLayer; }

    Inet::IPAddressType GetAddressType() const { return mAddressType; }
    UdpListenParameters & SetAddressType(Inet::IPAddressType type)
    {
        mAddressType = type;

        return *this;
    }

    uint16_t GetMessageSendPort() const { return mMessageSendPort; }
    UdpListenParameters & SetMessageSendPort(uint16_t port)
    {
        mMessageSendPort = port;

        return *this;
    }

    uint16_t GetListenPort() const { return mListenPort; }
    UdpListenParameters & SetListenPort(uint16_t port)
    {
        mListenPort = port;

        return *this;
    }

    InterfaceId GetInterfaceId() const { return mInterfaceId; }
    UdpListenParameters & SetInterfaceId(InterfaceId id)
    {
        mInterfaceId = id;

        return *this;
    }

private:
    Inet::InetLayer * mLayer         = nullptr;               ///< Associated inet layer
    Inet::IPAddressType mAddressType = kIPAddressType_IPv6;   ///< type of listening socket
    uint16_t mMessageSendPort        = CHIP_PORT;             ///< over what port to send requests
    uint16_t mListenPort             = CHIP_PORT;             ///< UDP listen port
    InterfaceId mInterfaceId         = INET_NULL_INTERFACEID; ///< Interface to listen on
};

/** Implements a transport using UDP. */
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
     * @param params        UDP configuration parameters for this transport
     *
     * @details
     *   Generally send and receive ports should be the same and equal to CHIP_PORT.
     *   The class allows separate definitions to allow local execution of several
     *   Nodes.
     */
    CHIP_ERROR Init(UdpListenParameters & params);

    CHIP_ERROR SendMessage(const MessageHeader & header, const Transport::PeerAddress & address,
                           System::PacketBuffer * msgBuf) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return (mState == State::kInitialized) && (address.GetTransportType() == Type::kUdp) &&
            (address.GetIPAddress().Type() == mUDPEndpointType);
    }

private:
    // UDP message receive handler.
    static void OnUdpReceive(Inet::IPEndPointBasis * endPoint, System::PacketBuffer * buffer, const IPPacketInfo * pktInfo);

    Inet::UDPEndPoint * mUDPEndPoint     = nullptr;                                     ///< UDP socket used by the transport
    Inet::IPAddressType mUDPEndpointType = Inet::IPAddressType::kIPAddressType_Unknown; ///< Socket listening type
    State mState                         = State::kNotReady;                            ///< State of the UDP transport
    uint16_t mSendPort                   = 0;                                           ///< Port where packets are sent by default
};

} // namespace Transport
} // namespace chip

#endif // __UDPTRANSPORT_H__
