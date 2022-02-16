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

#include "DnssdCache.h"
#include "Resolver.h"

#include <limits>

#include <lib/core/CHIPConfig.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/dnssd/minimal_mdns/ActiveResolveAttempts.h>
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

const ByteSpan GetSpan(const mdns::Minimal::BytesRange & range)
{
    return ByteSpan(range.Start(), range.Size());
}

template <class NodeData>
class TxtRecordDelegateImpl : public mdns::Minimal::TxtRecordDelegate
{
public:
    explicit TxtRecordDelegateImpl(NodeData & nodeData) : mNodeData(nodeData) {}
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override
    {
        FillNodeDataFromTxt(GetSpan(name), GetSpan(value), mNodeData);
    }

private:
    NodeData & mNodeData;
};

constexpr size_t kMdnsMaxPacketSize = 1024;
constexpr uint16_t kMdnsPort        = 5353;

using namespace mdns::Minimal;
using DnssdCacheType = Dnssd::DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE>;

class PacketDataReporter : public ParserDelegate
{
public:
    PacketDataReporter(OperationalResolveDelegate * opDelegate, CommissioningResolveDelegate * commissionDelegate,
                       chip::Inet::InterfaceId interfaceId, DiscoveryType discoveryType, const BytesRange & packet,
                       DnssdCacheType & mdnsCache) :
        mOperationalDelegate(opDelegate),
        mCommissioningDelegate(commissionDelegate), mDiscoveryType(discoveryType), mPacketRange(packet)
    {
        mInterfaceId = interfaceId;
    }

    // ParserDelegate implementation

    void OnHeader(ConstHeaderRef & header) override;
    void OnQuery(const QueryData & data) override;
    void OnResource(ResourceType type, const ResourceData & data) override;

    // Called after ParsePacket is complete to send final notifications to the delegate.
    // Used to ensure all the available IP addresses are attached before completion.
    void OnComplete(ActiveResolveAttempts & activeAttempts);

private:
    OperationalResolveDelegate * mOperationalDelegate;
    CommissioningResolveDelegate * mCommissioningDelegate;
    DiscoveryType mDiscoveryType;
    ResolvedNodeData mNodeData;
    DiscoveredNodeData mDiscoveredNodeData;
    chip::Inet::InterfaceId mInterfaceId;
    BytesRange mPacketRange;

    bool mValid       = false;
    bool mHasNodePort = false;
    bool mHasIP       = false;

    void OnCommissionableNodeSrvRecord(SerializedQNameIterator name, const SrvRecord & srv);
    void OnOperationalSrvRecord(SerializedQNameIterator name, const SrvRecord & srv);

    void OnDiscoveredNodeIPAddress(const chip::Inet::IPAddress & addr);
    void OnOperationalIPAddress(const chip::Inet::IPAddress & addr);
};

void PacketDataReporter::OnQuery(const QueryData & data)
{
    // Ignore queries:
    //   - unicast answers will include the corresponding query in the answer
    //     packet, however that is not interesting for the resolver.
}

void PacketDataReporter::OnHeader(ConstHeaderRef & header)
{
    mValid = header.GetFlags().IsResponse();

    if (header.GetFlags().IsTruncated())
    {
#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
        // MinMdns does not cache data, so receiving piecewise data does not work
        ChipLogError(Discovery, "Truncated responses not supported for address resolution");
#endif
    }
}

void PacketDataReporter::OnOperationalSrvRecord(SerializedQNameIterator name, const SrvRecord & srv)
{
    mdns::Minimal::SerializedQNameIterator it = srv.GetName();
    if (it.Next())
    {
        Platform::CopyString(mNodeData.mHostName, it.Value());
    }

    if (!name.Next())
    {
#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
        ChipLogError(Discovery, "mDNS packet is missing a valid server name");
#endif
        mHasNodePort = false;
        return;
    }

    if (ExtractIdFromInstanceName(name.Value(), &mNodeData.mPeerId) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to parse peer id from %s", name.Value());
        mHasNodePort = false;
        return;
    }

    mNodeData.mPort = srv.GetPort();
    mHasNodePort    = true;
}

