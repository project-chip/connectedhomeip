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

#include <lib/dnssd/wire/DnsHeader.h>
#include <platform/CHIPDeviceLayer.h>

namespace mdns {
namespace Minimal {
using namespace chip::Dnssd;

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

/**
 * Extracts the Listening UDP Endpoint from an underlying ServerBase::EndpointInfo
 */
class ListenSocketPickerDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    chip::Inet::UDPEndPointHandle Accept(ServerBase::EndpointInfo * info) override { return info->mListenUdp; }
};

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT

/**
 * Extracts the Querying UDP Endpoint from an underlying ServerBase::EndpointInfo
 */
class QuerySocketPickerDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    chip::Inet::UDPEndPointHandle Accept(ServerBase::EndpointInfo * info) override { return info->mUnicastQueryUdp; }
};

#else

using QuerySocketPickerDelegate = ListenSocketPickerDelegate;

#endif

/**
 * Validates that an endpoint belongs to a specific interface/ip address type before forwarding the
 * endpoint accept logic to another BroadcastSendDelegate.
 *
 * Usage like:
 *
 * SomeDelegate *child = ....;
 * InterfaceTypeFilterDelegate filter(interfaceId, IPAddressType::IPv6, child);
 *
 * UDPEndPoint *udp = filter.Accept(endpointInfo);
 */
class InterfaceTypeFilterDelegate : public ServerBase::BroadcastSendDelegate
{
public:
    InterfaceTypeFilterDelegate(chip::Inet::InterfaceId interface, chip::Inet::IPAddressType type,
                                ServerBase::BroadcastSendDelegate * child) :
        mInterface(interface),
        mAddressType(type), mChild(child)
    {}

    chip::Inet::UDPEndPointHandle Accept(ServerBase::EndpointInfo * info) override
    {
        if ((info->mInterfaceId != mInterface) && (info->mInterfaceId != chip::Inet::InterfaceId::Null()))
        {
            return nullptr;
        }

        if ((mAddressType != chip::Inet::IPAddressType::kAny) && (info->mAddressType != mAddressType))
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
chip::Inet::IPAddress Get(chip::Inet::IPAddressType addressType)
{
    chip::Inet::IPAddress address;
#if INET_CONFIG_ENABLE_IPV4
    if (addressType == chip::Inet::IPAddressType::kIPv4)
    {
        VerifyOrDie(chip::Inet::IPAddress::FromString("224.0.0.251", address));
    }
    else
#endif
    {
        VerifyOrDie(chip::Inet::IPAddress::FromString("FF02::FB", address));
    }
    return address;
}

} // namespace BroadcastIpAddresses

namespace {

#if CHIP_ERROR_LOGGING
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
#endif

} // namespace

ServerBase::~ServerBase()
{
    Shutdown();
}

void ServerBase::Shutdown()
{
    ShutdownEndpoints();
    mIsInitialized = false;
}

void ServerBase::ShutdownEndpoints()
{
    mEndpoints.ReleaseAll();
}

void ServerBase::ShutdownEndpoint(EndpointInfo & aEndpoint)
{
    mEndpoints.ReleaseObject(&aEndpoint);
}

bool ServerBase::IsListening() const
{
    bool listening = false;
    mEndpoints.ForEachActiveObject([&](auto * endpoint) {
        if (endpoint->mListenUdp)
        {
            listening = true;
            return chip::Loop::Break;
        }
        return chip::Loop::Continue;
    });
    return listening;
}

CHIP_ERROR ServerBase::Listen(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager, ListenIterator * it,
                              uint16_t port)
{
    ShutdownEndpoints(); // ensure everything starts fresh

    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::Null();
    chip::Inet::IPAddressType addressType;

    ShutdownOnError autoShutdown(this);

    while (it->Next(&interfaceId, &addressType))
    {
        // GetInterfaceName leaves the buffer untouched when it fails, and it fails for exactly the
        // interface this loop is about to have trouble with: one that has gone away since the
        // iterator listed it. Name it the way BroadcastImpl below does rather than logging
        // whatever the stack happened to hold.
        char interfaceName[chip::Inet::InterfaceId::kMaxIfNameLength];
        if (interfaceId.GetInterfaceName(interfaceName, sizeof(interfaceName)) != CHIP_NO_ERROR)
        {
            strcpy(interfaceName, "???");
        }

        // Running out of endpoints is a global resource problem rather than something about this
        // interface, and skipping interfaces would not make it any better, so it stays fatal.
        chip::Inet::UDPEndPointHandle listenUdp;
        ReturnErrorOnFailure(udpEndPointManager->NewEndPoint(listenUdp));

        // Binding and listening, on the other hand, are per-interface: an interface the iterator
        // reports as usable can still fail here, for instance while it is being reconfigured.
        // Failing the whole call would leave the server with no endpoints at all, so skip that
        // interface and keep the ones that do work.
        CHIP_ERROR err = listenUdp->Bind(addressType, chip::Inet::IPAddress::Any, port, interfaceId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "MDNS failed to bind to %s for address type %s: %" CHIP_ERROR_FORMAT, interfaceName,
                         AddressTypeStr(addressType), err.Format());
            continue;
        }

        err = listenUdp->Listen(OnUdpPacketReceived, nullptr /*OnReceiveError*/, this);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "MDNS failed to listen on %s for address type %s: %" CHIP_ERROR_FORMAT, interfaceName,
                         AddressTypeStr(addressType), err.Format());
            continue;
        }

        err = listenUdp->JoinMulticastGroup(interfaceId, BroadcastIpAddresses::Get(addressType));

        if (err != CHIP_NO_ERROR)
        {
            // Log only as non-fatal error. Failure to join will mean we reply to unicast queries only.
            ChipLogError(DeviceLayer, "MDNS failed to join multicast group on %s for address type %s: %" CHIP_ERROR_FORMAT,
                         interfaceName, AddressTypeStr(addressType), err.Format());

            listenUdp.Release();
        }

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
        // Separate UDP endpoint for unicast queries, bound to 0 (i.e. pick random ephemeral port)
        //   - helps in not having conflicts on port 5353, will receive unicast replies directly
        //   - has a *DRAWBACK* of unicast queries being considered LEGACY by mdns since they do
        //     not originate from 5353 and the answers will include a query section.
        chip::Inet::UDPEndPointHandle unicastQueryUdp;
        err = udpEndPointManager->NewEndPoint(unicastQueryUdp);
        if (err == CHIP_NO_ERROR)
        {
            err = unicastQueryUdp->Bind(addressType, chip::Inet::IPAddress::Any, 0, interfaceId);
        }
        if (err == CHIP_NO_ERROR)
        {
            err = unicastQueryUdp->Listen(OnUdpPacketReceived, nullptr /*OnReceiveError*/, this);
        }
        if (err != CHIP_NO_ERROR)
        {
            // The unicast query port is an optimisation on top of the multicast endpoint, so
            // giving up here would throw away an endpoint that already works. Answer legacy
            // unicast queries from port 5353 instead.
            ChipLogError(DeviceLayer, "MDNS failed to open a unicast query port on %s for address type %s: %" CHIP_ERROR_FORMAT,
                         interfaceName, AddressTypeStr(addressType), err.Format());
            unicastQueryUdp.Release();
        }
