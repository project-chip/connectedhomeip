/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "Resolver.h"

#include <limits>

#include <lib/core/CHIPConfig.h>
#include <lib/dnssd/ActiveResolveAttempts.h>
#include <lib/dnssd/IncrementalResolve.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/QueryBuilder.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/core/FlatAllocatedQName.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>

// MDNS servers will receive all broadcast packets over the network.
// Disable 'invalid packet' messages because the are expected and common
// These logs are useful for debug only
#undef MINMDNS_RESOLVER_OVERLY_VERBOSE

namespace chip {
namespace Dnssd {
namespace {

constexpr size_t kMdnsMaxPacketSize = 1024;
constexpr uint16_t kMdnsPort        = 5353;

using namespace mdns::Minimal;

/// Handles processing of minmdns packet data.
///
/// Can process multiple incremental resolves based on SRV data and allows
/// retreival of pending (e.g. to ask for AAAA) and complete data items.
///
class PacketParser : private ParserDelegate
{
public:
    PacketParser() {}

    /// Goes through the given SRV records within a response packet
    /// and sets up data resolution
    void ParseSrvRecords(const BytesRange & packet);

    /// Goes through non-SRV records and feeds them through the initialized
    /// SRV record parsing.
    ///
    /// Must be called AFTER ParseSrvRecords has been called.
    void ParseNonSrvRecords(const BytesRange & packet);

    IncrementalResolver * ResolverBegin() { return mResolvers; }
    const IncrementalResolver * ResolverBegin() const { return mResolvers; }

    IncrementalResolver * ResolverEnd() { return mResolvers + kMinMdnsNumParallelResolvers; }
    const IncrementalResolver * ResolverEnd() const { return mResolvers + kMinMdnsNumParallelResolvers; }

private:
    // ParserDelegate implementation
    void OnHeader(ConstHeaderRef & header) override;
    void OnQuery(const QueryData & data) override;
    void OnResource(ResourceType type, const ResourceData & data) override;

    /// Called IFF data is of SRV type and we are in SRV initialization state
    ///
    /// Initializes a resolver with the given SRV content as long as
    /// inactive resolvers exist.
    void ParseSRVResource(const ResourceData & data);

    /// Called IFF parsing state is in RecordParsing
    ///
    /// Forwards the resource to all active resolvers.
    void ParseResource(const ResourceData & data);

    enum class RecordParsingState
    {
        kIdle,
        kSrvInitialization,
        kRecordParsing,
    };

    // TODO: use a CHIP constant here
    static constexpr size_t kMinMdnsNumParallelResolvers = 2;

