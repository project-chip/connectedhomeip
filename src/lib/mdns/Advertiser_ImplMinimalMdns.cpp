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

#include "Advertiser.h"

#include <inttypes.h>
#include <stdio.h>

#include "MinimalMdnsServer.h"
#include "ServiceNaming.h"

#include <mdns/Advertiser_ImplMinimalMdnsAllocator.h>
#include <mdns/minimal/ResponseSender.h>
#include <mdns/minimal/Server.h>
#include <mdns/minimal/core/FlatAllocatedQName.h>
#include <mdns/minimal/responders/IP.h>
#include <mdns/minimal/responders/Ptr.h>
#include <mdns/minimal/responders/QueryResponder.h>
#include <mdns/minimal/responders/Srv.h>
#include <mdns/minimal/responders/Txt.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>
#include <support/StringBuilder.h>

// Enable detailed mDNS logging for received queries
#undef DETAIL_LOGGING
// #define DETAIL_LOGGING

namespace chip {
namespace Mdns {
namespace {

using namespace mdns::Minimal;

#ifdef DETAIL_LOGGING
const char * ToString(QClass qClass)
{
    switch (qClass)
    {
    case QClass::IN:
        return "IN";
    default:
        return "???";
    }
}

const char * ToString(QType qType)
{
    switch (qType)
    {
    case QType::ANY:
        return "ANY";
    case QType::A:
        return "A";
    case QType::AAAA:
        return "AAAA";
    case QType::TXT:
        return "TXT";
    case QType::SRV:
        return "SRV";
    case QType::PTR:
        return "PTR";
    default:
        return "???";
    }
}

void LogQuery(const QueryData & data)
{
    StringBuilder<128> logString;

    logString.Add("QUERY ").Add(ToString(data.GetClass())).Add("/").Add(ToString(data.GetType())).Add(": ");

    SerializedQNameIterator name = data.GetName();
    while (name.Next())
    {
        logString.Add(name.Value()).Add(".");
    }

    ChipLogDetail(Discovery, "%s", logString.c_str());
}
#else
void LogQuery(const QueryData & data) {}
#endif

class AdvertiserMinMdns : public ServiceAdvertiser,
                          public MdnsPacketDelegate, // receive query packets
                          public ParserDelegate      // parses queries
{
public:
    AdvertiserMinMdns() : mResponseSender(&GlobalMinimalMdnsServer::Server(), mQueryResponderAllocator.GetQueryResponder())
    {
        GlobalMinimalMdnsServer::Instance().SetQueryDelegate(this);
    }
    ~AdvertiserMinMdns() {}

    // Service advertiser
    CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint16_t port) override;
    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override;
    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override;
    CHIP_ERROR StopPublishDevice() override;

    // MdnsPacketDelegate
    void OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override;

    // ParserDelegate
    void OnHeader(ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(ResourceType type, const ResourceData & data) override {}
    void OnQuery(const QueryData & data) override;

private:
    /// Advertise available records configured within the server
    ///
    /// Usable as boot-time advertisement of available SRV records.
    void AdvertiseRecords();

    /// Determine if advertisement on the specified interface/address is ok given the
    /// interfaces on which the mDNS server is listening
    bool ShouldAdvertiseOn(const chip::Inet::InterfaceId id, const chip::Inet::IPAddress & addr);

    FullQName GetCommisioningTextEntries(const CommissionAdvertisingParameters & params);

    static constexpr size_t kMaxRecords = 32;
    QueryResponderAllocator<kMaxRecords> mQueryResponderAllocator;

    ResponseSender mResponseSender;

    // current request handling
    const chip::Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint32_t mMessageId                             = 0;

    const char * mEmptyTextEntries[1] = {
        "=",
    };
};

void AdvertiserMinMdns::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
#ifdef DETAIL_LOGGING
    ChipLogDetail(Discovery, "MinMdns received a query.");
#endif

