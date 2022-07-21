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
#include <lib/dnssd/minimal_mdns/Logging.h>
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
/// retrieval of pending (e.g. to ask for AAAA) and complete data items.
///
class PacketParser : private ParserDelegate
{
public:
    PacketParser(ActiveResolveAttempts & activeResolves) : mActiveResolves(activeResolves) {}

    /// Goes through the given SRV records within a response packet
    /// and sets up data resolution
    void ParseSrvRecords(const BytesRange & packet);

    /// Goes through non-SRV records and feeds them through the initialized
    /// SRV record parsing.
    ///
    /// Must be called AFTER ParseSrvRecords has been called.
    void ParseNonSrvRecords(Inet::InterfaceId interface, const BytesRange & packet);

    IncrementalResolver * ResolverBegin() { return mResolvers; }
    IncrementalResolver * ResolverEnd() { return mResolvers + kMinMdnsNumParallelResolvers; }

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

    static constexpr size_t kMinMdnsNumParallelResolvers = CHIP_CONFIG_MINMDNS_MAX_PARALLEL_RESOLVES;

    // Individual parse set
    bool mIsResponse               = false;
    Inet::InterfaceId mInterfaceId = Inet::InterfaceId::Null();
    BytesRange mPacketRange;
    RecordParsingState mParsingState = RecordParsingState::kIdle;

    // resolvers kept between parse steps
    ActiveResolveAttempts & mActiveResolves;
    IncrementalResolver mResolvers[kMinMdnsNumParallelResolvers];
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
        mdns::Minimal::Logging::LogReceivedResource(data);
        ParseSRVResource(data);
        break;
    }
    case RecordParsingState::kRecordParsing:
        if (data.GetType() != QType::SRV)
        {
            // SRV packets logged during 'SrvInitialization' phase
            mdns::Minimal::Logging::LogReceivedResource(data);
        }
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
            CHIP_ERROR err = resolver.OnRecord(mInterfaceId, data, mPacketRange);

            //
            // CHIP_ERROR_NO_MEMORY usually gets returned when we have no more memory available to hold the
            // resolved data. This gets emitted fairly frequently in dense environments or when receiving records
            // from devices with lots of interfaces. Consequently, don't log that unless we have DNS verbosity
            // logging enabled.
            //
            if (err != CHIP_NO_ERROR)
            {
#if !CHIP_MINMDNS_HIGH_VERBOSITY
                if (err != CHIP_ERROR_NO_MEMORY)
#endif
                    ChipLogError(Discovery, "DNSSD parse error: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }

    // Once an IP address is received, stop requesting it.
    if (data.GetType() == QType::AAAA)
    {
        mActiveResolves.CompleteIpResolution(data.GetName());
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
            ChipLogDetail(Discovery, "SRV record already actively processed.");
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
        }

        // Done finding an inactive resolver and attempting to use it.
        return;
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

void PacketParser::ParseNonSrvRecords(Inet::InterfaceId interface, const BytesRange & packet)
{
    mParsingState = RecordParsingState::kRecordParsing;
    mPacketRange  = packet;
    mInterfaceId  = interface;

    if (!ParsePacket(packet, this))
    {
        ChipLogError(Discovery, "DNSSD packet parsing failed (for non-srv records)");
    }

    mParsingState = RecordParsingState::kIdle;
}

class MinMdnsResolver : public Resolver, public MdnsPacketDelegate
{
public:
    MinMdnsResolver() : mActiveResolves(&chip::System::SystemClock()), mPacketParser(mActiveResolves)
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
    CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;

private:
    OperationalResolveDelegate * mOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mCommissioningDelegate = nullptr;
    System::Layer * mSystemLayer                          = nullptr;
    ActiveResolveAttempts mActiveResolves;
    PacketParser mPacketParser;

    void ScheduleIpAddressResolve(SerializedQNameIterator hostName);

    CHIP_ERROR SendAllPendingQueries();
    CHIP_ERROR ScheduleRetries();

    /// Prepare a query for the given schedule attempt
    CHIP_ERROR BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt & attempt);

    /// Prepare a query for specific resolve types
    CHIP_ERROR BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::Browse & data, bool firstSend);
    CHIP_ERROR BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::Resolve & data, bool firstSend);
    CHIP_ERROR BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::IpResolve & data, bool firstSend);

    /// Clear any incremental resolver that is not waiting for a AAAA address.
    void ExpireIncrementalResolvers();
    void AdvancePendingResolverStates();

    static void RetryCallback(System::Layer *, void * self);

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

void MinMdnsResolver::ScheduleIpAddressResolve(SerializedQNameIterator hostName)
{
    HeapQName target(hostName);
    if (!target.IsOk())
    {
        ChipLogError(Discovery, "Memory allocation error for IP address resolution");
        return;
    }
    mActiveResolves.MarkPending(ActiveResolveAttempts::ScheduledAttempt::IpResolve(std::move(target)));
}