void PacketDataReporter::OnCommissionableNodeSrvRecord(SerializedQNameIterator name, const SrvRecord & srv)
{
    // Host name is the first part of the qname
    mdns::Minimal::SerializedQNameIterator it = srv.GetName();
    if (it.Next())
    {
        Platform::CopyString(mDiscoveredNodeData.hostName, it.Value());
    }
    if (name.Next())
    {
        strncpy(mDiscoveredNodeData.instanceName, name.Value(), sizeof(DiscoveredNodeData::instanceName));
    }
    mDiscoveredNodeData.port = srv.GetPort();
}

void PacketDataReporter::OnOperationalIPAddress(const chip::Inet::IPAddress & addr)
{
    // TODO: should validate that the IP address we receive belongs to the
    // server associated with the SRV record.
    //
    // This code assumes that all entries in the mDNS packet relate to the
    // same entity. This may not be correct if multiple servers are reported
    // (if multi-admin decides to use unique ports for every ecosystem).
    if (mNodeData.mNumIPs >= ResolvedNodeData::kMaxIPAddresses)
    {
        return;
    }
    mNodeData.mAddress[mNodeData.mNumIPs++] = addr;
    mNodeData.mInterfaceId                  = mInterfaceId;
    mHasIP                                  = true;
}

void PacketDataReporter::OnDiscoveredNodeIPAddress(const chip::Inet::IPAddress & addr)
{
    if (mDiscoveredNodeData.numIPs >= DiscoveredNodeData::kMaxIPAddresses)
    {
        return;
    }
    mDiscoveredNodeData.ipAddress[mDiscoveredNodeData.numIPs] = addr;
    mDiscoveredNodeData.interfaceId                           = mInterfaceId;
    mDiscoveredNodeData.numIPs++;
}

bool HasQNamePart(SerializedQNameIterator qname, QNamePart part)
{
    while (qname.Next())
    {
        if (strcmp(qname.Value(), part) == 0)
        {
            return true;
        }
    }
    return false;
}

void PacketDataReporter::OnResource(ResourceType type, const ResourceData & data)
{
    if (!mValid)
    {
        return;
    }

    /// Data content is expected to contain:
    /// - A SRV entry that includes the node ID in expected format (fabric + nodeid)
    ///    - Can extract: fabricid, nodeid, port
    ///    - References ServerName
    /// - Additional records tied to ServerName contain A/AAAA records for IP address data
    switch (data.GetType())
    {
    case QType::SRV: {
        SrvRecord srv;
        if (!srv.Parse(data.GetData(), mPacketRange))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse SRV record");
            mHasNodePort = false;
        }
        else if (mDiscoveryType == DiscoveryType::kOperational)
        {
            // Ensure this is our record.
            // TODO: Fix this comparison which is too loose.
            if (HasQNamePart(data.GetName(), kOperationalServiceName))
            {
                OnOperationalSrvRecord(data.GetName(), srv);
            }
        }
        else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            // TODO: Fix this comparison which is too loose.
            if (HasQNamePart(data.GetName(), kCommissionableServiceName) || HasQNamePart(data.GetName(), kCommissionerServiceName))
            {
                OnCommissionableNodeSrvRecord(data.GetName(), srv);
            }
        }
        break;
    }
    case QType::PTR: {
        if (mDiscoveryType == DiscoveryType::kCommissionableNode)
        {
            SerializedQNameIterator qname;
            ParsePtrRecord(data.GetData(), mPacketRange, &qname);
            if (qname.Next())
            {
                strncpy(mDiscoveredNodeData.instanceName, qname.Value(), sizeof(DiscoveredNodeData::instanceName));
            }
        }
        break;
    }
    case QType::TXT:
        if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            TxtRecordDelegateImpl<DiscoveredNodeData> textRecordDelegate(mDiscoveredNodeData);
            ParseTxtRecord(data.GetData(), &textRecordDelegate);
        }
        else if (mDiscoveryType == DiscoveryType::kOperational)
        {
            TxtRecordDelegateImpl<ResolvedNodeData> textRecordDelegate(mNodeData);
            ParseTxtRecord(data.GetData(), &textRecordDelegate);
        }
        break;
    case QType::A: {
        Inet::IPAddress addr;
        if (!ParseARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse A record");
            mHasIP = false;
        }
        else
        {
            if (mDiscoveryType == DiscoveryType::kOperational)
            {
                OnOperationalIPAddress(addr);
            }
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
            {
                OnDiscoveredNodeIPAddress(addr);
            }
        }
        break;
    }
    case QType::AAAA: {
        Inet::IPAddress addr;
        if (!ParseAAAARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse AAAA record");
            mHasIP = false;
        }
        else
        {
            if (mDiscoveryType == DiscoveryType::kOperational)
            {
                OnOperationalIPAddress(addr);
            }
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
            {
                OnDiscoveredNodeIPAddress(addr);
            }
        }
        break;
    }
    default:
        break;
    }
}

