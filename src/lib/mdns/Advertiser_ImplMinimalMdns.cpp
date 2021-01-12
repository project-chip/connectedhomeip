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

#include <assert.h>
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
    State mState;
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
    CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint64_t macAddress, uint16_t port, bool enableIPv4) override;
    CHIP_ERROR AdvertiseOperational(const OperationalAdvertisingParameters & params) override;
    CHIP_ERROR AdvertiseCommission(const CommissionAdvertisingParameters & params) override;

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
    FullQName mHostname;

    // dynamically allocated items
    Responder * mAllocatedResponders[kMaxAllocatedResponders];
    void * mAllocatedQNameParts[kMaxAllocatedQNameData];

    const char * mEmptyTextEntries[1] = {
        "=",
    };

    char mDiscriminatorText[7];
    char mVendorProductIdText[15];
    const char * mCommissionTextEntries[2] = { mDiscriminatorText, mVendorProductIdText };
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

CHIP_ERROR AdvertiserMinMdns::Start(chip::Inet::InetLayer * inetLayer, uint64_t macAddress, uint16_t port, bool enableIPv4)
{
    mServer.Shutdown();

    char mac[17];
    ssize_t len = snprintf(mac, sizeof(mac), "%" PRIX64, macAddress);
    assert(static_cast<size_t>(len) < sizeof(mac));
    mHostname = AllocateQName(mac, "local");
    if (mHostname.nameCount == 0)
    {
        ChipLogError(Discovery, "Failed to allocate hostname.");
        return CHIP_ERROR_NO_MEMORY;
    }

    AllInterfaces allInterfaces;
    ReturnErrorOnFailure(mServer.Listen(inetLayer, &allInterfaces, port));

    if (!AddResponder<IPv6Responder>(mHostname).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (enableIPv4)
    {
        if (!AddResponder<IPv4Responder>(mHostname).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
    }

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

CHIP_ERROR AdvertiserMinMdns::AdvertiseOperational(const OperationalAdvertisingParameters & params)
{
    Clear();

    char uniqueName[64] = "";

    /// need to set server name
    ssize_t len = snprintf(uniqueName, sizeof(uniqueName), "%" PRIX64 "-%" PRIX64, params.GetFabricId(), params.GetNodeId());
    assert(static_cast<size_t>(len) < sizeof(uniqueName));

    FullQName operationalServiceName = AllocateQName("_chip", "_tcp", "local");
    FullQName operationalServerName  = AllocateQName(uniqueName, "_chip", "_tcp", "local");

    if ((operationalServiceName.nameCount == 0) || (operationalServerName.nameCount == 0))
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

    if (!AddResponder<SrvResponder>(SrvResourceRecord(operationalServerName, mHostname, params.GetPort()))
             .SetReportAdditional(mHostname)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!AddResponder<TxtResponder>(TxtResourceRecord(operationalServerName, mEmptyTextEntries))
             .SetReportAdditional(mHostname)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add TXT record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Operational device'.");

    return CHIP_NO_ERROR;
}

CHIP_ERROR AdvertiserMinMdns::AdvertiseCommission(const CommissionAdvertisingParameters & params)
{
    char uniqueName[17];
    char discriminatorName[7];

    ssize_t len = snprintf(uniqueName, sizeof(uniqueName), "%" PRIX64, params.GetIdentifier());
    assert(static_cast<size_t>(len) < sizeof(uniqueName));
    len = snprintf(discriminatorName, sizeof(discriminatorName), "_L%04X", params.GetDiscriminator());
    assert(static_cast<size_t>(len) < sizeof(discriminatorName));
    len = snprintf(mDiscriminatorText, sizeof(mDiscriminatorText), "D=%04X", params.GetDiscriminator());
    assert(static_cast<size_t>(len) < sizeof(mDiscriminatorText));
    len = snprintf(mVendorProductIdText, sizeof(mVendorProductIdText), "VP=%d+%d", params.GetVendorId(), params.GetProductId());
    assert(static_cast<size_t>(len) < sizeof(mVendorProductIdText));

    FullQName commissionServiceName = AllocateQName("_chipc", "_udp", "local");
    FullQName commissionServerName  = AllocateQName(uniqueName, "_chipc", "_udp", "local");
    FullQName discriptorSubTypeName = AllocateQName(discriminatorName, "_sub", "_chipc", "_udp", "local");

    if (!AddResponder<PtrResponder>(commissionServiceName, commissionServerName)
             .SetReportAdditional(commissionServerName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add commission service PTR record");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<PtrResponder>(discriptorSubTypeName, commissionServerName)
             .SetReportAdditional(commissionServerName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add commission service PTR record");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<SrvResponder>(SrvResourceRecord(commissionServerName, mHostname, params.GetPort()))
             .SetReportAdditional(mHostname)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add commission service SRV record");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!AddResponder<TxtResponder>(TxtResourceRecord(commissionServerName, mCommissionTextEntries))
             .SetReportAdditional(mHostname)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add commission service TXT record");
        return CHIP_ERROR_NO_MEMORY;
    }

    ChipLogProgress(Discovery, "CHIP minimal mDNS configured as 'Operational device'.");
    return CHIP_NO_ERROR;
}

AdvertiserMinMdns gAdvertiser;
} // namespace

ServiceAdvertiser & ServiceAdvertiser::Instance()
{
    return gAdvertiser;
}

} // namespace Mdns
} // namespace chip
