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

#include <lib/dnssd/minimal_mdns/ListenIterator.h>
#include <lib/dnssd/minimal_mdns/core/BytesRange.h>

namespace mdns {
namespace Minimal {

namespace BroadcastIpAddresses {

// Get standard mDNS Broadcast addresses

void GetIpv6Into(chip::Inet::IPAddress & dest);
void GetIpv4Into(chip::Inet::IPAddress & dest);

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
class Server
{
public:
    Server()
    {
        BroadcastIpAddresses::GetIpv6Into(mIpv6BroadcastAddress);
#if INET_CONFIG_ENABLE_IPV4
        BroadcastIpAddresses::GetIpv4Into(mIpv4BroadcastAddress);
#endif
    }
    virtual ~Server();

    /// Closes all currently open endpoints and resets the 'initialized' flag
    void Shutdown();

    void ShutdownEndpoints();

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

    Server & SetDelegate(ServerDelegate * d)
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
    CHIP_ERROR BroadcastImpl(chip::System::PacketBufferHandle && data, uint16_t port,
                             chip::Inet::InterfaceId interfaceId   = chip::Inet::InterfaceId::Null(),
                             chip::Inet::IPAddressType addressType = chip::Inet::IPAddressType::kAny);

    CHIP_ERROR SingleBroadcastImpl(chip::System::PacketBufferHandle && data, uint16_t port,
                                   chip::Inet::InterfaceId, chip::Inet::IPAddressType);

    static void OnUdpPacketReceived(chip::Inet::UDPEndPoint * endPoint, chip::System::PacketBufferHandle && buffer,
                                    const chip::Inet::IPPacketInfo * info);

    ServerDelegate * mDelegate = nullptr;

    chip::Inet::UDPEndPoint * mIpv6Endpoint = nullptr; // listen on port 5353
#if INET_CONFIG_ENABLE_IPV4
    chip::Inet::UDPEndPoint * mIpv4Endpoint = nullptr; // listen on port 5353
#endif

    // Broadcast IP addresses are cached to not require a string parse every time
    // Ideally we should be able to constexpr these
    chip::Inet::IPAddress mIpv6BroadcastAddress;
#if INET_CONFIG_ENABLE_IPV4
    chip::Inet::IPAddress mIpv4BroadcastAddress;
#endif

    struct BroadcastDestination {
        chip::Inet::InterfaceId interfaceId;
        chip::Inet::IPAddressType addressType;

        BroadcastDestination(chip::Inet::InterfaceId id, chip::Inet::IPAddressType type): interfaceId(id), addressType(type) {}
        BroadcastDestination() : interfaceId(chip::Inet::InterfaceId::Null()),
                                 addressType(chip::Inet::IPAddressType::kUnknown) {}

    };
    static constexpr size_t kMaxBroadcastDestinationCount = 20;
    size_t mBroadcastDestinationCount = 0;
    std::array<BroadcastDestination, kMaxBroadcastDestinationCount> mBroadcastDestination;

    bool mIsInitialized = false;
};

} // namespace Minimal
} // namespace mdns