    mCurrentSource = info;
    if (!ParsePacket(data, this))
    {
        ChipLogError(Discovery, "Failed to parse mDNS query");
    }
    mCurrentSource = nullptr;
}

void AdvertiserMinMdns::OnQuery(const QueryData & data)
{
    if (mCurrentSource == nullptr)
    {
        ChipLogError(Discovery, "INTERNAL CONSISTENCY ERROR: missing query source");
        return;
    }

    LogQuery(data);

    CHIP_ERROR err = mResponseSender.Respond(mMessageId, data, mCurrentSource);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to reply to query: %s", ErrorStr(err));
    }
}

CHIP_ERROR AdvertiserMinMdns::Start(chip::Inet::InetLayer * inetLayer, uint16_t port)
{
    GlobalMinimalMdnsServer::Server().Shutdown();

    ReturnErrorOnFailure(GlobalMinimalMdnsServer::Instance().StartServer(inetLayer, port));

    ChipLogProgress(Discovery, "CHIP minimal mDNS started advertising.");

    AdvertiseRecords();

    return CHIP_NO_ERROR;
}

/// Stops the advertiser.
CHIP_ERROR AdvertiserMinMdns::StopPublishDevice()
{
    mQueryResponderAllocator.Clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::Advertise(const OperationalAdvertisingParameters & params)
{
    mQueryResponderAllocator.Clear();
    char nameBuffer[64] = "";

    /// need to set server name
    ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), params.GetPeerId()));

    FullQName operationalServiceName = mQueryResponderAllocator.AllocateQName("_chip", "_tcp", "local");
    FullQName operationalServerName  = mQueryResponderAllocator.AllocateQName(nameBuffer, "_chip", "_tcp", "local");

    ReturnErrorOnFailure(MakeHostName(nameBuffer, sizeof(nameBuffer), params.GetMac()));
    FullQName serverName = mQueryResponderAllocator.AllocateQName(nameBuffer, "local");

    if ((operationalServiceName.nameCount == 0) || (operationalServerName.nameCount == 0) || (serverName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponderAllocator.AddResponder<PtrResponder>(operationalServiceName, operationalServerName)
             .SetReportAdditional(operationalServerName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponderAllocator.AddResponder<SrvResponder>(SrvResourceRecord(operationalServerName, serverName, params.GetPort()))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!mQueryResponderAllocator.AddResponder<TxtResponder>(TxtResourceRecord(operationalServerName, mEmptyTextEntries))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponderAllocator.AddResponder<IPv6Responder>(serverName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!mQueryResponderAllocator.AddResponder<IPv4Responder>(serverName).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Operational device'.");

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::Advertise(const CommissionAdvertisingParameters & params)
{
    mQueryResponderAllocator.Clear();
    // TODO: need to detect colisions here
    char nameBuffer[64] = "";
    size_t len          = snprintf(nameBuffer, sizeof(nameBuffer), ChipLogFormatX64, GetRandU32(), GetRandU32());
    if (len >= sizeof(nameBuffer))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    const char * serviceType = params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode
        ? kCommissionableServiceName
        : kCommissionerServiceName;
    FullQName serviceName  = mQueryResponderAllocator.AllocateQName(serviceType, kCommissionProtocol, kLocalDomain);
    FullQName instanceName = mQueryResponderAllocator.AllocateQName(nameBuffer, serviceType, kCommissionProtocol, kLocalDomain);

    ReturnErrorOnFailure(MakeHostName(nameBuffer, sizeof(nameBuffer), params.GetMac()));
    FullQName hostName = mQueryResponderAllocator.AllocateQName(nameBuffer, kLocalDomain);

    if ((serviceName.nameCount == 0) || (instanceName.nameCount == 0) || (hostName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponderAllocator.AddResponder<PtrResponder>(serviceName, instanceName)
             .SetReportAdditional(instanceName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponderAllocator.AddResponder<SrvResponder>(SrvResourceRecord(instanceName, hostName, params.GetPort()))
             .SetReportAdditional(hostName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!mQueryResponderAllocator.AddResponder<IPv6Responder>(hostName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!mQueryResponderAllocator.AddResponder<IPv4Responder>(hostName).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (params.GetVendorId().HasValue())
    {
        MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                           DiscoveryFilter(DiscoveryFilterType::kVendor, params.GetVendorId().Value()));
        FullQName vendorServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                             kCommissionProtocol, kLocalDomain);
        ReturnErrorCodeIf(vendorServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!mQueryResponderAllocator.AddResponder<PtrResponder>(vendorServiceName, instanceName)
                 .SetReportAdditional(instanceName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add vendor PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (params.GetDeviceType().HasValue())
    {
        MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                           DiscoveryFilter(DiscoveryFilterType::kDeviceType, params.GetDeviceType().Value()));
        FullQName vendorServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                             kCommissionProtocol, kLocalDomain);
        ReturnErrorCodeIf(vendorServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!mQueryResponderAllocator.AddResponder<PtrResponder>(vendorServiceName, instanceName)
                 .SetReportAdditional(instanceName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add device type PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    // the following sub types only apply to commissionable node advertisements
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kShort, params.GetShortDiscriminator()));
            FullQName shortServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                                kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(shortServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

            if (!mQueryResponderAllocator.AddResponder<PtrResponder>(shortServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add short discriminator PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kLong, params.GetLongDiscriminator()));
            FullQName longServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                               kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
            if (!mQueryResponderAllocator.AddResponder<PtrResponder>(longServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add long discriminator PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kCommissioningMode, params.GetCommissioningMode() ? 1 : 0));
            FullQName longServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                               kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
            if (!mQueryResponderAllocator.AddResponder<PtrResponder>(longServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add commissioning mode PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        if (params.GetCommissioningMode() && params.GetOpenWindowCommissioningMode())
        {
            MakeServiceSubtype(nameBuffer, sizeof(nameBuffer),
                               DiscoveryFilter(DiscoveryFilterType::kCommissioningModeFromCommand, 1));
            FullQName longServiceName = mQueryResponderAllocator.AllocateQName(nameBuffer, kSubtypeServiceNamePart, serviceType,
                                                                               kCommissionProtocol, kLocalDomain);
            ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
            if (!mQueryResponderAllocator.AddResponder<PtrResponder>(longServiceName, instanceName)
                     .SetReportAdditional(instanceName)
                     .SetReportInServiceListing(true)
                     .IsValid())
            {
                ChipLogError(Discovery, "Failed to add open window commissioning mode PTR record mDNS responder");
                return CHIP_ERROR_NO_MEMORY;
            }
        }
    }

    if (!mQueryResponderAllocator.AddResponder<TxtResponder>(TxtResourceRecord(instanceName, GetCommisioningTextEntries(params)))
             .SetReportAdditional(hostName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Commissionable node device'.");
    }
    else
    {
        ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Commissioner device'.");
    }

    return CHIP_NO_ERROR;
}

FullQName AdvertiserMinMdns::GetCommisioningTextEntries(const CommissionAdvertisingParameters & params)
{
    // Max number of TXT fields from the spec is 9: D, VP, AP, CM, DT, DN, RI, PI, PH.
    constexpr size_t kMaxTxtFields = 9;
    const char * txtFields[kMaxTxtFields];
    size_t numTxtFields = 0;

    char txtVidPid[chip::Mdns::kKeyVendorProductMaxLength + 4];
    if (params.GetProductId().HasValue() && params.GetVendorId().HasValue())
    {
        sprintf(txtVidPid, "VP=%d+%d", params.GetVendorId().Value(), params.GetProductId().Value());
        txtFields[numTxtFields++] = txtVidPid;
    }
    else if (params.GetVendorId().HasValue())
    {
        sprintf(txtVidPid, "VP=%d", params.GetVendorId().Value());
        txtFields[numTxtFields++] = txtVidPid;
    }

    char txtDeviceType[chip::Mdns::kKeyDeviceTypeMaxLength + 4];
    if (params.GetDeviceType().HasValue())
    {
        sprintf(txtDeviceType, "DT=%d", params.GetDeviceType().Value());
        txtFields[numTxtFields++] = txtDeviceType;
    }

    char txtDeviceName[chip::Mdns::kKeyDeviceNameMaxLength + 4];
    if (params.GetDeviceName().HasValue())
    {
        sprintf(txtDeviceName, "DN=%s", params.GetDeviceName().Value());
        txtFields[numTxtFields++] = txtDeviceName;
    }

    // the following sub types only apply to commissionable node advertisements
    if (params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissionableNode)
    {
        // a discriminator always exists
        char txtDiscriminator[chip::Mdns::kKeyDiscriminatorMaxLength + 3];
        sprintf(txtDiscriminator, "D=%d", params.GetLongDiscriminator());
        txtFields[numTxtFields++] = txtDiscriminator;

        if (!params.GetVendorId().HasValue())
        {
            return mQueryResponderAllocator.AllocateQName(txtDiscriminator);
        }

        char txtCommissioningMode[chip::Mdns::kKeyCommissioningModeMaxLength + 4];
        sprintf(txtCommissioningMode, "CM=%d", params.GetCommissioningMode() ? 1 : 0);
        txtFields[numTxtFields++] = txtCommissioningMode;

        char txtOpenWindowCommissioningMode[chip::Mdns::kKeyAdditionalPairingMaxLength + 4];
        if (params.GetCommissioningMode() && params.GetOpenWindowCommissioningMode())
        {
            sprintf(txtOpenWindowCommissioningMode, "AP=1");
            txtFields[numTxtFields++] = txtOpenWindowCommissioningMode;
        }

        char txtRotatingDeviceId[chip::Mdns::kKeyRotatingIdMaxLength + 4];
        if (params.GetRotatingId().HasValue())
        {
            sprintf(txtRotatingDeviceId, "RI=%s", params.GetRotatingId().Value());
            txtFields[numTxtFields++] = txtRotatingDeviceId;
        }

        char txtPairingHint[chip::Mdns::kKeyPairingInstructionMaxLength + 4];
        if (params.GetPairingHint().HasValue())
        {
            sprintf(txtPairingHint, "PH=%d", params.GetPairingHint().Value());
            txtFields[numTxtFields++] = txtPairingHint;
        }

        char txtPairingInstr[chip::Mdns::kKeyPairingInstructionMaxLength + 4];
        if (params.GetPairingInstr().HasValue())
        {
            sprintf(txtPairingInstr, "PI=%s", params.GetPairingInstr().Value());
            txtFields[numTxtFields++] = txtPairingInstr;
        }
    }
    if (numTxtFields == 0)
    {
        return mQueryResponderAllocator.AllocateQNameFromArray(mEmptyTextEntries, 1);
    }
    else
    {
        return mQueryResponderAllocator.AllocateQNameFromArray(txtFields, numTxtFields);
    }
} // namespace

bool AdvertiserMinMdns::ShouldAdvertiseOn(const chip::Inet::InterfaceId id, const chip::Inet::IPAddress & addr)
{
    auto & server = GlobalMinimalMdnsServer::Server();
    for (unsigned i = 0; i < server.GetEndpointCount(); i++)
    {
        const ServerBase::EndpointInfo & info = server.GetEndpoints()[i];

        if (info.udp == nullptr)
        {
            continue;
        }

        if (info.interfaceId != id)
        {
            continue;
        }

        if (info.addressType != addr.Type())
        {
            continue;
        }

        return true;
    }

    return false;
}

void AdvertiserMinMdns::AdvertiseRecords()
{
    chip::Inet::InterfaceAddressIterator interfaceAddress;

    if (!interfaceAddress.Next())
    {
        return;
    }

    for (; interfaceAddress.HasCurrent(); interfaceAddress.Next())
    {
        if (!Internal::IsCurrentInterfaceUsable(interfaceAddress))
        {
            continue;
        }

        if (!ShouldAdvertiseOn(interfaceAddress.GetInterfaceId(), interfaceAddress.GetAddress()))
        {
            continue;
        }

        chip::Inet::IPPacketInfo packetInfo;

        packetInfo.Clear();
        packetInfo.SrcAddress = interfaceAddress.GetAddress();
        if (interfaceAddress.GetAddress().IsIPv4())
        {
            BroadcastIpAddresses::GetIpv4Into(packetInfo.DestAddress);
        }
        else
        {
            BroadcastIpAddresses::GetIpv6Into(packetInfo.DestAddress);
        }
        packetInfo.SrcPort   = kMdnsPort;
        packetInfo.DestPort  = kMdnsPort;
        packetInfo.Interface = interfaceAddress.GetInterfaceId();

        QueryData queryData(QType::PTR, QClass::IN, false /* unicast */);
        queryData.SetIsBootAdvertising(true);

        mQueryResponderAllocator.GetQueryResponder()->ClearBroadcastThrottle();

        CHIP_ERROR err = mResponseSender.Respond(0, queryData, &packetInfo);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to advertise records: %s", ErrorStr(err));
        }
    }

    // Once all automatic broadcasts are done, allow immediate replies once.
    mQueryResponderAllocator.GetQueryResponder()->ClearBroadcastThrottle();
}

AdvertiserMinMdns gAdvertiser;
} // namespace

ServiceAdvertiser & ServiceAdvertiser::Instance()
{
    return gAdvertiser;
}

} // namespace Mdns
} // namespace chip
