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
 *      This file defines the CHIP Connection object that maintains TCP connections.
 *      It binds to any avaiable local addr and port and begins listening.
 */

#ifndef __TCPTRANSPORT_H__
#define __TCPTRANSPORT_H__

#include <utility>

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <inet/InetInterface.h>
#include <transport/Base.h>

namespace chip {
namespace Transport {

/** Defines listening parameters for setting up a TCP transport */
class TcpListenParameters
{
public:
    explicit TcpListenParameters(Inet::InetLayer * layer) : mLayer(layer) {}
    TcpListenParameters(const TcpListenParameters &) = default;
    TcpListenParameters(TcpListenParameters &&)      = default;

    Inet::InetLayer * GetInetLayer() { return mLayer; }

    Inet::IPAddressType GetAddressType() const { return mAddressType; }
    TcpListenParameters & SetAddressType(Inet::IPAddressType type)
    {
        mAddressType = type;

        return *this;
    }

    uint16_t GetListenPort() const { return mListenPort; }
    TcpListenParameters & SetListenPort(uint16_t port)
    {
        mListenPort = port;

        return *this;
    }

    InterfaceId GetInterfaceId() const { return mInterfaceId; }
    TcpListenParameters & SetInterfaceId(InterfaceId id)
    {
        mInterfaceId = id;

        return *this;
    }

private:
    Inet::InetLayer * mLayer         = nullptr;               ///< Associated inet layer
    Inet::IPAddressType mAddressType = kIPAddressType_IPv6;   ///< type of listening socket
    uint16_t mListenPort             = CHIP_PORT;             ///< TCP listen port
    InterfaceId mInterfaceId         = INET_NULL_INTERFACEID; ///< Interface to listen on
};

/** Implements a transport using TCP. */
class DLL_EXPORT TCP : public Base
{
    /**
     *  The State of the TCP connection
     *
     */
    enum class State
    {
        kNotReady    = 0, /**< State before initialization. */
        kInitialized = 1, /**< State after class is listening and ready. */
    };

public:
    virtual ~TCP();

    /**
     * Initialize a TCP transport on a given port.
     *
     * @param params        TCP configuration parameters for this transport
     *
     * @details
     *   Generally send and receive ports should be the same and equal to CHIP_PORT.
     *   The class allows separate definitions to allow local execution of several
     *   Nodes.
     */
    CHIP_ERROR Init(TcpListenParameters & params);

    CHIP_ERROR SendMessage(const MessageHeader & header, const Transport::PeerAddress & address,
                           System::PacketBuffer * msgBuf) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return (mState == State::kInitialized) && (address.GetTransportType() == Type::kTcp) &&
            (address.GetIPAddress().Type() == mEndpointType);
    }

private:
    // TCP message receive handler.
    static void OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBuffer * buffer);
    // FIXME: accept, error, ...

    Inet::TCPEndPoint * mListenSocket = nullptr;                                     ///< TCP socket used by the transport
    Inet::IPAddressType mEndpointType = Inet::IPAddressType::kIPAddressType_Unknown; ///< Socket listening type
    State mState                      = State::kNotReady;                            ///< State of the TCP transport
};

} // namespace Transport
} // namespace chip

#endif // __TCPTRANSPORT_H__
