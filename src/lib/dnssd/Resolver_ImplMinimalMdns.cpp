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
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/TxtFields.h>
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

class PacketDataReporter : public ParserDelegate
{
public:
    PacketDataReporter(OperationalResolveDelegate * opDelegate, CommissioningResolveDelegate * commissionDelegate,
                       chip::Inet::InterfaceId interfaceId, DiscoveryType discoveryType, const BytesRange & packet) :
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

    /// Handle processing of a newly received IP address
    ///
    /// Will place the given [addr] into the address list of [resolutionData] assuming that
    /// there is enough space for that.
    void OnNodeIPAddress(CommonResolutionData & resolutionData, const chip::Inet::IPAddress & addr);
};

void PacketDataReporter::OnQuery(const QueryData & data)
{
    // Ignore queries:
    //   - unicast answers will include the corresponding query in the answer
    //     packet, however that is not interesting for the resolver.
}

void PacketDataReporter::OnHeader(ConstHeaderRef & header)
{
    mValid       = header.GetFlags().IsResponse();
    mHasIP       = false; // will need to get at least one valid IP eventually
    mHasNodePort = false; // also need node-port which we do not have yet

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
        Platform::CopyString(mNodeData.resolutionData.hostName, it.Value());
    }

    if (!name.Next())
    {
#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
        ChipLogError(Discovery, "mDNS packet is missing a valid server name");
#endif
        return;
    }

    if (ExtractIdFromInstanceName(name.Value(), &mNodeData.operationalData.peerId) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to parse peer id from %s", name.Value());
        return;
    }

    mNodeData.resolutionData.port = srv.GetPort();
    mHasNodePort                  = true;
}

void PacketDataReporter::OnCommissionableNodeSrvRecord(SerializedQNameIterator name, const SrvRecord & srv)
{
    // Host name is the first part of the qname
    mdns::Minimal::SerializedQNameIterator it = srv.GetName();
    if (it.Next())
    {
        Platform::CopyString(mDiscoveredNodeData.resolutionData.hostName, it.Value());
    }
    if (name.Next())
    {
        strncpy(mDiscoveredNodeData.commissionData.instanceName, name.Value(), sizeof(CommissionNodeData::instanceName));
    }
    mDiscoveredNodeData.resolutionData.port = srv.GetPort();
    mHasNodePort                            = true;
}

void PacketDataReporter::OnNodeIPAddress(CommonResolutionData & resolutionData, const chip::Inet::IPAddress & addr)
{
    // TODO: should validate that the IP address we receive belongs to the
    // server associated with the SRV record.
    //
    // This code assumes that all entries in the mDNS packet relate to the
    // same entity. This may not be correct if multiple servers are reported
    // (if multi-admin decides to use unique ports for every ecosystem).
    if (resolutionData.numIPs >= CommonResolutionData::kMaxIPAddresses)
    {
        ChipLogDetail(Discovery, "Number of IP addresses overflow. Discarding extra addresses.");
        return;
    }
    resolutionData.ipAddress[resolutionData.numIPs++] = addr;
    resolutionData.interfaceId                        = mInterfaceId;
    mHasIP                                            = true;
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
        }
        else if (mDiscoveryType == DiscoveryType::kOperational)
        {
            // Ensure this is our record.
            // TODO: Fix this comparison which is too loose.
            if (HasQNamePart(data.GetName(), kOperationalServiceName))
            {
                OnOperationalSrvRecord(data.GetName(), srv);
            }
            else
            {
                ChipLogError(Discovery, "Invalid operational srv name: no '%s' part found.", kOperationalServiceName);
            }
        }
        else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            // TODO: Fix this comparison which is too loose.
            if (HasQNamePart(data.GetName(), kCommissionableServiceName) || HasQNamePart(data.GetName(), kCommissionerServiceName))
            {
                OnCommissionableNodeSrvRecord(data.GetName(), srv);
            }
            else
            {
                ChipLogError(Discovery, "Invalid commision srv name: no '%s' or '%s' part found.", kCommissionableServiceName,
                             kCommissionerServiceName);
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
                strncpy(mDiscoveredNodeData.commissionData.instanceName, qname.Value(), sizeof(CommissionNodeData::instanceName));
            }
        }
        break;
    }
    case QType::TXT:
        if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            TxtRecordDelegateImpl<CommonResolutionData> commonDelegate(mDiscoveredNodeData.resolutionData);
            ParseTxtRecord(data.GetData(), &commonDelegate);

            TxtRecordDelegateImpl<CommissionNodeData> commissionDelegate(mDiscoveredNodeData.commissionData);
            ParseTxtRecord(data.GetData(), &commissionDelegate);
        }
        else if (mDiscoveryType == DiscoveryType::kOperational)
        {
            TxtRecordDelegateImpl<CommonResolutionData> commonDelegate(mNodeData.resolutionData);
            ParseTxtRecord(data.GetData(), &commonDelegate);
        }
        break;
    case QType::A: {
        Inet::IPAddress addr;
        if (!ParseARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse A record");
        }
        else
        {
            if (mDiscoveryType == DiscoveryType::kOperational)
            {
                OnNodeIPAddress(mNodeData.resolutionData, addr);
            }
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
            {
                OnNodeIPAddress(mDiscoveredNodeData.resolutionData, addr);
            }
        }
        break;
    }
    case QType::AAAA: {
        Inet::IPAddress addr;
        if (!ParseAAAARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse AAAA record");
        }
        else
        {
            if (mDiscoveryType == DiscoveryType::kOperational)
            {
                OnNodeIPAddress(mNodeData.resolutionData, addr);
            }
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
            {
                OnNodeIPAddress(mDiscoveredNodeData.resolutionData, addr);
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
    if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
    {
        if (!mDiscoveredNodeData.resolutionData.IsValid())
        {
            ChipLogError(Discovery, "Discovered node data is not valid. Commissioning discovery not complete.");
            return;
        }

        activeAttempts.Complete(mDiscoveredNodeData);
        if (mCommissioningDelegate != nullptr)
        {
            mCommissioningDelegate->OnNodeDiscovered(mDiscoveredNodeData);
        }
        else
        {
            ChipLogError(Discovery, "No delegate to report commissioning node discovery");
        }
    }
    else if (mDiscoveryType == DiscoveryType::kOperational)
    {
        if (!mHasIP)
        {
            ChipLogError(Discovery, "Operational discovery has no valid ip address. Resolve not complete.");
            return;
        }

        if (!mHasNodePort)
        {
            ChipLogError(Discovery, "Operational discovery has no valid node/port. Resolve not complete.");
            return;
        }

        activeAttempts.Complete(mNodeData.operationalData.peerId);
        mNodeData.LogNodeIdResolved();

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

private:
    OperationalResolveDelegate * mOperationalDelegate     = nullptr;
    CommissioningResolveDelegate * mCommissioningDelegate = nullptr;
    DiscoveryType mDiscoveryType                          = DiscoveryType::kUnknown;
    System::Layer * mSystemLayer                          = nullptr;
    ActiveResolveAttempts mActiveResolves;

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
    if ((mOperationalDelegate == nullptr) && (mCommissioningDelegate == nullptr))
    {
        return;
    }

    PacketDataReporter reporter(mOperationalDelegate, mCommissioningDelegate, info->Interface, mDiscoveryType, data);

    if (!ParsePacket(data, &reporter))
    {
        ChipLogError(Discovery, "Failed to parse received mDNS packet");
    }
    else
    {
        reporter.OnComplete(mActiveResolves);
        ScheduleRetries();
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