    bool mIsResponse = false;
    BytesRange mPacketRange;
    IncrementalResolver mResolvers[kMinMdnsNumParallelResolvers];
    RecordParsingState mParsingState = RecordParsingState::kIdle;
};

void PacketParser::OnHeader(ConstHeaderRef & header)
{
    mIsResponse = header.GetFlags().IsResponse();

#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
    if (header.GetFlags().IsTruncated())
    {
        // MinMdns does not cache data, so receiving piecewise data does not work
        ChipLogError(Discovery, "Truncated responses not supported for address resolution");
    }
#endif
}

void PacketParser::OnQuery(const QueryData & data)
{
    // Ignore queries:
    //   - unicast answers will include the corresponding query in the answer
    //     packet, however that is not interesting for the resolver.
}

void PacketParser::OnResource(ResourceType type, const ResourceData & data)
{
    if (!mIsResponse)
    {
        return;
    }

    switch (mParsingState)
    {
    case RecordParsingState::kSrvInitialization: {
        if (data.GetType() != QType::SRV)
        {
            return;
        }
        ParseSRVResource(data);
        break;
    }
    case RecordParsingState::kRecordParsing:
        ParseResource(data);
        break;
    case RecordParsingState::kIdle:
        ChipLogError(Discovery, "Illegal state: received DNSSD resource while IDLE");
        break;
    }
}

void PacketParser::ParseResource(const ResourceData & data)
{
    for (auto & resolver : mResolvers)
    {
        if (resolver.IsActive())
        {
            CHIP_ERROR err = resolver.OnRecord(data, mPacketRange);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "DNSSD parse error: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }
}

void PacketParser::ParseSRVResource(const ResourceData & data)
{
    SrvRecord srv;
    if (!srv.Parse(data.GetData(), mPacketRange))
    {
        ChipLogError(Discovery, "Packet data reporter failed to parse SRV record");
        return;
    }

    for (auto & resolver : mResolvers)
    {
        if (resolver.IsActive() && (resolver.GetRecordName() == data.GetName()))
        {
            ChipLogDetail(Discovery, "SRV record already actively procssed.");
            return;
        }
    }

    for (auto & resolver : mResolvers)
    {
        if (resolver.IsActive())
        {
            continue;
        }

        CHIP_ERROR err = resolver.InitializeParsing(data.GetName(), srv);
        if (err != CHIP_NO_ERROR)
        {
            // Receiving records that we do not need to parse is normal:
            // MinMDNS may receive all DNSSD packets on the network, only
            // interested in a subset that is matter-specific
#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
            ChipLogError(Discovery, "Could not start SRV record processing: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            return;
        }
    }

    ChipLogError(Discovery, "Insufficient parsers to process all SRV entries.");
}

void PacketParser::ParseSrvRecords(const BytesRange & packet)
{
    mParsingState = RecordParsingState::kSrvInitialization;
    mPacketRange  = packet;

    if (!ParsePacket(packet, this))
    {
        ChipLogError(Discovery, "DNSSD packet parsing failed (for SRV records)");
    }

    mParsingState = RecordParsingState::kIdle;
}

void PacketParser::ParseNonSrvRecords(const BytesRange & packet)
{
    mParsingState = RecordParsingState::kRecordParsing;
    mPacketRange  = packet;

    if (!ParsePacket(packet, this))
    {
        ChipLogError(Discovery, "DNSSD packet parsing failed (for non-srv records)");
    }

    mParsingState = RecordParsingState::kIdle;
}

class MinMdnsResolver : public Resolver, public MdnsPacketDelegate
{
public:
    MinMdnsResolver() : mActiveResolves(&chip::System::SystemClock())
    {
        GlobalMinimalMdnsServer::Instance().SetResponseDelegate(this);
    }

    //// MdnsPacketDelegate implementation
    void OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override;

    ///// Resolver implementation
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager) override;
    void Shutdown() override;
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override { mOperationalDelegate = delegate; }
    void SetCommissioningDelegate(CommissioningResolveDelegate * delegate) override { mCommissioningDelegate = delegate; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) override;
    CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;

private:
    OperationalResolveDelegate * mOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mCommissioningDelegate = nullptr;
    DiscoveryType mDiscoveryType                          = DiscoveryType::kUnknown;
    System::Layer * mSystemLayer                          = nullptr;
    ActiveResolveAttempts mActiveResolves;
    PacketParser mPacketParser;

    CHIP_ERROR SendPendingResolveQueries();
    CHIP_ERROR SendPendingBrowseQueries();
    CHIP_ERROR SendAllPendingQueries();
    CHIP_ERROR ScheduleRetries();

    static void RetryCallback(System::Layer *, void * self);

    CHIP_ERROR SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type, bool unicastResponse);
    CHIP_ERROR BrowseNodes(DiscoveryType type, DiscoveryFilter subtype);
    template <typename... Args>
    mdns::Minimal::FullQName CheckAndAllocateQName(Args &&... parts)
    {
        size_t requiredSize = mdns::Minimal::FlatAllocatedQName::RequiredStorageSize(parts...);
        if (requiredSize > kMaxQnameSize)
        {
            return mdns::Minimal::FullQName();
        }
        return mdns::Minimal::FlatAllocatedQName::Build(qnameStorage, parts...);
    }
    static constexpr int kMaxQnameSize = 100;
    char qnameStorage[kMaxQnameSize];
};

void MinMdnsResolver::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    // Fill up any relevant data
    mPacketParser.ParseSrvRecords(data);
    mPacketParser.ParseNonSrvRecords(data);

    // TODO:
    //   - loop through results and schedule OnComplete
    //   - schedule AAAA queries for elements requiring it
    //   - done?

    ScheduleRetries();
}

CHIP_ERROR MinMdnsResolver::Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager)
{
    /// Note: we do not double-check the port as we assume the APP will always use
    /// the same udpEndPointManager and port for mDNS.
    mSystemLayer = &udpEndPointManager->SystemLayer();

    if (GlobalMinimalMdnsServer::Server().IsListening())
    {
        return CHIP_NO_ERROR;
    }

    return GlobalMinimalMdnsServer::Instance().StartServer(udpEndPointManager, kMdnsPort);
}

void MinMdnsResolver::Shutdown()
{
    GlobalMinimalMdnsServer::Instance().ShutdownServer();
}

CHIP_ERROR MinMdnsResolver::SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type, bool unicastResponse)
{
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    QueryBuilder builder(std::move(buffer));
    builder.Header().SetMessageId(0);

    mdns::Minimal::Query query(qname);
    query.SetType(type).SetClass(mdns::Minimal::QClass::IN);
    query.SetAnswerViaUnicast(unicastResponse);

    builder.AddQuery(query);

    ReturnErrorCodeIf(!builder.Ok(), CHIP_ERROR_INTERNAL);

    if (unicastResponse)
    {
        ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastUnicastQuery(builder.ReleasePacket(), kMdnsPort));
    }
    else
    {
        ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastSend(builder.ReleasePacket(), kMdnsPort));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::SendAllPendingQueries()
{
    CHIP_ERROR browseErr  = SendPendingBrowseQueries();
    CHIP_ERROR resolveErr = SendPendingResolveQueries();
    return resolveErr == CHIP_NO_ERROR ? browseErr : resolveErr;
}

CHIP_ERROR MinMdnsResolver::FindCommissionableNodes(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionableNode, filter);
}

