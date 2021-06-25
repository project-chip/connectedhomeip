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

#include "MinimalMdnsServer.h"
#include "ServiceNaming.h"

#include <mdns/TxtFields.h>
#include <mdns/minimal/Parser.h>
#include <mdns/minimal/QueryBuilder.h>
#include <mdns/minimal/RecordData.h>
#include <mdns/minimal/core/FlatAllocatedQName.h>

#include <support/logging/CHIPLogging.h>

// MDNS servers will receive all broadcast packets over the network.
// Disable 'invalid packet' messages because the are expected and common
// These logs are useful for debug only
#undef MINMDNS_RESOLVER_OVERLY_VERBOSE

namespace chip {
namespace Mdns {
namespace {

enum class DiscoveryType
{
    kUnknown,
    kOperational,
    kCommissionableNode,
    kCommissionerNode
};

class TxtRecordDelegateImpl : public mdns::Minimal::TxtRecordDelegate
{
public:
    TxtRecordDelegateImpl(DiscoveredNodeData * nodeData) : mNodeData(nodeData) {}
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value);

private:
    DiscoveredNodeData * mNodeData;
};

const ByteSpan GetSpan(const mdns::Minimal::BytesRange & range)
{
    return ByteSpan(range.Start(), range.Size());
}

void TxtRecordDelegateImpl::OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
{
    if (mNodeData == nullptr)
    {
        return;
    }
    ByteSpan key = GetSpan(name);
    ByteSpan val = GetSpan(value);
    FillNodeDataFromTxt(key, val, mNodeData);
}

constexpr size_t kMdnsMaxPacketSize = 1024;
constexpr uint16_t kMdnsPort        = 5353;

using namespace mdns::Minimal;

class PacketDataReporter : public ParserDelegate
{
public:
    PacketDataReporter(ResolverDelegate * delegate, chip::Inet::InterfaceId interfaceId, DiscoveryType discoveryType,
                       const BytesRange & packet) :
        mDelegate(delegate),
        mDiscoveryType(discoveryType), mPacketRange(packet)
    {
        mNodeData.mInterfaceId = interfaceId;
    }

    // ParserDelegate implementation

    void OnHeader(ConstHeaderRef & header) override;
    void OnQuery(const QueryData & data) override;
    void OnResource(ResourceType type, const ResourceData & data) override;
    // Called after ParsePacket is complete to send final notifications to the delegate.
    // Used to ensure all the available IP addresses are attached before completion.
    void OnComplete();

private:
    ResolverDelegate * mDelegate = nullptr;
    DiscoveryType mDiscoveryType;
    ResolvedNodeData mNodeData;
    DiscoveredNodeData mDiscoveredNodeData;
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
    ChipLogError(Discovery, "Unexpected query packet being parsed as a response");
    mValid = false;
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
    if (!it.Next())
    {
        return;
    }
    strncpy(mDiscoveredNodeData.hostName, it.Value(), sizeof(DiscoveredNodeData::hostName));
}

void PacketDataReporter::OnOperationalIPAddress(const chip::Inet::IPAddress & addr)
{
    // TODO: should validate that the IP address we receive belongs to the
    // server associated with the SRV record.
    //
    // This code assumes that all entries in the mDNS packet relate to the
    // same entity. This may not be correct if multiple servers are reported
    // (if multi-admin decides to use unique ports for every ecosystem).
    mNodeData.mAddress = addr;
    mHasIP             = true;
}

void PacketDataReporter::OnDiscoveredNodeIPAddress(const chip::Inet::IPAddress & addr)
{
    if (mDiscoveredNodeData.numIPs >= DiscoveredNodeData::kMaxIPAddresses)
    {
        return;
    }
    mDiscoveredNodeData.ipAddress[mDiscoveredNodeData.numIPs++] = addr;
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
            if (HasQNamePart(data.GetName(), kOperationalServiceName))
            {
                OnOperationalSrvRecord(data.GetName(), srv);
            }
            else
            {
                mValid = false;
            }
        }
        else if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            if (HasQNamePart(data.GetName(), kCommissionableServiceName) || HasQNamePart(data.GetName(), kCommissionerServiceName))
            {
                OnCommissionableNodeSrvRecord(data.GetName(), srv);
            }
            else
            {
                mValid = false;
            }
        }
        break;
    }
    case QType::TXT:
        if (mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode)
        {
            TxtRecordDelegateImpl textRecordDelegate(&mDiscoveredNodeData);
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

void PacketDataReporter::OnComplete()
{
    if ((mDiscoveryType == DiscoveryType::kCommissionableNode || mDiscoveryType == DiscoveryType::kCommissionerNode) &&
        mDiscoveredNodeData.IsValid())
    {
        mDelegate->OnNodeDiscoveryComplete(mDiscoveredNodeData);
    }
    else if (mDiscoveryType == DiscoveryType::kOperational && mHasIP && mHasNodePort)
    {
        mDelegate->OnNodeIdResolved(mNodeData);
    }
}

class MinMdnsResolver : public Resolver, public MdnsPacketDelegate
{
public:
    MinMdnsResolver() { GlobalMinimalMdnsServer::Instance().SetResponseDelegate(this); }

    //// MdnsPacketDelegate implementation
    void OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info) override;

    ///// Resolver implementation
    CHIP_ERROR StartResolver(chip::Inet::InetLayer * inetLayer, uint16_t port) override;
    CHIP_ERROR SetResolverDelegate(ResolverDelegate * delegate) override;
    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) override;
    CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override;
    CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override;