void MinMdnsResolver::AdvancePendingResolverStates()
{
    for (IncrementalResolver * resolver = mPacketParser.ResolverBegin(); resolver != mPacketParser.ResolverEnd(); resolver++)
    {
        if (!resolver->IsActive())
        {
            continue;
        }

        IncrementalResolver::RequiredInformationFlags missing = resolver->GetMissingRequiredInformation();

        if (missing.Has(IncrementalResolver::RequiredInformationBitFlags::kIpAddress))
        {
            ScheduleIpAddressResolve(resolver->GetTargetHostName());
            continue;
        }

        if (missing.HasAny())
        {
            // Expect either IP missing (ask for it) or done. Anything else is not handled
            ChipLogError(Discovery, "Unexpected state: cannot advance resolver with missing information");
            resolver->ResetToInactive();
            continue;
        }

        // SUCCESS. Call the delegates
        if (resolver->IsActiveCommissionParse())
        {
            DiscoveredNodeData nodeData;

            CHIP_ERROR err = resolver->Take(nodeData);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to take discovery result: %" CHIP_ERROR_FORMAT, err.Format());
            }

            mActiveResolves.Complete(nodeData);
            if (mCommissioningDelegate != nullptr)
            {
                mCommissioningDelegate->OnNodeDiscovered(nodeData);
            }
            else
            {
#if CHIP_MINMDNS_HIGH_VERBOSITY
                ChipLogError(Discovery, "No delegate to report commissioning node discovery");
#endif
            }
        }
        else if (resolver->IsActiveOperationalParse())
        {
            ResolvedNodeData nodeData;

            CHIP_ERROR err = resolver->Take(nodeData);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Discovery, "Failed to take discovery result: %" CHIP_ERROR_FORMAT, err.Format());
            }

            mActiveResolves.Complete(nodeData.operationalData.peerId);
            if (mOperationalDelegate != nullptr)
            {
                mOperationalDelegate->OnOperationalNodeResolved(nodeData);
            }
            else
            {
#if CHIP_MINMDNS_HIGH_VERBOSITY
                ChipLogError(Discovery, "No delegate to report operational node discovery");
#endif
            }
        }
        else
        {
            ChipLogError(Discovery, "Unexpected state: record type unknown");
            resolver->ResetToInactive();
        }
    }
}

void MinMdnsResolver::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    // Fill up any relevant data
    mPacketParser.ParseSrvRecords(data);
    mPacketParser.ParseNonSrvRecords(info->Interface, data);

    AdvancePendingResolverStates();

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