CHIP_ERROR MinMdnsResolver::FindCommissioners(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionerNode, filter);
}

CHIP_ERROR MinMdnsResolver::BrowseNodes(DiscoveryType type, DiscoveryFilter filter)
{
    mDiscoveryType = type;
    mActiveResolves.MarkPending(filter, type);

    return SendPendingBrowseQueries();
}

CHIP_ERROR MinMdnsResolver::SendPendingBrowseQueries()
{
    CHIP_ERROR returnErr = CHIP_NO_ERROR;
    while (true)
    {
        Optional<ActiveResolveAttempts::ScheduledAttempt> attempt = mActiveResolves.NextScheduled();

        if (!attempt.HasValue())
        {
            break;
        }
        if (!attempt.Value().IsBrowse())
        {
            continue;
        }

        mdns::Minimal::FullQName qname;

        switch (attempt.Value().browse.type)
        {
        case DiscoveryType::kOperational:
            qname = CheckAndAllocateQName(kOperationalServiceName, kOperationalProtocol, kLocalDomain);
            break;
        case DiscoveryType::kCommissionableNode:
            if (attempt.Value().browse.filter.type == DiscoveryFilterType::kNone)
            {
                qname = CheckAndAllocateQName(kCommissionableServiceName, kCommissionProtocol, kLocalDomain);
            }
            else if (attempt.Value().browse.filter.type == DiscoveryFilterType::kInstanceName)
            {
                qname = CheckAndAllocateQName(attempt.Value().browse.filter.instanceName, kCommissionableServiceName,
                                              kCommissionProtocol, kLocalDomain);
            }
            else
            {
                char subtypeStr[Common::kSubTypeMaxLength + 1];
                ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), attempt.Value().browse.filter));
                qname = CheckAndAllocateQName(subtypeStr, kSubtypeServiceNamePart, kCommissionableServiceName, kCommissionProtocol,
                                              kLocalDomain);
            }
            break;
        case DiscoveryType::kCommissionerNode:
            if (attempt.Value().browse.filter.type == DiscoveryFilterType::kNone)
            {
                qname = CheckAndAllocateQName(kCommissionerServiceName, kCommissionProtocol, kLocalDomain);
            }
            else
            {
                char subtypeStr[Common::kSubTypeMaxLength + 1];
                ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), attempt.Value().browse.filter));
                qname = CheckAndAllocateQName(subtypeStr, kSubtypeServiceNamePart, kCommissionerServiceName, kCommissionProtocol,
                                              kLocalDomain);
            }
            break;
        case DiscoveryType::kUnknown:
            break;
        }
        if (!qname.nameCount)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        bool unicastResponse = attempt.Value().firstSend;

        CHIP_ERROR err = SendQuery(qname, mdns::Minimal::QType::ANY, unicastResponse);
        if (err != CHIP_NO_ERROR)
        {
            // We want to continue sending, but we do want this error returned
            returnErr = err;
        }
    }
    ReturnErrorOnFailure(ScheduleRetries());
    return returnErr;
}

