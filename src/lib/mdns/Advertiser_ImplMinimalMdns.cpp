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
#include <mdns/minimal/responders/IP.h>
#include <mdns/minimal/responders/Ptr.h>
#include <mdns/minimal/responders/QueryResponder.h>
#include <mdns/minimal/responders/Srv.h>
#include <mdns/minimal/responders/Txt.h>
#include <support/ReturnMacros.h>

namespace chip {
namespace Mdns {
namespace {

template <size_t kSize>
class StringBuilder
{
public:
    const char * c_str() const { return mBuffer; }

    StringBuilder & Append(const char * s)
    {
        size_t available = kSize - mUsed;
        size_t len       = strlen(s);

        if (len > available)
        {
            len = available - 1;
        }
        memcpy(mBuffer + mUsed, s, len);
        mUsed += len;
        mBuffer[mUsed] = 0;

        return *this;
    }

private:
    char mBuffer[kSize] = { 0 };
    size_t mUsed        = 0;
};

void LogQuery(const mdns::Minimal::QueryData & data)
{
    StringBuilder<128> logString;

    logString.Append("QUERY ");
    switch (data.GetClass())
    {
    case mdns::Minimal::QClass::IN:
        logString.Append("IN");
        break;
    default:
        logString.Append("???");
        break;
    }
    logString.Append("/");
    switch (data.GetType())
    {
    case mdns::Minimal::QType::ANY:
        logString.Append("ANY");
        break;
    case mdns::Minimal::QType::A:
        logString.Append("A");
        break;
    case mdns::Minimal::QType::AAAA:
        logString.Append("AAAA");
        break;
    case mdns::Minimal::QType::TXT:
        logString.Append("TXT");
        break;
    case mdns::Minimal::QType::SRV:
        logString.Append("SRV");
        break;
    case mdns::Minimal::QType::PTR:
        logString.Append("PTR");
        break;
    default:
        logString.Append("???");
        break;
    }
    logString.Append(": ");

    mdns::Minimal::SerializedQNameIterator name = data.GetName();
    while (name.Next())
    {
        logString.Append(name.Value()).Append(".");
    }

    ChipLogDetail(Discovery, "%s", logString.c_str());
}

class AllInterfaces : public mdns::Minimal::ListenIterator
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
        if (SkipCurrentInterface())
        {
            while (mIterator.Next())
            {
                if (!SkipCurrentInterface())
                {
                    break;
                }
            }
        }
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
        if (!mIterator.GetInterfaceName(name, sizeof(name)) == CHIP_NO_ERROR)
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
                          public mdns::Minimal::ServerDelegate, // gets queries
                          public mdns::Minimal::ParserDelegate  // parses queries
{
public:
    AdvertiserMinMdns() :
        mResponseSender(&mServer, &mQueryResponder), mPtrResponder(mOperationalServiceQName, mOperationalServerQName),
        mSrvResponder(mOperationalServerQName, mdns::Minimal::SrvResourceRecord(mOperationalServiceQName, mServerQName, CHIP_PORT)),
        mIPv4Responder(mServerQName), mIPv6Responder(mServerQName)

    {
        mServer.SetDelegate(this);
    }

    // Service advertiser
    CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint16_t port) override;
    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override;

    // ServerDelegate
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override;
    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override {}

    // ParserDelegate
    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data) override {}
    void OnQuery(const mdns::Minimal::QueryData & data) override;

private:
    static constexpr size_t kMaxEndPoints = 10;
    static constexpr size_t kMaxRecords   = 16;

    mdns::Minimal::Server<kMaxEndPoints> mServer;
    mdns::Minimal::QueryResponder<kMaxRecords> mQueryResponder;
    mdns::Minimal::ResponseSender mResponseSender;

    // current request handling
    const chip::Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint32_t mMessageId                             = 0;

    /// data members for variable things
    char mServerName[64] = "";

    mdns::Minimal::QNamePart mOperationalServiceQName[3] = { "_chip", "_tcp", "local" };
    mdns::Minimal::QNamePart mOperationalServerQName[4]  = { mServerName, "_chip", "_tcp", "local" };
    mdns::Minimal::QNamePart mServerQName[2]             = { mServerName, "local" };

    /// responders
    mdns::Minimal::PtrResponder mPtrResponder;
    mdns::Minimal::SrvResponder mSrvResponder;
    mdns::Minimal::IPv4Responder mIPv4Responder;
    mdns::Minimal::IPv6Responder mIPv6Responder;
};

void AdvertiserMinMdns::OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    ChipLogDetail(Discovery, "MinMdns received a query.");

    mCurrentSource = info;
    if (!mdns::Minimal::ParsePacket(data, this))
    {
        ChipLogError(Discovery, "Failed to parse mDNS query");
    }
    mCurrentSource = nullptr;
}

void AdvertiserMinMdns::OnQuery(const mdns::Minimal::QueryData & data)
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

CHIP_ERROR AdvertiserMinMdns::Advertise(const OperationalAdvertisingParameters & params)
{
    mQueryResponder.Init(); // start fresh

    /// need to set server name
    size_t len = snprintf(mServerName, sizeof(mServerName), "%" PRIX64 "-%" PRIX64, params.GetFabricId(), params.GetNodeId());
    if (len >= sizeof(mServerName))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (!mQueryResponder.AddResponder(&mPtrResponder)
             .SetReportAdditional(mOperationalServerQName)
             .SetReportInServiceListing(true)
             .IsValid())
    {
        ChipLogError(Discovery, "Failed to add service PTR record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    mSrvResponder.SetRecord(mdns::Minimal::SrvResourceRecord(mOperationalServiceQName, mServerQName, params.GetPort()));
    if (!mQueryResponder.AddResponder(&mSrvResponder).SetReportAdditional(mServerQName).IsValid())
    {
        ChipLogError(Discovery, "Failed to add SRV record mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (!mQueryResponder.AddResponder(&mIPv6Responder).IsValid())
    {
        ChipLogError(Discovery, "Failed to add IPv6 mDNS responder");
        return CHIP_ERROR_NO_MEMORY;
    }

    if (params.IsIPv4Enabled())
    {
        if (!mQueryResponder.AddResponder(&mIPv4Responder).IsValid())
        {
            ChipLogError(Discovery, "Failed to add IPv4 mDNS responder");
            return CHIP_ERROR_NO_MEMORY;
        }
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
