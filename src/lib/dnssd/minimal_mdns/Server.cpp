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

#include "Server.h"

#include <errno.h>
#include <utility>

#include <lib/dnssd/minimal_mdns/core/DnsHeader.h>

namespace mdns {
namespace Minimal {
namespace {

// Port number to use t o signal 'select random unused port' during UDP sockeg
// binding.
constexpr uint16_t kPickRandomBindPort = 0;

class ShutdownOnError
{
public:
    ShutdownOnError(ServerBase * s) : mServer(s) {}
    ~ShutdownOnError()
    {
        if (mServer != nullptr)
        {
            mServer->Shutdown();
        }
    }

    CHIP_ERROR ReturnSuccess()
    {
        mServer = nullptr;
        return CHIP_NO_ERROR;
    }

private:
    ServerBase * mServer;
};

/**
 * Extracts the Listening UDP Endpoint from an underlying ServerBase::EndpointInfo
 */
class ListenSocketDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    chip::Inet::UDPEndPoint * Accept(ServerBase::EndpointInfo * info) override { return info->listen_udp; }
};

/**
 * Extracts the Querying UDP Endpoint from an underlying ServerBase::EndpointInfo
 */
class QuerySocketDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    chip::Inet::UDPEndPoint * Accept(ServerBase::EndpointInfo * info) override { return info->unicast_query_udp; }
};

/**
 * Validates that an endpoint belongs to a specific interface/ip address type before forwarding the
 * endpoint accept logic to another BroadcastSendDelegate.
 *
 * Usage like:
 *
 * SomeDelegate *child = ....;
 * InterfaceTypeValidateDelegate validator(interfaceId, IPAddressType::IPv6, child);
 *
 * UDPEndPoint *udp = validator.Accept(endpointInfo);
 */
class InterfaceTypeValidateDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    InterfaceTypeValidateDelegate(chip::Inet::InterfaceId interface, chip::Inet::IPAddressType type,
                                  ServerBase::BroadcastSendDelegate * child) :
        mInterface(interface),
        mAddressType(type), mChild(child)
    {}

    chip::Inet::UDPEndPoint * Accept(ServerBase::EndpointInfo * info) override
    {
        if ((info->interfaceId != mInterface) && (info->interfaceId != chip::Inet::InterfaceId::Null()))
        {
            return nullptr;
        }

        if ((mAddressType != chip::Inet::IPAddressType::kAny) && (info->addressType != mAddressType))
        {
            return nullptr;
        }

        return mChild->Accept(info);
    }

private:
    chip::Inet::InterfaceId mInterface;
    chip::Inet::IPAddressType mAddressType;
    ServerBase::BroadcastSendDelegate * mChild = nullptr;
};

} // namespace

namespace BroadcastIpAddresses {

// Get standard mDNS Broadcast addresses

void GetIpv6Into(chip::Inet::IPAddress & dest)
{
    if (!chip::Inet::IPAddress::FromString("FF02::FB", dest))
    {
        ChipLogError(Discovery, "Failed to parse standard IPv6 broadcast address");
    }
}

void GetIpv4Into(chip::Inet::IPAddress & dest)
{
    if (!chip::Inet::IPAddress::FromString("224.0.0.251", dest))
    {
        ChipLogError(Discovery, "Failed to parse standard IPv4 broadcast address");
    }
}

} // namespace BroadcastIpAddresses