CHIP_ERROR MinMdnsResolver::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    mDiscoveryType = DiscoveryType::kOperational;
    mActiveResolves.MarkPending(peerId);

    return SendPendingResolveQueries();
}

CHIP_ERROR MinMdnsResolver::ScheduleRetries()
{
    ReturnErrorCodeIf(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mSystemLayer->CancelTimer(&RetryCallback, this);

    Optional<System::Clock::Timeout> delay = mActiveResolves.GetTimeUntilNextExpectedResponse();

    if (!delay.HasValue())
    {
        return CHIP_NO_ERROR;
    }

    return mSystemLayer->StartTimer(delay.Value(), &RetryCallback, this);
}

void MinMdnsResolver::RetryCallback(System::Layer *, void * self)
{
    reinterpret_cast<MinMdnsResolver *>(self)->SendAllPendingQueries();
}

CHIP_ERROR MinMdnsResolver::SendPendingResolveQueries()
{
    while (true)
    {
        Optional<ActiveResolveAttempts::ScheduledAttempt> resolve = mActiveResolves.NextScheduled();

        if (!resolve.HasValue())
        {
            break;
        }
        if (!resolve.Value().IsResolve())
        {
            continue;
        }

        System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
        ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        QueryBuilder builder(std::move(buffer));
        builder.Header().SetMessageId(0);

        {
            char nameBuffer[kMaxOperationalServiceNameSize] = "";

            // Node and fabricid are encoded in server names.
            ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), resolve.Value().peerId));

            const char * instanceQName[] = { nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain };
            Query query(instanceQName);

            query
                .SetClass(QClass::IN)                           //
                .SetType(QType::ANY)                            //
                .SetAnswerViaUnicast(resolve.Value().firstSend) //
                ;

            // NOTE: type above is NOT A or AAAA because the name searched for is
            // a SRV record. The layout is:
            //    SRV -> hostname
            //    Hostname -> A
            //    Hostname -> AAAA
            //
            // Query is sent for ANY and expectation is to receive A/AAAA records
            // in the additional section of the reply.
            //
            // Sending a A/AAAA query will return no results
            // Sending a SRV query will return the srv only and an additional query
            // would be needed to resolve the host name to an IP address

            builder.AddQuery(query);
        }

        ReturnErrorCodeIf(!builder.Ok(), CHIP_ERROR_INTERNAL);

        if (resolve.Value().firstSend)
        {
            ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastUnicastQuery(builder.ReleasePacket(), kMdnsPort));
        }
        else
        {
            ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastSend(builder.ReleasePacket(), kMdnsPort));
        }
    }

    return ScheduleRetries();
}

MinMdnsResolver gResolver;

} // namespace

Resolver & chip::Dnssd::Resolver::Instance()
{
    return gResolver;
}

// Minimal implementation does not support associating a context to a request (while platforms implementations do). So keep
// updating the delegate that ends up being used by the server by calling 'SetOperationalDelegate'.
// This effectively allow minimal to have multiple controllers issuing requests as long the requests are serialized, but
// it won't work well if requests are issued in parallel.
CHIP_ERROR ResolverProxy::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Discovery, "Resolving " ChipLogFormatX64 ":" ChipLogFormatX64 " ...",
                    ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()));
    chip::Dnssd::Resolver::Instance().SetOperationalDelegate(mDelegate);
    return chip::Dnssd::Resolver::Instance().ResolveNodeId(peerId, type);
}

CHIP_ERROR ResolverProxy::FindCommissionableNodes(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    chip::Dnssd::Resolver::Instance().SetCommissioningDelegate(mDelegate);
    return chip::Dnssd::Resolver::Instance().FindCommissionableNodes(filter);
}

CHIP_ERROR ResolverProxy::FindCommissioners(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    chip::Dnssd::Resolver::Instance().SetCommissioningDelegate(mDelegate);
    return chip::Dnssd::Resolver::Instance().FindCommissioners(filter);
}

} // namespace Dnssd
} // namespace chip