private:
    ResolverDelegate * mDelegate = nullptr;
    DiscoveryType mDiscoveryType = DiscoveryType::kUnknown;

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
};

void MinMdnsResolver::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    if (mDelegate == nullptr)
    {
        return;
    }

    PacketDataReporter reporter(mDelegate, info->Interface, mDiscoveryType, data);

    if (!ParsePacket(data, &reporter))
    {
        ChipLogError(Discovery, "Failed to parse received mDNS packet");
    }
    else
    {
        reporter.OnComplete();
    }
}

CHIP_ERROR MinMdnsResolver::StartResolver(chip::Inet::InetLayer * inetLayer, uint16_t port)
{
    /// Note: we do not double-check the port as we assume the APP will always use
    /// the same inetLayer and port for mDNS.
    if (GlobalMinimalMdnsServer::Server().IsListening())
    {
        return CHIP_NO_ERROR;
    }

    return GlobalMinimalMdnsServer::Instance().StartServer(inetLayer, port);
}

CHIP_ERROR MinMdnsResolver::SetResolverDelegate(ResolverDelegate * delegate)
{
    mDelegate = delegate;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MinMdnsResolver::SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type)
{

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    QueryBuilder builder(std::move(buffer));
    builder.Header().SetMessageId(0);

    mdns::Minimal::Query query(qname);
    query.SetType(type).SetClass(mdns::Minimal::QClass::IN);
    // TODO(cecille): Not sure why unicast response isn't working - fix.
    query.SetAnswerViaUnicast(false);

    builder.AddQuery(query);

    ReturnErrorCodeIf(!builder.Ok(), CHIP_ERROR_INTERNAL);

    return GlobalMinimalMdnsServer::Server().BroadcastSend(builder.ReleasePacket(), kMdnsPort);
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
        else
        {
            char subtypeStr[kMaxSubtypeDescSize];
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
            char subtypeStr[kMaxSubtypeDescSize];
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
    mDiscoveryType                    = DiscoveryType::kOperational;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    QueryBuilder builder(std::move(buffer));
    builder.Header().SetMessageId(0);

    {
        char nameBuffer[64] = "";

        // Node and fabricid are encoded in server names.
        ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), peerId));

        const char * instanceQName[] = { nameBuffer, kOperationalServiceName, kOperationalProtocol, kLocalDomain };
        Query query(instanceQName);

        query
            .SetClass(QClass::IN)       //
            .SetType(QType::ANY)        //
            .SetAnswerViaUnicast(false) //
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

    return GlobalMinimalMdnsServer::Server().BroadcastSend(builder.ReleasePacket(), kMdnsPort);
}

MinMdnsResolver gResolver;

} // namespace

Resolver & chip::Mdns::Resolver::Instance()
{
    return gResolver;
}

} // namespace Mdns
} // namespace chip