namespace {

CHIP_ERROR JoinMulticastGroup(chip::Inet::InterfaceId interfaceId, chip::Inet::UDPEndPoint * endpoint,
                              chip::Inet::IPAddressType addressType)
{

    chip::Inet::IPAddress address;

    if (addressType == chip::Inet::IPAddressType::kIPv6)
    {
        BroadcastIpAddresses::GetIpv6Into(address);
#if INET_CONFIG_ENABLE_IPV4
    }
    else if (addressType == chip::Inet::IPAddressType::kIPv4)
    {
        BroadcastIpAddresses::GetIpv4Into(address);
#endif // INET_CONFIG_ENABLE_IPV4
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return endpoint->JoinMulticastGroup(interfaceId, address);
}

const char * AddressTypeStr(chip::Inet::IPAddressType addressType)
{
    switch (addressType)
    {
    case chip::Inet::IPAddressType::kIPv6:
        return "IPv6";
#if INET_CONFIG_ENABLE_IPV4
    case chip::Inet::IPAddressType::kIPv4:
        return "IPv4";
#endif // INET_CONFIG_ENABLE_IPV4
    default:
        return "UNKNOWN";
    }
}

void ShutdownEndpoint(mdns::Minimal::ServerBase::EndpointInfo & aEndpoint)
{
    if (aEndpoint.listen_udp != nullptr)
    {
        aEndpoint.listen_udp->Free();
        aEndpoint.listen_udp = nullptr;
    }

    if (aEndpoint.unicast_query_udp != nullptr)
    {
        aEndpoint.unicast_query_udp->Free();
        aEndpoint.unicast_query_udp = nullptr;
    }
}

} // namespace

ServerBase::~ServerBase()
{
    Shutdown();
}

void ServerBase::Shutdown()
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        ShutdownEndpoint(mEndpoints[i]);
    }
}

bool ServerBase::IsListening() const
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        if (mEndpoints[i].listen_udp != nullptr)
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR ServerBase::Listen(chip::Inet::InetLayer * inetLayer, ListenIterator * it, uint16_t port)
{
    Shutdown(); // ensure everything starts fresh

    size_t endpointIndex                = 0;
    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::Null();
    chip::Inet::IPAddressType addressType;

    ShutdownOnError autoShutdown(this);

    while (it->Next(&interfaceId, &addressType))
    {
        ReturnErrorCodeIf(endpointIndex >= mEndpointCount, CHIP_ERROR_NO_MEMORY);

        EndpointInfo * info = &mEndpoints[endpointIndex];
        info->addressType   = addressType;
        info->interfaceId   = interfaceId;

        ReturnErrorOnFailure(inetLayer->NewUDPEndPoint(&info->listen_udp));

        ReturnErrorOnFailure(info->listen_udp->Bind(addressType, chip::Inet::IPAddress::Any, port, interfaceId));

        ReturnErrorOnFailure(info->listen_udp->Listen(OnUdpPacketReceived, nullptr /*OnReceiveError*/, this));

        CHIP_ERROR err = JoinMulticastGroup(interfaceId, info->listen_udp, addressType);
        if (err != CHIP_NO_ERROR)
        {
            char interfaceName[chip::Inet::InterfaceId::kMaxIfNameLength];
            interfaceId.GetInterfaceName(interfaceName, sizeof(interfaceName));

            // Log only as non-fatal error. Failure to join will mean we reply to unicast queries only.
            ChipLogError(DeviceLayer, "MDNS failed to join multicast group on %s for address type %s: %s", interfaceName,
                         AddressTypeStr(addressType), chip::ErrorStr(err));
            ShutdownEndpoint(mEndpoints[endpointIndex]);
        }
        else
        {
            endpointIndex++;
        }

        // Separate UDP endpoint for unicast queries:
        //   - helps in not having conflicts on port 5353, will receive unicast replies directly
        //   - has a *DRAWBACK* of unicast queries being considered LEGACY by mdns since they do
        //     not originate from 5353 and the answers will include a query section.
        ReturnErrorOnFailure(inetLayer->NewUDPEndPoint(&info->unicast_query_udp));
        ReturnErrorOnFailure(
            info->unicast_query_udp->Bind(addressType, chip::Inet::IPAddress::Any, kPickRandomBindPort, interfaceId));
        ReturnErrorOnFailure(info->unicast_query_udp->Listen(OnUdpPacketReceived, nullptr /*OnReceiveError*/, this));
    }

    return autoShutdown.ReturnSuccess();
}

CHIP_ERROR ServerBase::DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                                  chip::Inet::InterfaceId interface)
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        EndpointInfo * info = &mEndpoints[i];
        if (info->listen_udp == nullptr)
        {
            continue;
        }

        if (info->addressType != addr.Type())
        {
            continue;
        }

        chip::Inet::InterfaceId boundIf = info->listen_udp->GetBoundInterface();

        if ((boundIf.IsPresent()) && (boundIf != interface))
        {
            continue;
        }

        return info->listen_udp->SendTo(addr, port, std::move(data));
    }

    return CHIP_ERROR_NOT_CONNECTED;
}

