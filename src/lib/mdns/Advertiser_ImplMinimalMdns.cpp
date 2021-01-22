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
#include <support/ReturnMacros.h>
#include <support/StringBuilder.h>

namespace chip {
namespace Mdns {
namespace {

using namespace mdns::Minimal;

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

class AllInterfaces : public ListenIterator
{
public:
    AllInterfaces() {}

    bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) override
    {
#if INET_CONFIG_ENABLE_IPV4
        if (mState == State::kIpV4)
        {
            *id    = INET_NULL_INTERFACEID;
            *type  = chip::Inet::kIPAddressType_IPv4;
            mState = State::kIpV6;

            SkipToFirstValidInterface();
            return true;
        }
#else
        mState = State::kIpV6;
        SkipToFirstValidInterface();
#endif

        if (!mIterator.HasCurrent())
        {
            return false;
        }

        *id   = mIterator.GetInterfaceId();
        *type = chip::Inet::kIPAddressType_IPv6;

        for (mIterator.Next(); SkipCurrentInterface(); mIterator.Next())
        {
        }
        return true;
    }

private:
    enum class State
    {
        kIpV4,
        kIpV6,
    };
    State mState = State::kIpV4;
    chip::Inet::InterfaceIterator mIterator;

    void SkipToFirstValidInterface()
    {
        do
        {
            if (!SkipCurrentInterface())
            {
                break;
            }
        } while (mIterator.Next());
    }

    bool SkipCurrentInterface()
    {
        if (!mIterator.HasCurrent())
        {
            return false; // nothing to try.
        }

        if (!mIterator.IsUp() || !mIterator.SupportsMulticast())
        {
            return true; // not a usable interface
        }
        char name[64];
        if (mIterator.GetInterfaceName(name, sizeof(name)) != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Interface iterator failed to get interface name.");
            return true;
        }

        if (strncmp(name, "lo", 2) == 0)
        {
            ChipLogDetail(Discovery, "Skipping interface '%s' (assume local loopback)", name);
            return true;
        }
        return false;
    }
};

class AdvertiserMinMdns : public ServiceAdvertiser,
                          public ServerDelegate, // gets queries
                          public ParserDelegate  // parses queries
{
public:
    AdvertiserMinMdns() : mResponseSender(&mServer, &mQueryResponder)

    {
        mServer.SetDelegate(this);

        for (size_t i = 0; i < kMaxAllocatedResponders; i++)
        {
            mAllocatedResponders[i] = nullptr;
        }
        for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
        {
            mAllocatedQNameParts[i] = nullptr;
        }
    }
    ~AdvertiserMinMdns() { Clear(); }

    // Service advertiser
    CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint16_t port) override;
    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override;
    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override;