void PacketDataReporter::OnComplete(ActiveResolveAttempts & activeAttempts)
{
    if ((mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode) &&
        mDiscoveredNodeData.IsValid())
    {
        if (mCommissioningDelegate != nullptr)
        {
            mCommissioningDelegate->OnNodeDiscovered(mDiscoveredNodeData);
        }
        else
        {
            ChipLogError(Discovery, "No delegate to report commissioning node discovery");
        }
    }
    else if (mDiscoveryType == DiscoveryType::kOperational && mHasIP && mHasNodePort)
    {
        activeAttempts.Complete(mNodeData.mPeerId);
        mNodeData.LogNodeIdResolved();
        mNodeData.PrioritizeAddresses();

        if (mOperationalDelegate != nullptr)
        {
            mOperationalDelegate->OnOperationalNodeResolved(mNodeData);
        }
        else
        {
            ChipLogError(Discovery, "No delegate to report operational node discovery");
        }
    }
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
    bool ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type) override;

private:
    OperationalResolveDelegate * mOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mCommissioningDelegate = nullptr;
    DiscoveryType mDiscoveryType                          = DiscoveryType::kUnknown;
    System::Layer * mSystemLayer                          = nullptr;
    ActiveResolveAttempts mActiveResolves;

    CHIP_ERROR SendPendingResolveQueries();
    CHIP_ERROR ScheduleResolveRetries();

    static void ResolveRetryCallback(System::Layer *, void * self);

    CHIP_ERROR SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type);
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
    // should this be static?
    // original version had:    static Dnssd::IPCache<CHIP_CONFIG_IPCACHE_SIZE, CHIP_CONFIG_TTL_MS> sIPCache;
    DnssdCacheType sDnssdCache;
};

void MinMdnsResolver::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    if ((mOperationalDelegate == nullptr) && (mCommissioningDelegate == nullptr))
    {
        return;
    }

    PacketDataReporter reporter(mOperationalDelegate, mCommissioningDelegate, info->Interface, mDiscoveryType, data, sDnssdCache);

    if (!ParsePacket(data, &reporter))
    {
        ChipLogError(Discovery, "Failed to parse received mDNS packet");
    }
    else
    {
        reporter.OnComplete(mActiveResolves);
        ScheduleResolveRetries();
    }
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

CHIP_ERROR MinMdnsResolver::SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type)
{
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    QueryBuilder builder(std::move(buffer));
    builder.Header().SetMessageId(0);

    mdns::Minimal::Query query(qname);
    query.SetType(type).SetClass(mdns::Minimal::QClass::IN);
    query.SetAnswerViaUnicast(true);

    builder.AddQuery(query);

    ReturnErrorCodeIf(!builder.Ok(), CHIP_ERROR_INTERNAL);

    return GlobalMinimalMdnsServer::Server().BroadcastUnicastQuery(builder.ReleasePacket(), kMdnsPort);
}

CHIP_ERROR MinMdnsResolver::FindCommissionableNodes(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionableNode, filter);
}

CHIP_ERROR MinMdnsResolver::FindCommissioners(DiscoveryFilter filter)
{
    return BrowseNodes(DiscoveryType::kCommissionerNode, filter);
}

