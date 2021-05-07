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

#include "MinimalMdnsServer.h"
#include "ServiceNaming.h"

#include <mdns/minimal/Parser.h>
#include <mdns/minimal/QueryBuilder.h>
#include <mdns/minimal/RecordData.h>

#include <support/logging/CHIPLogging.h>

// MDNS servers will receive all broadcast packets over the network.
// Disable 'invalid packet' messages because the are expected and common
// These logs are useful for debug only
#undef MINMDNS_RESOLVER_OVERLY_VERBOSE

namespace chip {
namespace Mdns {
namespace {

constexpr size_t kMdnsMaxPacketSize = 1024;
constexpr uint16_t kMdnsPort        = 5353;

using namespace mdns::Minimal;

class PacketDataReporter : public ParserDelegate
{
public:
    PacketDataReporter(ResolverDelegate * delegate, chip::Inet::InterfaceId interfaceId, const BytesRange & packet) :
        mDelegate(delegate), mPacketRange(packet)
    {
        mNodeData.mInterfaceId = interfaceId;
    }

    // ParserDelegate implementation

    void OnHeader(ConstHeaderRef & header) override;
    void OnQuery(const QueryData & data) override;
    void OnResource(ResourceType type, const ResourceData & data) override;

private:
    ResolverDelegate * mDelegate = nullptr;
    ResolvedNodeData mNodeData;
    BytesRange mPacketRange;

    bool mValid       = false;
    bool mHasNodePort = false;
    bool mHasIP       = false;

    void OnSrvRecord(SerializedQNameIterator name, const SrvRecord & srv);
    void OnIPAddress(const chip::Inet::IPAddress & addr);
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

void PacketDataReporter::OnSrvRecord(SerializedQNameIterator name, const SrvRecord & srv)
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

void PacketDataReporter::OnIPAddress(const chip::Inet::IPAddress & addr)
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

    if (data.GetType() == QType::SRV)
    {
        SrvRecord srv;

        if (!srv.Parse(data.GetData(), mPacketRange))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse SRV record");
            mHasNodePort = false;
        }
        else
        {
            OnSrvRecord(data.GetName(), srv);
        }
    }
    else if (data.GetType() == QType::A)
    {
        chip::Inet::IPAddress addr;

        if (!ParseARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse A record");
            mHasIP = false;
        }
        else
        {
            OnIPAddress(addr);
        }
    }
    else if (data.GetType() == QType::AAAA)
    {
        chip::Inet::IPAddress addr;

        if (!ParseAAAARecord(data.GetData(), &addr))
        {
            ChipLogError(Discovery, "Packet data reporter failed to parse A record");
            mHasIP = false;
        }
        else
        {
            OnIPAddress(addr);
        }
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

private:
    ResolverDelegate * mDelegate = nullptr;
};

void MinMdnsResolver::OnMdnsPacketData(const BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    if (mDelegate == nullptr)
    {
        return;
    }

    PacketDataReporter reporter(mDelegate, info->Interface, data);

    if (!ParsePacket(data, &reporter))
    {
        ChipLogError(Discovery, "Failed to parse received mDNS packet");
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

CHIP_ERROR MinMdnsResolver::ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type)
{
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    QueryBuilder builder(std::move(buffer));
    builder.Header().SetMessageId(0);

    {
        char nameBuffer[64] = "";

        // Node and fabricid are encoded in server names.
        ReturnErrorOnFailure(MakeInstanceName(nameBuffer, sizeof(nameBuffer), peerId));

        const char * instanceQName[] = { nameBuffer, "_chip", "_tcp", "local" };
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
