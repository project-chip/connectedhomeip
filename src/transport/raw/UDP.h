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
 *      It binds to any available local addr and port and begins listening.
 *
 */

#pragma once

#include <utility>

#include <inet/IPAddress.h>
#include <inet/IPPacketInfo.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPCore.h>
#include <transport/raw/Base.h>

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
struct otInstance;
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

namespace chip {
namespace Transport {

/** Defines listening parameters for setting up a UDP transport */
class UdpListenParameters
{
public:
    explicit UdpListenParameters(Inet::EndPointManager<Inet::UDPEndPoint> * endPointManager) : mEndPointManager(endPointManager) {}
    UdpListenParameters(const UdpListenParameters &) = default;
    UdpListenParameters(UdpListenParameters &&)      = default;

    Inet::EndPointManager<Inet::UDPEndPoint> * GetEndPointManager() { return mEndPointManager; }

    Inet::IPAddressType GetAddressType() const { return mAddressType; }
    UdpListenParameters & SetAddressType(Inet::IPAddressType type)
    {
        mAddressType = type;

        return *this;
    }

    uint16_t GetListenPort() const { return mListenPort; }
    UdpListenParameters & SetListenPort(uint16_t port)
    {
        mListenPort = port;

        return *this;
    }

    Inet::InterfaceId GetInterfaceId() const { return mInterfaceId; }
    UdpListenParameters & SetInterfaceId(Inet::InterfaceId id)
    {
        mInterfaceId = id;

        return *this;
    }

    /**
     * Networking Stack Native parameter (optional)
     */
    void * GetNativeParams() const { return mNativeParams; }
    UdpListenParameters & SetNativeParams(void * params)
    {
        mNativeParams = params;

        return *this;
    }

private:
    Inet::EndPointManager<Inet::UDPEndPoint> * mEndPointManager;   ///< Associated endpoint factory
    Inet::IPAddressType mAddressType = Inet::IPAddressType::kIPv6; ///< type of listening socket
    uint16_t mListenPort             = CHIP_PORT;                  ///< UDP listen port
    Inet::InterfaceId mInterfaceId   = Inet::InterfaceId::Null();  ///< Interface to listen on
    void * mNativeParams             = nullptr;
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
    ~UDP() override;

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

    uint16_t GetBoundPort();

    /**
     * Close the open endpoint without destroying the object
     */
    void Close() override;

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

    CHIP_ERROR MulticastGroupJoinLeave(const Transport::PeerAddress & address, bool join) override;

    bool CanListenMulticast() override
    {
        return (mState == State::kInitialized) && (mUDPEndpointType == Inet::IPAddressType::kIPv6);
    }

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return (mState == State::kInitialized) && (address.GetTransportType() == Type::kUdp) &&
            (address.GetIPAddress().Type() == mUDPEndpointType);
    }

private:
    // UDP message receive handler.
    static void OnUdpReceive(Inet::UDPEndPoint * endPoint, System::PacketBufferHandle && buffer,
                             const Inet::IPPacketInfo * pktInfo);

    static void OnUdpError(Inet::UDPEndPoint * endPoint, CHIP_ERROR err, const Inet::IPPacketInfo * pktInfo);

    Inet::UDPEndPoint * mUDPEndPoint     = nullptr;                       ///< UDP socket used by the transport
    Inet::IPAddressType mUDPEndpointType = Inet::IPAddressType::kUnknown; ///< Socket listening type
    State mState                         = State::kNotReady;              ///< State of the UDP transport
};

} // namespace Transport
} // namespace chip
