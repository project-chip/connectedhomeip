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

#include <lib/mdns/minimal/core/DnsHeader.h>

namespace mdns {
namespace Minimal {
namespace {

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

    if (addressType == chip::Inet::IPAddressType::kIPAddressType_IPv6)
    {
        BroadcastIpAddresses::GetIpv6Into(address);
#if INET_CONFIG_ENABLE_IPV4
    }
    else if (addressType == chip::Inet::IPAddressType::kIPAddressType_IPv4)
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
    case chip::Inet::IPAddressType::kIPAddressType_IPv6:
        return "IPv6";
#if INET_CONFIG_ENABLE_IPV4
    case chip::Inet::IPAddressType::kIPAddressType_IPv4:
        return "IPv4";
#endif // INET_CONFIG_ENABLE_IPV4
    default:
        return "UNKNOWN";
    }
}

void ShutdownEndpoint(mdns::Minimal::ServerBase::EndpointInfo & aEndpoint)
{
    aEndpoint.udp->Free();
    aEndpoint.udp = nullptr;
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
        if (mEndpoints[i].udp != nullptr)
        {
            ShutdownEndpoint(mEndpoints[i]);
        }
    }
}

bool ServerBase::IsListening() const
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        if (mEndpoints[i].udp != nullptr)
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
    chip::Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;
    chip::Inet::IPAddressType addressType;

    ShutdownOnError autoShutdown(this);

    while (it->Next(&interfaceId, &addressType))
    {
        ReturnErrorCodeIf(endpointIndex >= mEndpointCount, CHIP_ERROR_NO_MEMORY);

        EndpointInfo * info = &mEndpoints[endpointIndex];
        info->addressType   = addressType;
        info->interfaceId   = interfaceId;

        ReturnErrorOnFailure(inetLayer->NewUDPEndPoint(&info->udp));

        ReturnErrorOnFailure(info->udp->Bind(addressType, chip::Inet::IPAddress::Any, port, interfaceId));

        ReturnErrorOnFailure(info->udp->Listen(OnUdpPacketReceived, nullptr /*OnReceiveError*/, this));

        CHIP_ERROR err = JoinMulticastGroup(interfaceId, info->udp, addressType);
        if (err != CHIP_NO_ERROR)
        {
            char interfaceName[chip::Inet::InterfaceIterator::kMaxIfNameLength];
            chip::Inet::GetInterfaceName(interfaceId, interfaceName, sizeof(interfaceName));

            // Log only as non-fatal error. Failure to join will mean we reply to unicast queries only.
            ChipLogError(DeviceLayer, "MDNS failed to join multicast group on %s for address type %s: %s", interfaceName,
                         AddressTypeStr(addressType), chip::ErrorStr(err));
            ShutdownEndpoint(mEndpoints[endpointIndex]);
        }
        else
        {
            endpointIndex++;
        }
    }

    return autoShutdown.ReturnSuccess();
}

CHIP_ERROR ServerBase::DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                                  chip::Inet::InterfaceId interface)
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        EndpointInfo * info = &mEndpoints[i];
        if (info->udp == nullptr)
        {
            continue;
        }

        if (info->addressType != addr.Type())
        {
            continue;
        }

        chip::Inet::InterfaceId boundIf = info->udp->GetBoundInterface();

        if ((boundIf != INET_NULL_INTERFACEID) && (boundIf != interface))
        {
            continue;
        }

        return info->udp->SendTo(addr, port, std::move(data));
    }

    return CHIP_ERROR_NOT_CONNECTED;
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface)
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        EndpointInfo * info = &mEndpoints[i];

        if (info->udp == nullptr)
        {
            continue;
        }

        if ((info->udp->GetBoundInterface() != interface) && (info->udp->GetBoundInterface() != INET_NULL_INTERFACEID))
        {
            continue;
        }

        CHIP_ERROR err;

        /// The same packet needs to be sent over potentially multiple interfaces.
        /// LWIP does not like having a pbuf sent over serparate interfaces, hence we create a copy
        /// TODO: this wastes one copy of the data and that could be optimized away
        chip::System::PacketBufferHandle copy = data.CloneData();

        if (info->addressType == chip::Inet::kIPAddressType_IPv6)
        {
            err = info->udp->SendTo(mIpv6BroadcastAddress, port, info->udp->GetBoundInterface(), std::move(copy));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (info->addressType == chip::Inet::kIPAddressType_IPv4)
        {
            err = info->udp->SendTo(mIpv4BroadcastAddress, port, info->udp->GetBoundInterface(), std::move(copy));
        }
#endif
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port)
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
        EndpointInfo * info = &mEndpoints[i];

        if (info->udp == nullptr)
        {
            continue;
        }

        CHIP_ERROR err;

        /// The same packet needs to be sent over potentially multiple interfaces.
        /// LWIP does not like having a pbuf sent over serparate interfaces, hence we create a copy
        /// TODO: this wastes one copy of the data and that could be optimized away
        chip::System::PacketBufferHandle copy = data.CloneData();

        if (info->addressType == chip::Inet::kIPAddressType_IPv6)
        {
            err = info->udp->SendTo(mIpv6BroadcastAddress, port, info->udp->GetBoundInterface(), std::move(copy));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (info->addressType == chip::Inet::kIPAddressType_IPv4)
        {
            err = info->udp->SendTo(mIpv4BroadcastAddress, port, info->udp->GetBoundInterface(), std::move(copy));
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
            ChipLogProgress(Discovery, "mDNS broadcast success");
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

void ServerBase::OnUdpPacketReceived(chip::Inet::IPEndPointBasis * endPoint, chip::System::PacketBufferHandle && buffer,
                                     const chip::Inet::IPPacketInfo * info)
{
    ServerBase * srv = static_cast<ServerBase *>(endPoint->AppState);
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