CHIP_ERROR ServerBase::BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port)
{
    QuerySocketDelegate delegate;
    return BroadcastImpl(std::move(data), port, &delegate);
}

CHIP_ERROR ServerBase::BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port,
                                             chip::Inet::InterfaceId interface, chip::Inet::IPAddressType addressType)
{
    QuerySocketDelegate delegate;
    InterfaceTypeValidateDelegate validator(interface, addressType, &delegate);

    return BroadcastImpl(std::move(data), port, &validator);
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface,
                                     chip::Inet::IPAddressType addressType)
{
    ListenSocketDelegate delegate;
    InterfaceTypeValidateDelegate validator(interface, addressType, &delegate);

    return BroadcastImpl(std::move(data), port, &validator);
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port)
{
    ListenSocketDelegate delegate;
    return BroadcastImpl(std::move(data), port, &delegate);
}

CHIP_ERROR ServerBase::BroadcastImpl(chip::System::PacketBufferHandle && data, uint16_t port, BroadcastSendDelegate * delegate)
{
    // Broadcast requires sending data multiple times, each of which may error
    // out, yet broadcast only has a single error code.
    //
    // The general logic of error handling is:
    //   - if no send done at all, return error
    //   - if at least one broadcast succeeds, assume success overall
    //   + some internal consistency validations for state error.

    bool hadSuccesfulSend = false;
    CHIP_ERROR lastError  = CHIP_ERROR_NO_ENDPOINT;

    for (size_t i = 0; i < mEndpointCount; i++)
    {
        EndpointInfo * info           = &mEndpoints[i];
        chip::Inet::UDPEndPoint * udp = delegate->Accept(info);

        if (udp == nullptr)
        {
            continue;
        }

        CHIP_ERROR err;

        /// The same packet needs to be sent over potentially multiple interfaces.
        /// LWIP does not like having a pbuf sent over serparate interfaces, hence we create a copy
        /// for sending via `CloneData`
        ///
        /// TODO: this wastes one copy of the data and that could be optimized away
        if (info->addressType == chip::Inet::IPAddressType::kIPv6)
        {
            err = udp->SendTo(mIpv6BroadcastAddress, port, data.CloneData(), udp->GetBoundInterface());
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (info->addressType == chip::Inet::IPAddressType::kIPv4)
        {
            err = udp->SendTo(mIpv4BroadcastAddress, port, data.CloneData(), udp->GetBoundInterface());
        }
#endif
        else
        {
            // This is a general error of internal consistency: every address has a known type
            // Fail completely otherwise.
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (err == CHIP_NO_ERROR)
        {
            hadSuccesfulSend = true;
        }
        else
        {
            ChipLogError(Discovery, "Attempt to mDNS broadcast failed:  %s", chip::ErrorStr(err));
            lastError = err;
        }
    }

    if (!hadSuccesfulSend)
    {
        return lastError;
    }

    return CHIP_NO_ERROR;
}

void ServerBase::OnUdpPacketReceived(chip::Inet::UDPEndPoint * endPoint, chip::System::PacketBufferHandle && buffer,
                                     const chip::Inet::IPPacketInfo * info)
{
    ServerBase * srv = static_cast<ServerBase *>(endPoint->mAppState);
    if (!srv->mDelegate)
    {
        return;
    }

    mdns::Minimal::BytesRange data(buffer->Start(), buffer->Start() + buffer->DataLength());
    if (data.Size() < HeaderRef::kSizeBytes)
    {
        ChipLogError(Discovery, "Packet to small for mDNS data: %d bytes", static_cast<int>(data.Size()));
        return;
    }

    if (HeaderRef(const_cast<uint8_t *>(data.Start())).GetFlags().IsQuery())
    {
        srv->mDelegate->OnQuery(data, info);
    }
    else
    {
        srv->mDelegate->OnResponse(data, info);
    }
}

} // namespace Minimal
} // namespace mdns