#endif

#if CHIP_MINMDNS_USE_EPHEMERAL_UNICAST_PORT
        bool interfaceIsListening = listenUdp || unicastQueryUdp;
        if (interfaceIsListening)
        {
            // If allocation fails, the rref will not be consumed, so that the endpoint will also be freed correctly.
            // An interface whose endpoint could not be registered is not listening.
            interfaceIsListening =
                mEndpoints.CreateObject(interfaceId, addressType, std::move(listenUdp), std::move(unicastQueryUdp)) != nullptr;
        }
#else
        bool interfaceIsListening = static_cast<bool>(listenUdp);
        if (interfaceIsListening)
        {
            // If allocation fails, the rref will not be consumed, so that the endpoint will also be freed correctly.
            // An interface whose endpoint could not be registered is not listening.
            interfaceIsListening = mEndpoints.CreateObject(interfaceId, addressType, std::move(listenUdp)) != nullptr;
        }
#endif

        // If at least one IPv6 interface is used by the mDNS server, notify the application that DNS-SD is ready.
        // Only once an interface actually has an endpoint: now that a failure here is survivable, reaching this
        // point is no longer proof that one was created.
        if (interfaceIsListening && !mIsInitialized && addressType == chip::Inet::IPAddressType::kIPv6)
        {
#if !CHIP_DEVICE_LAYER_NONE
            chip::DeviceLayer::ChipDeviceEvent event{};
            event.Type = chip::DeviceLayer::DeviceEventType::kDnssdInitialized;
            chip::DeviceLayer::PlatformMgr().PostEventOrDie(&event);
#endif
            mIsInitialized = true;
        }
    }

    return autoShutdown.ReturnSuccess();
}

CHIP_ERROR ServerBase::DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                                  chip::Inet::InterfaceId interface)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_CONNECTED;
    mEndpoints.ForEachActiveObject([&](auto * info) {
        if (info->mListenUdp.IsNull())
        {
            return chip::Loop::Continue;
        }

        if (info->mAddressType != addr.Type())
        {
            return chip::Loop::Continue;
        }

        chip::Inet::InterfaceId boundIf = info->mListenUdp->GetBoundInterface();

        if ((boundIf.IsPresent()) && (boundIf != interface))
        {
            return chip::Loop::Continue;
        }

        err = info->mListenUdp->SendTo(addr, port, std::move(data));
        return chip::Loop::Break;
    });

    return err;
}

