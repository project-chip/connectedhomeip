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

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include "DnsHeader.h"

namespace mdns {
namespace Minimal {
namespace {

struct BroadcastIpAddresses
{
    chip::Inet::IPAddress ipv6;
    chip::Inet::IPAddress ipv4;

    BroadcastIpAddresses()
    {
        chip::Inet::IPAddress::FromString("FF02::FB", ipv6);
        chip::Inet::IPAddress::FromString("224.0.0.251", ipv4);
    }
};

const BroadcastIpAddresses kBroadcastIp;

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
            mEndpoints[i].udp->Close();
            mEndpoints[i].udp = nullptr;
        }
    }
}

CHIP_ERROR ServerBase::Listen(ListenIterator * it, uint16_t port)
{
    Shutdown(); // ensure everything starts fresh

#if CONFIG_DEVICE_LAYER
    size_t endpointIndex                = 0;
    chip::Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;
    chip::Inet::IPAddressType addressType;

    ShutdownOnError autoShutdown(this);

    while (it->Next(&interfaceId, &addressType))
    {
        if (endpointIndex >= mEndpointCount)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        EndpointInfo * info = &mEndpoints[endpointIndex];
        info->addressType   = addressType;

        CHIP_ERROR err = chip::DeviceLayer::InetLayer.NewUDPEndPoint(&info->udp);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        info->udp->Bind(addressType, chip::Inet::IPAddress::Any, port, interfaceId);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        info->udp->AppState          = static_cast<void *>(this);
        info->udp->OnMessageReceived = OnUdpPacketReceived;

        err = info->udp->Listen();
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        endpointIndex++;
    }

    return autoShutdown.ReturnSuccess();
#else // #if CONFIG_DEVICE_LAYER
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR ServerBase::DirectSend(chip::System::PacketBuffer * data, const chip::Inet::IPAddress & addr, uint16_t port,
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

        return info->udp->SendTo(addr, port, data);
    }

    chip::System::PacketBuffer::Free(data);
    return CHIP_ERROR_NOT_CONNECTED;
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBuffer * data, uint16_t port)
{
    for (size_t i = 0; i < mEndpointCount; i++)
    {
        EndpointInfo * info = &mEndpoints[i];

        if (info->udp == nullptr)
        {
            continue;
        }

        // data may be sent over multiple packets. Keep the one ref active all the time
        data->AddRef();

        CHIP_ERROR err;

        if (info->addressType == chip::Inet::kIPAddressType_IPv6)
        {
            err = info->udp->SendTo(kBroadcastIp.ipv6, port, info->udp->GetBoundInterface(), data);
        }
        else if (info->addressType == chip::Inet::kIPAddressType_IPv4)
        {
            err = info->udp->SendTo(kBroadcastIp.ipv4, port, info->udp->GetBoundInterface(), data);
        }
        else
        {
            // remove extra ref and then also clear it
            chip::System::PacketBuffer::Free(data);
            chip::System::PacketBuffer::Free(data);
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (err == chip::System::MapErrorPOSIX(ENETUNREACH))
        {
            // Send attempted to an unreachable network. Generally should not happen if
            // interfaces are configured properly, however such a failure to broadcast
            // may not be critical either.
            ChipLogError(Discovery, "Attempt to mDNS broadcast to an unreachable destination.");
        }
        else if (err != CHIP_NO_ERROR)
        {
            chip::System::PacketBuffer::Free(data);
            return err;
        }
    }

    chip::System::PacketBuffer::Free(data);
    return CHIP_NO_ERROR;
}

void ServerBase::OnUdpPacketReceived(chip::Inet::IPEndPointBasis * endPoint, chip::System::PacketBufferHandle buffer,
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