// TODO(cecille): Extend filter and use this for Resolve
CHIP_ERROR MinMdnsResolver::BrowseNodes(DiscoveryType type, DiscoveryFilter filter)
{
    mDiscoveryType = type;

    mdns::Minimal::FullQName qname;

    switch (type)
    {
    case DiscoveryType::kOperational:
        qname = CheckAndAllocateQName(kOperationalServiceName, kOperationalProtocol, kLocalDomain);
        break;
    case DiscoveryType::kCommissionableNode:
        if (filter.type == DiscoveryFilterType::kNone)
        {
            qname = CheckAndAllocateQName(kCommissionableServiceName, kCommissionProtocol, kLocalDomain);
        }
        else if (filter.type == DiscoveryFilterType::kInstanceName)
        {
            qname = CheckAndAllocateQName(filter.instanceName, kCommissionableServiceName, kCommissionProtocol, kLocalDomain);
        }
        else
        {
            char subtypeStr[Common::kSubTypeMaxLength + 1];
            ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), filter));
            qname = CheckAndAllocateQName(subtypeStr, kSubtypeServiceNamePart, kCommissionableServiceName, kCommissionProtocol,
                                          kLocalDomain);
        }
        break;
    case DiscoveryType::kCommissionerNode:
        if (filter.type == DiscoveryFilterType::kNone)
        {
            qname = CheckAndAllocateQName(kCommissionerServiceName, kCommissionProtocol, kLocalDomain);
        }
        else
        {
            char subtypeStr[Common::kSubTypeMaxLength + 1];
            ReturnErrorOnFailure(MakeServiceSubtype(subtypeStr, sizeof(subtypeStr), filter));
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

    return SendQuery(qname, mdns::Minimal::QType::ANY);
}

CHIP_ERROR MinMdnsResolver::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    mDiscoveryType = DiscoveryType::kOperational;
    mActiveResolves.MarkPending(peerId);

    return SendPendingResolveQueries();
}

bool MinMdnsResolver::ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type)
{
    // MinMDNS does not do cache node address resolutions.
    return false;
}

CHIP_ERROR MinMdnsResolver::ScheduleResolveRetries()
{
    ReturnErrorCodeIf(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mSystemLayer->CancelTimer(&ResolveRetryCallback, this);

    Optional<System::Clock::Timeout> delay = mActiveResolves.GetTimeUntilNextExpectedResponse();

    if (!delay.HasValue())
    {
        return CHIP_NO_ERROR;
    }

    return mSystemLayer->StartTimer(delay.Value(), &ResolveRetryCallback, this);
}

void MinMdnsResolver::ResolveRetryCallback(System::Layer *, void * self)
{
    reinterpret_cast<MinMdnsResolver *>(self)->SendPendingResolveQueries();
}

CHIP_ERROR MinMdnsResolver::SendPendingResolveQueries()
{
    while (true)
    {
        Optional<PeerId> peerId = mActiveResolves.NextScheduledPeer();

        if (!peerId.HasValue())
        {
            break;
        }

        System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
        ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

        QueryBuilder builder(std::move(buffer));
        builder.Header().SetMessageId(0);

        {
            char nameBuffer[kMaxOperationalServiceNameSize] = "";

            // Node and fabricid are encoded in server names.
            ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), peerId.Value()));

            const char * instanceQName[] = { nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain };
            Query query(instanceQName);

            query
                .SetClass(QClass::IN)      //
                .SetType(QType::ANY)       //
                .SetAnswerViaUnicast(true) //
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

        ReturnErrorOnFailure(GlobalMinimalMdnsServer::Server().BroadcastUnicastQuery(builder.ReleasePacket(), kMdnsPort));
    }

    return ScheduleResolveRetries();
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

bool ResolverProxy::ResolveNodeIdFromInternalCache(const PeerId & peerId, Inet::IPAddressType type)
{
    return chip::Dnssd::Resolver::Instance().ResolveNodeIdFromInternalCache(peerId, type);
}

} // namespace Dnssd
} // namespace chip
