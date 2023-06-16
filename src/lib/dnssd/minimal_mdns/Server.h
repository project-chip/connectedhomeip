/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <lib/support/PoolWrapper.h>

#include <lib/dnssd/minimal_mdns/ListenIterator.h>
#include <lib/dnssd/minimal_mdns/core/BytesRange.h>

namespace mdns {
namespace Minimal {

namespace BroadcastIpAddresses {

// Get standard mDNS Broadcast addresses
chip::Inet::IPAddress Get(chip::Inet::IPAddressType addressType);

} // namespace BroadcastIpAddresses

/// Handles mDNS Server Callbacks
class ServerDelegate
{
public:
    virtual ~ServerDelegate() {}

    // Callback of when a query is received
    virtual void OnQuery(const BytesRange & data, const chip::Inet::IPPacketInfo * info) = 0;

    // Callback of when a response is received
    virtual void OnResponse(const BytesRange & data, const chip::Inet::IPPacketInfo * info) = 0;
};

// Defines an mDNS server that listens on one or more interfaces.
//
// I can send and receive mDNS packets (requests/replies)
class ServerBase
{
public:
    class EndpointInfo
    {
    public:
        struct EndPointDeletor
        {
            void operator()(chip::Inet::UDPEndPoint * e) { e->Free(); }
        };

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
        EndpointInfo(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType addressType,
                     std::unique_ptr<chip::Inet::UDPEndPoint, EndPointDeletor> && listenUdp,
                     std::unique_ptr<chip::Inet::UDPEndPoint, EndPointDeletor> && unicastQueryUdp) :
            mInterfaceId(interfaceId),
            mAddressType(addressType), mListenUdp(listenUdp.release()), mUnicastQueryUdp(unicastQueryUdp.release())
        {}
#else
        EndpointInfo(chip::Inet::InterfaceId interfaceId, chip::Inet::IPAddressType addressType,
                     std::unique_ptr<chip::Inet::UDPEndPoint, EndPointDeletor> && listenUdp) :
            mInterfaceId(interfaceId),
            mAddressType(addressType), mListenUdp(listenUdp.release())
        {}
#endif

        ~EndpointInfo()
        {
            if (mListenUdp != nullptr)
            {
                mListenUdp->Free();
            }

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
            if (mUnicastQueryUdp != nullptr)
            {
                mUnicastQueryUdp->Free();
            }
#endif
        }

        const chip::Inet::InterfaceId mInterfaceId;
        const chip::Inet::IPAddressType mAddressType;
        chip::Inet::UDPEndPoint * const mListenUdp;
#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
        chip::Inet::UDPEndPoint * const mUnicastQueryUdp;
#endif
    };

    /**
     * Helps implement a generic broadcast implementation:
     *    - provides the ability to determine what udp endpoint to use  to broadcast
     *      a packet for the given endpoint info
     */
    class BroadcastSendDelegate
    {
    public:
        virtual ~BroadcastSendDelegate() = default;

        /**
         * Returns non-null UDPEndpoint IFF a broadcast should be performed for the given EndpointInfo
         */
        virtual chip::Inet::UDPEndPoint * Accept(ServerBase::EndpointInfo * info) = 0;
    };

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
    using EndpointInfoPoolType = chip::PoolInterface<EndpointInfo, chip::Inet::InterfaceId, chip::Inet::IPAddressType,
                                                     std::unique_ptr<chip::Inet::UDPEndPoint, EndpointInfo::EndPointDeletor> &&,
                                                     std::unique_ptr<chip::Inet::UDPEndPoint, EndpointInfo::EndPointDeletor> &&>;
#else
    using EndpointInfoPoolType = chip::PoolInterface<EndpointInfo, chip::Inet::InterfaceId, chip::Inet::IPAddressType,
                                                     std::unique_ptr<chip::Inet::UDPEndPoint, EndpointInfo::EndPointDeletor> &&>;
#endif

    ServerBase(EndpointInfoPoolType & pool) : mEndpoints(pool)
    {
        mIpv6BroadcastAddress = BroadcastIpAddresses::Get(chip::Inet::IPAddressType::kIPv6);
#if INET_CONFIG_ENABLE_IPV4
        mIpv4BroadcastAddress = BroadcastIpAddresses::Get(chip::Inet::IPAddressType::kIPv4);
#endif
    }
    virtual ~ServerBase();

    /// Closes all currently open endpoints and resets the 'initialized' flag
    void Shutdown();

    void ShutdownEndpoints();
    void ShutdownEndpoint(EndpointInfo & aEndpoint);

    /// Listen on the given interfaces/address types.
    ///
    /// Since mDNS uses link-local addresses, one generally wants to listen on all
    /// non-loopback interfaces.
    CHIP_ERROR Listen(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager, ListenIterator * it,
                      uint16_t port);

    /// Send the specified packet to a destination IP address over the specified address
    virtual CHIP_ERROR DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                                  chip::Inet::InterfaceId interface);

    /// Send out a broadcast query, may use an ephemeral port to receive replies.
    /// Ephemeral ports will make replies be marked as 'LEGACY' and replies will include a query secion.
    virtual CHIP_ERROR BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port);

    /// Send a specific packet broadcast to a specific interface using a specific address type
    /// May use an ephemeral port to receive replies.
    /// Ephemeral ports will make replies be marked as 'LEGACY' and replies will include a query secion.
    virtual CHIP_ERROR BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port,
                                             chip::Inet::InterfaceId interface, chip::Inet::IPAddressType addressType);

    /// Send a specific packet broadcast to all interfaces
    virtual CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port);

    /// Send a specific packet broadcast to a specific interface using a specific address type
    virtual CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface,
                                     chip::Inet::IPAddressType addressType);

    ServerBase & SetDelegate(ServerDelegate * d)
    {
        mDelegate = d;
        return *this;
    }

    /// A server is considered listening if any UDP endpoint is active.
    ///
    /// This is expected to return false after any Shutdown() and will
    /// return true IFF lListen was called and the listen iterator successfully
    /// found a valid listening interface.
    bool IsListening() const;

private:
    CHIP_ERROR BroadcastImpl(chip::System::PacketBufferHandle && data, uint16_t port, BroadcastSendDelegate * delegate);

    static void OnUdpPacketReceived(chip::Inet::UDPEndPoint * endPoint, chip::System::PacketBufferHandle && buffer,
                                    const chip::Inet::IPPacketInfo * info);

    EndpointInfoPoolType & mEndpoints; // possible endpoints, to listen on multiple interfaces
    ServerDelegate * mDelegate = nullptr;

    // Broadcast IP addresses are cached to not require a string parse every time
    // Ideally we should be able to constexpr these
    chip::Inet::IPAddress mIpv6BroadcastAddress;
#if INET_CONFIG_ENABLE_IPV4
    chip::Inet::IPAddress mIpv4BroadcastAddress;
#endif
    bool mIsInitialized = false;
};

// The PoolImpl impl is used as a base class because its destructor must be called after ServerBase's destructor.
template <size_t kCount>
class Server : private chip::PoolImpl<ServerBase::EndpointInfo, kCount, chip::ObjectPoolMem::kInline,
                                      ServerBase::EndpointInfoPoolType::Interface>,
               public ServerBase
{
public:
    Server() : ServerBase(*static_cast<ServerBase::EndpointInfoPoolType *>(this)) {}
    ~Server() override {}
};

} // namespace Minimal
} // namespace mdns