CHIP_ERROR MinMdnsResolver::BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::Browse & data,
                                       bool firstSend)
{
    mdns::Minimal::FullQName qname;

    switch (data.type)
    {
    case DiscoveryType::kOperational:
        qname = CheckAndAllocateQName(kOperationalServiceName, kOperationalProtocol, kLocalDomain);
        break;
    case DiscoveryType::kCommissionableNode:
        if (data.filter.type == DiscoveryFilterType::kNone)
        {
            qname = CheckAndAllocateQName(kCommissionableServiceName, kCommissionProtocol, kLocalDomain);
        }
        else if (data.filter.type == DiscoveryFilterType::kInstanceName)
        {
            qname = CheckAndAllocateQName(data.filter.instanceName, kCommissionableServiceName, kCommissionProtocol, kLocalDomain);
        }
        else
        {
            char subtypeStr[Common::kSubTypeMaxLength + 1];
            ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), data.filter));
            qname = CheckAndAllocateQName(subtypeStr, kSubtypeServiceNamePart, kCommissionableServiceName, kCommissionProtocol,
                                          kLocalDomain);
        }
        break;
    case DiscoveryType::kCommissionerNode:
        if (data.filter.type == DiscoveryFilterType::kNone)
        {
            qname = CheckAndAllocateQName(kCommissionerServiceName, kCommissionProtocol, kLocalDomain);
        }
        else
        {
            char subtypeStr[Common::kSubTypeMaxLength + 1];
            ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), data.filter));
            qname = CheckAndAllocateQName(subtypeStr, kSubtypeServiceNamePart, kCommissionerServiceName, kCommissionProtocol,
                                          kLocalDomain);
        }
        break;
    case DiscoveryType::kUnknown:
        break;
    }

    ReturnErrorCodeIf(!qname.nameCount, CHIP_ERROR_NO_MEMORY);

    mdns::Minimal::Query query(qname);
    query
        .SetClass(QClass::IN)           //
        .SetType(QType::ANY)            //
        .SetAnswerViaUnicast(firstSend) //
        ;

    mdns::Minimal::Logging::LogSendingQuery(query);
    builder.AddQuery(query);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::Resolve & data,
                                       bool firstSend)
{
    char nameBuffer[kMaxOperationalServiceNameSize] = "";

    // Node and fabricid are encoded in server names.
    ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), data.peerId));

    const char * instanceQName[] = { nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain };
    Query query(instanceQName);

    query
        .SetClass(QClass::IN)           //
        .SetType(QType::ANY)            //
        .SetAnswerViaUnicast(firstSend) //
        ;

    mdns::Minimal::Logging::LogSendingQuery(query);
    builder.AddQuery(query);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt::IpResolve & data,
                                       bool firstSend)
{

    Query query(data.hostName.Content());

    query
        .SetClass(QClass::IN)           //
        .SetType(QType::AAAA)           //
        .SetAnswerViaUnicast(firstSend) //
        ;

    mdns::Minimal::Logging::LogSendingQuery(query);
    builder.AddQuery(query);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::BuildQuery(QueryBuilder & builder, const ActiveResolveAttempts::ScheduledAttempt & attempt)
{
    if (attempt.IsResolve())
    {
        ReturnErrorOnFailure(BuildQuery(builder, attempt.ResolveData(), attempt.firstSend));
    }
    else if (attempt.IsBrowse())
    {
        ReturnErrorOnFailure(BuildQuery(builder, attempt.BrowseData(), attempt.firstSend));
    }
    else if (attempt.IsIpResolve())
    {
        ReturnErrorOnFailure(BuildQuery(builder, attempt.IpResolveData(), attempt.firstSend));
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorCodeIf(!builder.Ok(), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::SendAllPendingQueries()
{
    while (true)
    {
        Optional<ActiveResolveAttempts::ScheduledAttempt> resolve = mActiveResolves.NextScheduled();

        if (!resolve.HasValue())
        {
            break;
        }

        System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
        ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        QueryBuilder builder(std::move(buffer));
        builder.Header().SetMessageId(0);

        ReturnErrorOnFailure(BuildQuery(builder, resolve.Value()));

        if (resolve.Value().firstSend)
        {
            ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastUnicastQuery(builder.ReleasePacket(), kMdnsPort));
        }
        else
        {
            ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastSend(builder.ReleasePacket(), kMdnsPort));
        }
    }

    ExpireIncrementalResolvers();

    return ScheduleRetries();
}

void MinMdnsResolver::ExpireIncrementalResolvers()
{
    // once all queries are sent, if any SRV cannot receive AAAA addresses, expire it
    for (IncrementalResolver * resolver = mPacketParser.ResolverBegin(); resolver != mPacketParser.ResolverEnd(); resolver++)
    {
        if (!resolver->IsActive())
        {
            continue;
        }

        IncrementalResolver::RequiredInformationFlags missing = resolver->GetMissingRequiredInformation();
        if (missing.Has(IncrementalResolver::RequiredInformationBitFlags::kIpAddress))
        {
            if (mActiveResolves.IsWaitingForIpResolutionFor(resolver->GetTargetHostName()))
            {
                continue;
            }
        }

        // mark as expired: not waiting for anything
        resolver->ResetToInactive();
    }
}

CHIP_ERROR MinMdnsResolver::DiscoverCommissionableNodes(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionableNode, filter);
}

CHIP_ERROR MinMdnsResolver::DiscoverCommissioners(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionerNode, filter);
}

CHIP_ERROR MinMdnsResolver::BrowseNodes(DiscoveryType type, DiscoveryFilter filter)
{
    mActiveResolves.MarkPending(filter, type);

    return SendAllPendingQueries();
}

CHIP_ERROR MinMdnsResolver::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    mActiveResolves.MarkPending(peerId);

    return SendAllPendingQueries();
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

MinMdnsResolver gResolver;

} // namespace

Resolver & chip::Dnssd::Resolver::Instance()
{
    return gResolver;
}

ResolverProxy::~ResolverProxy()
{
    // TODO: this is a hack: resolver proxies used for commissionable discovery
    //       and they don't interact well with each other.
    gResolver.SetCommissioningDelegate(nullptr);
    Shutdown();
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

CHIP_ERROR ResolverProxy::DiscoverCommissionableNodes(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    chip::Dnssd::Resolver::Instance().SetCommissioningDelegate(mDelegate);
    return chip::Dnssd::Resolver::Instance().DiscoverCommissionableNodes(filter);
}

CHIP_ERROR ResolverProxy::DiscoverCommissioners(DiscoveryFilter filter)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    chip::Dnssd::Resolver::Instance().SetCommissioningDelegate(mDelegate);
    return chip::Dnssd::Resolver::Instance().DiscoverCommissioners(filter);
}

} // namespace Dnssd
} // namespace chip