CHIP_ERROR ServerBase::BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port)
{
    QuerySocketPickerDelegate socketPicker;
    return BroadcastImpl(std::move(data), port, &socketPicker);
}

CHIP_ERROR ServerBase::BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port,
                                             chip::Inet::InterfaceId interface, chip::Inet::IPAddressType addressType)
{
    QuerySocketPickerDelegate socketPicker;
    InterfaceTypeFilterDelegate filter(interface, addressType, &socketPicker);

    return BroadcastImpl(std::move(data), port, &filter);
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface,
                                     chip::Inet::IPAddressType addressType)
{
    ListenSocketPickerDelegate socketPicker;
    InterfaceTypeFilterDelegate filter(interface, addressType, &socketPicker);

    return BroadcastImpl(std::move(data), port, &filter);
}

CHIP_ERROR ServerBase::BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port)
{
    ListenSocketPickerDelegate socketPicker;
    return BroadcastImpl(std::move(data), port, &socketPicker);
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

    unsigned successes   = 0;
    unsigned failures    = 0;
    CHIP_ERROR lastError = CHIP_ERROR_NO_ENDPOINT;

    if (chip::Loop::Break == mEndpoints.ForEachActiveObject([&](auto * info) {
            auto udp = delegate->Accept(info);

            if (udp.IsNull())
            {
                return chip::Loop::Continue;
            }

            CHIP_ERROR err = CHIP_NO_ERROR;

            /// The same packet needs to be sent over potentially multiple interfaces.
            /// LWIP does not like having a pbuf sent over serparate interfaces, hence we create a copy
            /// for sending via `CloneData`
            ///
            /// TODO: this wastes one copy of the data and that could be optimized away
            chip::System::PacketBufferHandle tempBuf = data.CloneData();
            if (tempBuf.IsNull())
            {
                // Not enough memory available to clone pbuf
                err = CHIP_ERROR_NO_MEMORY;
            }
            else if (info->mAddressType == chip::Inet::IPAddressType::kIPv6)
            {
                err = udp->SendTo(mIpv6BroadcastAddress, port, std::move(tempBuf), udp->GetBoundInterface());
            }
#if INET_CONFIG_ENABLE_IPV4
            else if (info->mAddressType == chip::Inet::IPAddressType::kIPv4)
            {
                err = udp->SendTo(mIpv4BroadcastAddress, port, std::move(tempBuf), udp->GetBoundInterface());
            }
#endif
            else
            {
                // This is a general error of internal consistency: every address has a known type. Fail completely otherwise.
                lastError = CHIP_ERROR_INCORRECT_STATE;
                return chip::Loop::Break;
            }

            if (err == CHIP_NO_ERROR)
            {
                successes++;
            }
            else
            {
                failures++;
                lastError = err;
#if CHIP_DETAIL_LOGGING
                char ifaceName[chip::Inet::InterfaceId::kMaxIfNameLength];
                err = info->mInterfaceId.GetInterfaceName(ifaceName, sizeof(ifaceName));
                if (err != CHIP_NO_ERROR)
                    strcpy(ifaceName, "???");
                ChipLogDetail(Discovery, "Warning: Attempt to mDNS broadcast failed on %s:  %s", ifaceName, lastError.AsString());
#endif
            }
            return chip::Loop::Continue;
        }))
    {
        return lastError;
    }

    if (failures != 0)
    {
        // if we had failures, log if the final status was success or failure, to make log reading
        // easier. Some mDNS failures may be expected (e.g. for interfaces unavailable)
        if (successes != 0)
        {
            ChipLogDetail(Discovery, "mDNS broadcast had only partial success: %u successes and %u failures.", successes, failures);
        }
        else
        {
            ChipLogProgress(Discovery, "mDNS broadcast full failed in %u separate send attempts.", failures);
        }
    }

    if (!successes)
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

    chip::Dnssd::BytesRange data(buffer->Start(), buffer->Start() + buffer->DataLength());
    if (data.Size() < HeaderRef::kSizeBytes)
    {
        ChipLogError(Discovery, "Packet too small for mDNS data: %d bytes", static_cast<int>(data.Size()));
        return;
    }

    if (HeaderRef(const_cast<uint8_t *>(data.Start())).GetFlags().IsQuery())
    {
        // Only consider queries that are received on the same interface we are listening on.
        // Without this, queries show up on all addresses on all interfaces, resulting
        // in more replies than one would expect.
        if (endPoint->GetBoundInterface() == info->Interface)
        {
            srv->mDelegate->OnQuery(data, info);
        }
    }
    else
    {
        srv->mDelegate->OnResponse(data, info);
    }
}

} // namespace Minimal
} // namespace mdns