    // ServerDelegate
    void OnQuery(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override;
    void OnResponse(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override {}

    // ParserDelegate
    void OnHeader(ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(ResourceType type, const ResourceData & data) override {}
    void OnQuery(const QueryData & data) override;

private:
    /// Sets the query responder to a blank state and frees up any
    /// allocated memory.
    void Clear();

    QueryResponderSettings AddAllocatedResponder(Responder * responder)
    {
        if (responder == nullptr)
        {
            ChipLogError(Discovery, "Responder memory allocation failed");
            return QueryResponderSettings(); // failed
        }

        for (size_t i = 0; i < kMaxAllocatedResponders; i++)
        {
            if (mAllocatedResponders[i] != nullptr)
            {
                continue;
            }

            mAllocatedResponders[i] = responder;
            return mQueryResponder.AddResponder(mAllocatedResponders[i]);
        }

        Platform::Delete(responder);
        ChipLogError(Discovery, "Failed to find free slot for adding a responder");
        return QueryResponderSettings();
    }

    /// Appends another responder to the internal replies.
    template <typename ResponderType, typename... Args>
    QueryResponderSettings AddResponder(Args &&... args)
    {
        return AddAllocatedResponder(chip::Platform::New<ResponderType>(std::forward<Args>(args)...));
    }

    template <typename... Args>
    FullQName AllocateQName(Args &&... names)
    {
        for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
        {
            if (mAllocatedQNameParts[i] != nullptr)
            {
                continue;
            }

            mAllocatedQNameParts[i] =
                chip::Platform::MemoryAlloc(FlatAllocatedQName::RequiredStorageSize(std::forward<Args>(names)...));

            if (mAllocatedQNameParts[i] == nullptr)
            {
                ChipLogError(Discovery, "QName memory allocation failed");
                return FullQName();
            }
            return FlatAllocatedQName::Build(mAllocatedQNameParts[i], std::forward<Args>(names)...);
        }

        ChipLogError(Discovery, "Failed to find free slot for adding a qname");
        return FullQName();
    }

    FullQName GetCommisioningTextEntries(const CommissionAdvertisingParameters & params);

    static constexpr size_t kMaxEndPoints           = 10;
    static constexpr size_t kMaxRecords             = 16;
    static constexpr size_t kMaxAllocatedResponders = 16;
    static constexpr size_t kMaxAllocatedQNameData  = 8;

    Server<kMaxEndPoints> mServer;
    QueryResponder<kMaxRecords> mQueryResponder;
    ResponseSender mResponseSender;

    // current request handling
    const chip::Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint32_t mMessageId                             = 0;

    // dynamically allocated items
    Responder * mAllocatedResponders[kMaxAllocatedResponders];
    void * mAllocatedQNameParts[kMaxAllocatedQNameData];

    const char * mEmptyTextEntries[1] = {
        "=",
    };
};

void AdvertiserMinMdns::OnQuery(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    ChipLogDetail(Discovery, "MinMdns received a query.");

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
    mServer.Shutdown();

    AllInterfaces allInterfaces;

    ReturnErrorOnFailure(mServer.Listen(inetLayer, &allInterfaces, port));

    ChipLogProgress(Discovery, "CHIP minimal mDNS started advertising.");
    return CHIP_NO_ERROR;
}

void AdvertiserMinMdns::Clear()
{
    // Init clears all responders, so that data can be freed
    mQueryResponder.Init();

    // Free all allocated data
    for (size_t i = 0; i < kMaxAllocatedResponders; i++)
    {
        if (mAllocatedResponders[i] != nullptr)
        {
            chip::Platform::Delete(mAllocatedResponders[i]);
            mAllocatedResponders[i] = nullptr;
        }
    }

    for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
    {
        if (mAllocatedQNameParts[i] != nullptr)
        {
            chip::Platform::MemoryFree(mAllocatedQNameParts[i]);
            mAllocatedQNameParts[i] = nullptr;
        }
    }
}

CHIP_ERROR AdvertiserMinMdns::Advertise(const OperationalAdvertisingParameters & params)
{
    Clear();

    char uniqueName[64] = "";

    /// need to set server name
    size_t len = snprintf(uniqueName, sizeof(uniqueName), "%" PRIX64 "-%" PRIX64, params.GetFabricId(), params.GetNodeId());
    if (len >= sizeof(uniqueName))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    FullQName operationalServiceName = AllocateQName("_chip", "_tcp", "local");
    FullQName operationalServerName  = AllocateQName(uniqueName, "_chip", "_tcp", "local");
    FullQName serverName             = AllocateQName(uniqueName, "local");

    if ((operationalServiceName.nameCount == 0) || (operationalServerName.nameCount == 0) || (serverName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<PtrResponder>(operationalServiceName, operationalServerName)
             .SetReportAdditional(operationalServerName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<SrvResponder>(SrvResourceRecord(operationalServerName, serverName, params.GetPort()))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!AddResponder<TxtResponder>(TxtResourceRecord(operationalServerName, mEmptyTextEntries))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<IPv6Responder>(serverName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!AddResponder<IPv4Responder>(serverName).IsValid())
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
    Clear();

    // TODO: need to detect colisions here
    char nameBuffer[64] = "";
    size_t len          = snprintf(nameBuffer, sizeof(nameBuffer), "%016" PRIX64, GetRandU64());
    if (len >= sizeof(nameBuffer))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    const char * serviceType = params.GetCommissionAdvertiseMode() == CommssionAdvertiseMode::kCommissioning ? "_chipc" : "_chipd";

    FullQName operationalServiceName = AllocateQName(serviceType, "_udp", "local");
    FullQName operationalServerName  = AllocateQName(nameBuffer, serviceType, "_udp", "local");
    FullQName serverName             = AllocateQName(nameBuffer, "local");

    if ((operationalServiceName.nameCount == 0) || (operationalServerName.nameCount == 0) || (serverName.nameCount == 0))
    {
        ChipLogError(Discovery, "Failed to allocate QNames.");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<PtrResponder>(operationalServiceName, operationalServerName)
             .SetReportAdditional(operationalServerName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<SrvResponder>(SrvResourceRecord(operationalServerName, serverName, params.GetPort()))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!AddResponder<IPv6Responder>(serverName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!AddResponder<IPv4Responder>(serverName).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    {
        sprintf(nameBuffer, "S%03d", params.GetShortDiscriminator());
        FullQName shortServiceName = AllocateQName(nameBuffer, "_sub", serviceType, "_udp", "local");
        ReturnErrorCodeIf(shortServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!AddResponder<PtrResponder>(shortServiceName, operationalServerName)
                 .SetReportAdditional(operationalServerName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add short discriminator PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    {
        sprintf(nameBuffer, "L%04d", params.GetLongDiscriminator());
        FullQName longServiceName = AllocateQName(nameBuffer, "_sub", serviceType, "_udp", "local");
        ReturnErrorCodeIf(longServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);
        if (!AddResponder<PtrResponder>(longServiceName, operationalServerName)
                 .SetReportAdditional(operationalServerName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add long discriminator PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (params.GetVendorId().HasValue())
    {
        sprintf(nameBuffer, "V%d", params.GetVendorId().Value());
        FullQName vendorServiceName = AllocateQName(nameBuffer, "_sub", serviceType, "_udp", "local");
        ReturnErrorCodeIf(vendorServiceName.nameCount == 0, CHIP_ERROR_NO_MEMORY);

        if (!AddResponder<PtrResponder>(vendorServiceName, operationalServerName)
                 .SetReportAdditional(operationalServerName)
                 .SetReportInServiceListing(true)
                 .IsValid())
        {
            ChipLogError(Discovery, "Failed to add vendor discriminator PTR record mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (!AddResponder<TxtResponder>(TxtResourceRecord(operationalServerName, GetCommisioningTextEntries(params)))
             .SetReportAdditional(serverName)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Commisioning device'.");

    return CHIP_NO_ERROR;
}

FullQName AdvertiserMinMdns::GetCommisioningTextEntries(const CommissionAdvertisingParameters & params)
{
    // a discriminator always exists
    char txtDiscriminator[32];
    sprintf(txtDiscriminator, "D=%d", params.GetLongDiscriminator());

    if (!params.GetVendorId().HasValue())
    {
        return AllocateQName(txtDiscriminator);
    }

    // Need to also set a vid/pid string
    char txtVidPid[64];
    if (params.GetProductId().HasValue())
    {
        sprintf(txtVidPid, "V=%d+%d", params.GetVendorId().Value(), params.GetProductId().Value());
    }
    else
    {
        sprintf(txtVidPid, "V=%d", params.GetVendorId().Value());
    }

    char txtPairingInstrHint[128];
    if (params.GetPairingInstr().HasValue() && params.GetPairingHint().HasValue())
    {
        sprintf(txtPairingInstrHint, "P=%s+%d", params.GetPairingInstr().Value(), params.GetPairingHint().Value());
        return AllocateQName(txtDiscriminator, txtVidPid, txtPairingInstrHint);
    }
    else
    {
        return AllocateQName(txtDiscriminator, txtVidPid);
    }
}

AdvertiserMinMdns gAdvertiser;
} // namespace

ServiceAdvertiser & ServiceAdvertiser::Instance()
{
    return gAdvertiser;
}

} // namespace Mdns
} // namespace chip
