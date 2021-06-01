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
};

class TxtRecordDelegateImpl : public mdns::Minimal::TxtRecordDelegate
{
public:
    TxtRecordDelegateImpl(CommissionableNodeData * nodeData) : mNodeData(nodeData) {}
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value);

private:
    CommissionableNodeData * mNodeData;
};

bool IsKey(const mdns::Minimal::BytesRange key, const char * desired)
{
    if (key.Size() != strlen(desired))
    {
        return false;
    }
    return memcmp(key.Start(), desired, key.Size()) == 0;
}

uint16_t MakeU16(const mdns::Minimal::BytesRange & val)
{
    uint32_t u32          = 0;
    const uint16_t errval = 0x0;
    for (size_t i = 0; i < val.Size(); ++i)
    {
        char c = static_cast<char>(val.Start()[i]);
        if (c < '0' || c > '9')
        {
            return errval;
        }
        u32 = u32 * 10;
        u32 += val.Start()[i] - static_cast<uint8_t>('0');
        if (u32 > std::numeric_limits<uint16_t>::max())
        {
            return errval;
        }
    }
    return static_cast<uint16_t>(u32);
}

void TxtRecordDelegateImpl::OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
{
    if (mNodeData == nullptr)
    {
        return;
    }

    if (IsKey(name, "D"))
    {
        mNodeData->longDiscriminator = MakeU16(value);
    }
    else if (IsKey(name, "VP"))
    {
        // Fist value is the vendor id, second (after the +) is the product.
        size_t plussign = value.Size();
        for (size_t i = 0; i < value.Size(); ++i)
        {
            if (static_cast<char>(value.Start()[i]) == '+')
            {
                plussign = i;
                break;
            }
        }
        if (plussign > 0)
        {
            mNodeData->vendorId = MakeU16(mdns::Minimal::BytesRange(value.Start(), value.Start() + plussign));
        }
        if (plussign < value.Size() - 1)
        {
            mNodeData->productId = MakeU16(mdns::Minimal::BytesRange(value.Start() + plussign + 1, value.End()));
        }
    }
    // TODO(cecille): Add the new stuff from 0.7 ballot 2.
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
    CommissionableNodeData mCommissionableNodeData;
    BytesRange mPacketRange;

    bool mValid       = false;
    bool mHasNodePort = false;
    bool mHasIP       = false;

    void OnCommissionableNodeSrvRecord(SerializedQNameIterator name, const SrvRecord & srv);
    void OnOperationalSrvRecord(SerializedQNameIterator name, const SrvRecord & srv);

    void OnCommissionableNodeIPAddress(const chip::Inet::IPAddress & addr);
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

    // Before attempting to parse hex values for node/fabrid, validate
    // that he response is indeed from a chip tcp service.
    {
        SerializedQNameIterator suffix = name;

        constexpr const char * kExpectedSuffix[] = { "_chip", "_tcp", "local" };

        if (suffix != FullQName(kExpectedSuffix))
        {
#ifdef MINMDNS_RESOLVER_OVERLY_VERBOSE
            ChipLogError(Discovery, "mDNS packet is not for a CHIP device");
#endif
            mHasNodePort = false;
            return;
        }
    }

    if (ExtractIdFromInstanceName(name.Value(), &mNodeData.mPeerId) != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to parse peer id from %s", name.Value());
        mHasNodePort = false;
        return;
    }

    mNodeData.mPort = srv.GetPort();
    mHasNodePort    = true;

    if (mHasIP)
    {
        mDelegate->OnNodeIdResolved(mNodeData);
    }
}

void PacketDataReporter::OnCommissionableNodeSrvRecord(SerializedQNameIterator name, const SrvRecord & srv)
{
    // Host name is the first part of the qname
    mdns::Minimal::SerializedQNameIterator it = srv.GetName();
    if (!it.Next())
    {
        return;
    }
    strncpy(mCommissionableNodeData.hostName, it.Value(), sizeof(CommissionableNodeData::hostName));
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

    if (mHasNodePort)
    {
        mDelegate->OnNodeIdResolved(mNodeData);
    }
}

void PacketDataReporter::OnCommissionableNodeIPAddress(const chip::Inet::IPAddress & addr)
{
    if (mCommissionableNodeData.numIPs >= CommissionableNodeData::kMaxIPAddresses)
    {
        return;
    }
    mCommissionableNodeData.ipAddress[mCommissionableNodeData.numIPs++] = addr;
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
            OnOperationalSrvRecord(data.GetName(), srv);
        }
        else if (mDiscoveryType == DiscoveryType::kCommissionableNode)
        {
            OnCommissionableNodeSrvRecord(data.GetName(), srv);
        }
        break;
    }
    case QType::TXT:
        if (mDiscoveryType == DiscoveryType::kCommissionableNode)
        {
            TxtRecordDelegateImpl textRecordDelegate(&mCommissionableNodeData);
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
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode)
            {
                OnCommissionableNodeIPAddress(addr);
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
            else if (mDiscoveryType == DiscoveryType::kCommissionableNode)
            {
                OnCommissionableNodeIPAddress(addr);
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
    if (mDiscoveryType == DiscoveryType::kCommissionableNode && mCommissionableNodeData.IsValid())
    {
        mDelegate->OnCommissionableNodeFound(mCommissionableNodeData);
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
