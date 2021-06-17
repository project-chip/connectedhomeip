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

#include <core/CHIPError.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <inet/UDPEndPoint.h>

#include <mdns/minimal/core/BytesRange.h>

namespace mdns {
namespace Minimal {

namespace BroadcastIpAddresses {

// Get standard mDNS Broadcast addresses

void GetIpv6Into(chip::Inet::IPAddress & dest);
void GetIpv4Into(chip::Inet::IPAddress & dest);

} // namespace BroadcastIpAddresses

/// Provides a list of intefaces to listen on.
///
/// When listening on IP, both IP address type (IPv4 or IPv6) and interface id
/// are important. In particular, when using link-local IP addresses, the actual
/// interface matters (e.g. FF02::FB will care over which IPv6 interface it is sent)
///
/// For MDNS in particular, you may want:
///  - IPv4 listen on INET_NULL_INTERFACEID
///  - IPv6 listen on every specific interface id available (except local loopback and other
///    not usable interfaces like docker)
class ListenIterator
{
public:
    virtual ~ListenIterator() {}

    // Get the next interface/address type to listen on
    virtual bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) = 0;
};

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
    struct EndpointInfo
    {
        chip::Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;
        chip::Inet::IPAddressType addressType;
        chip::Inet::UDPEndPoint * udp = nullptr;
    };

    ServerBase(EndpointInfo * endpointStorage, size_t kStorageSize) : mEndpoints(endpointStorage), mEndpointCount(kStorageSize)
    {
        for (size_t i = 0; i < mEndpointCount; i++)
        {
            mEndpoints[i].udp = nullptr;
        }

        BroadcastIpAddresses::GetIpv6Into(mIpv6BroadcastAddress);

#if INET_CONFIG_ENABLE_IPV4
        BroadcastIpAddresses::GetIpv4Into(mIpv4BroadcastAddress);
#endif
    }
    virtual ~ServerBase();

    /// Closes all currently open endpoints
    void Shutdown();

    /// Listen on the given interfaces/address types.
    ///
    /// Since mDNS uses link-local addresses, one generally wants to listen on all
    /// non-loopback interfaces.
    CHIP_ERROR Listen(chip::Inet::InetLayer * inetLayer, ListenIterator * it, uint16_t port);

    /// Send the specified packet to a destination IP address over the specified address
    virtual CHIP_ERROR DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                                  chip::Inet::InterfaceId interface);

    /// Send a specific packet broadcast to all interfaces
    virtual CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port);

    /// Send a specific packet broadcast to a specific interface
    virtual CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface);

    ServerBase & SetDelegate(ServerDelegate * d)
    {
        mDelegate = d;
        return *this;
    }

    /// How many endpoints are availabe to be used by the server.
    size_t GetEndpointCount() const { return mEndpointCount; }

    /// Get the endpoints that are used by this server
    ///
    /// Entries with non-null UDP are considered usable.
    const EndpointInfo * GetEndpoints() const { return mEndpoints; }

    /// A server is considered listening if any UDP endpoint is active.
    ///
    /// This is expected to return false after any Shutdown() and will
    /// return true IFF lListen was called and the listen iterator successfully
    /// found a valid listening interface.
    bool IsListening() const;

private:
    static void OnUdpPacketReceived(chip::Inet::IPEndPointBasis * endPoint, chip::System::PacketBufferHandle && buffer,
                                    const chip::Inet::IPPacketInfo * info);

    EndpointInfo * mEndpoints;   // possible endpoints, to listen on multiple interfaces
    const size_t mEndpointCount; // how many endpoints are allocated
    ServerDelegate * mDelegate = nullptr;

    // Broadcast IP addresses are cached to not require a string parse every time
    // Ideally we should be able to constexpr these
    chip::Inet::IPAddress mIpv6BroadcastAddress;
#if INET_CONFIG_ENABLE_IPV4
    chip::Inet::IPAddress mIpv4BroadcastAddress;
#endif
};

template <size_t kCount>
class Server : public ServerBase
{
public:
    Server() : ServerBase(mAllocatedStorage, kCount) {}
    ~Server() {}

private:
    EndpointInfo mAllocatedStorage[kCount];
};

} // namespace Minimal
} // namespace mdns
