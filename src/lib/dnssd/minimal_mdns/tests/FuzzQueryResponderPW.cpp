/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      Seeded FuzzTest harness for the mDNS query-answering path: a received
 *      query packet is parsed, and every question in it is run through
 *      ResponseSender against a QueryResponder holding a realistic Matter
 *      advertisement (PTR, SRV, TXT and AAAA responders plus the
 *      _services._dns-sd._udp service listing).
 *
 *      Coverage gap closed: FuzzPacketParsingPW stops once a query has been
 *      decoded, and the resolver harnesses only cover the receive side of
 *      responses. Nothing drove the answering side, i.e. QueryReplyFilter's
 *      name/type/class matching, QueryResponder's iteration and
 *      additional-record marking, and ResponseSender's packet building and
 *      flushing. Those run whenever a device that advertises receives a query.
 *
 *      The whole query packet is fuzzer-controlled, header included, so the
 *      question count, flags, QType, QClass, unicast bit and question names are
 *      all driven. Seeds carry well-formed queries for the names this responder
 *      actually advertises, so the matching paths are reached immediately rather
 *      than only after the mutator guesses a name.
 *
 *      The generated reply is parsed back rather than dropped, so every byte the
 *      responders emitted is walked by the parser and read under the sanitizer.
 *
 *      Oracle beyond no-crash under ASan/UBSan: a reply, if one was produced,
 *      must be a well-formed mDNS response that the parser accepts, and the
 *      responder must never claim to have sent a reply that fails to parse.
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy_DefaultImpl.h>
#include <lib/dnssd/minimal_mdns/ResponseSender.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/responders/IP.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/QueryResponder.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/dnssd/wire/FlatAllocatedQName.h>
#include <lib/dnssd/wire/Parser.h>
#include <lib/dnssd/wire/RecordData.h>
#include <lib/dnssd/wire/records/Ptr.h>
#include <lib/dnssd/wire/records/Srv.h>
#include <lib/dnssd/wire/records/Txt.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>

namespace {

using namespace chip;
using namespace fuzztest;
using namespace mdns::Minimal;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        // The address responders resolve the addresses to advertise through the
        // global address policy, which the mDNS server installs during startup.
        SetDefaultAddressPolicy();
        return true;
    }();
    (void) sInitialized;
}

// Accepts whatever the responders produce and keeps the bytes so the reply can
// be parsed back. Mirrors the endpoint-pool construction the in-tree mDNS server
// uses, with a pool size of 0 since nothing is ever actually listened on.
class CapturingServer : private chip::PoolImpl<ServerBase::EndpointInfo, 0, chip::ObjectPoolMem::kInline,
                                               ServerBase::EndpointInfoPoolType::Interface>,
                        public ServerBase
{
public:
    CapturingServer() : ServerBase(*static_cast<ServerBase::EndpointInfoPoolType *>(this)) {}

    CHIP_ERROR DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
                          chip::Inet::InterfaceId interface) override
    {
        return Capture(std::move(data));
    }

    CHIP_ERROR BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port) override
    {
        return Capture(std::move(data));
    }

    CHIP_ERROR BroadcastUnicastQuery(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface,
                                     chip::Inet::IPAddressType addressType) override
    {
        return Capture(std::move(data));
    }

    CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port) override { return Capture(std::move(data)); }

    CHIP_ERROR BroadcastSend(chip::System::PacketBufferHandle && data, uint16_t port, chip::Inet::InterfaceId interface,
                             chip::Inet::IPAddressType addressType) override
    {
        return Capture(std::move(data));
    }

private:
    CHIP_ERROR Capture(chip::System::PacketBufferHandle && data)
    {
        if (data.IsNull())
        {
            return CHIP_NO_ERROR;
        }
        // Walk the reply the responders just built. A reply that the parser
        // rejects would mean the builder emitted something it cannot itself read.
        NullParserDelegate delegate;
        const BytesRange reply(data->Start(), data->Start() + data->DataLength());
        EXPECT_TRUE(ParsePacket(reply, &delegate));
        return CHIP_NO_ERROR;
    }

    class NullParserDelegate : public ParserDelegate
    {
    public:
        void OnHeader(ConstHeaderRef & header) override { (void) header.GetFlags().IsResponse(); }
        void OnQuery(const QueryData & data) override { (void) data.GetType(); }
        void OnResource(ResourceType type, const ResourceData & data) override { (void) data.GetType(); }
    };
};

// A realistic advertisement for the query to match against: the operational and
// commissionable service names, an instance, a host and a TXT set.
struct Advertisement
{
    // FlatAllocatedQName::Build lays out a QNamePart pointer array followed by the
    // NUL-terminated labels, so each buffer must hold
    // sum(sizeof(QNamePart) + strlen(label) + 1) and must be aligned for
    // QNamePart. On a 64-bit build the largest of the names below needs 68 bytes.
    static constexpr size_t kQNameStorage = 128;

    alignas(QNamePart) uint8_t dnsSdStorage[kQNameStorage];
    alignas(QNamePart) uint8_t serviceStorage[kQNameStorage];
    alignas(QNamePart) uint8_t instanceStorage[kQNameStorage];
    alignas(QNamePart) uint8_t hostStorage[kQNameStorage];
    alignas(QNamePart) uint8_t txtStorage[kQNameStorage];

    FullQName dnsSd;
    FullQName service;
    FullQName instance;
    FullQName host;
    FullQName txt;

    static constexpr uint16_t kPort = 5540;

    PtrResourceRecord ptrRecord;
    SrvResourceRecord srvRecord;
    TxtResourceRecord txtRecord;

    PtrResponder ptrResponder;
    SrvResponder srvResponder;
    TxtResponder txtResponder;
    IPv4Responder ipv4Responder;
    IPv6Responder ipv6Responder;

    QueryResponder<16> queryResponder;

    Advertisement() :
        dnsSd(FlatAllocatedQName::Build(dnsSdStorage, "_services", "_dns-sd", "_udp", "local")),
        service(FlatAllocatedQName::Build(serviceStorage, "_matter", "_tcp", "local")),
        instance(FlatAllocatedQName::Build(instanceStorage, "C5038835313B8B98", "_matter", "_tcp", "local")),
        host(FlatAllocatedQName::Build(hostStorage, "B75AFB458ECD", "local")),
        txt(FlatAllocatedQName::Build(txtStorage, "SII=23", "SAI=321", "T=1")), ptrRecord(service, instance),
        srvRecord(instance, host, kPort), txtRecord(instance, txt), ptrResponder(service, instance), srvResponder(srvRecord),
        txtResponder(txtRecord), ipv4Responder(host), ipv6Responder(host)
    {
        // Tripwire for a later edit that lengthens a label past the sizing above.
        // ASSERT_* cannot be used here: it expands to a return, which a
        // constructor may not do.
        VerifyOrDie(FlatAllocatedQName::RequiredStorageSize("_services", "_dns-sd", "_udp", "local") <= kQNameStorage);
        VerifyOrDie(FlatAllocatedQName::RequiredStorageSize("_matter", "_tcp", "local") <= kQNameStorage);
        VerifyOrDie(FlatAllocatedQName::RequiredStorageSize("C5038835313B8B98", "_matter", "_tcp", "local") <= kQNameStorage);
        VerifyOrDie(FlatAllocatedQName::RequiredStorageSize("B75AFB458ECD", "local") <= kQNameStorage);
        VerifyOrDie(FlatAllocatedQName::RequiredStorageSize("SII=23", "SAI=321", "T=1") <= kQNameStorage);

        queryResponder.Init();
        queryResponder.AddResponder(&ptrResponder).SetReportAdditional(instance).SetReportInServiceListing(true);
        queryResponder.AddResponder(&srvResponder).SetReportAdditional(host);
        queryResponder.AddResponder(&txtResponder);
        queryResponder.AddResponder(&ipv4Responder);
        queryResponder.AddResponder(&ipv6Responder);
    }
};

// Well-formed query packets. Header is 12 bytes: id, flags, qdcount, ancount,
// nscount, arcount; then each question is a name, a 2-byte QType and a 2-byte
// QClass.
std::vector<std::vector<uint8_t>> QuerySeeds()
{
    const std::vector<uint8_t> header = { 0x12, 0x34, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    auto query = [&header](std::vector<uint8_t> question) {
        std::vector<uint8_t> packet = header;
        packet.insert(packet.end(), question.begin(), question.end());
        return packet;
    };

    // "_matter._tcp.local" ANY/IN -- matches the PTR responder.
    const std::vector<uint8_t> serviceName = { 0x07, '_', 'm', 'a',  't', 't', 'e', 'r', 0x04, '_',
                                               't',  'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l',  0x00 };
    // The advertised instance, matching SRV and TXT.
    const std::vector<uint8_t> instanceName = { 0x10, 'C', '5', '0', '3',  '8', '8', '3', '5', '3', '1', '3', 'B',
                                                '8',  'B', '9', '8', 0x07, '_', 'm', 'a', 't', 't', 'e', 'r', 0x04,
                                                '_',  't', 'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 };
    // The service-listing name.
    const std::vector<uint8_t> dnsSdName = { 0x09, '_', 's', 'e',  'r', 'v', 'i', 'c', 'e',  's', 0x07, '_', 'd', 'n', 's',
                                             '-',  's', 'd', 0x04, '_', 'u', 'd', 'p', 0x05, 'l', 'o',  'c', 'a', 'l', 0x00 };

    auto withTypeClass = [](std::vector<uint8_t> name, uint16_t qtype, uint16_t qclass) {
        name.push_back(static_cast<uint8_t>(qtype >> 8));
        name.push_back(static_cast<uint8_t>(qtype & 0xFF));
        name.push_back(static_cast<uint8_t>(qclass >> 8));
        name.push_back(static_cast<uint8_t>(qclass & 0xFF));
        return name;
    };

    std::vector<std::vector<uint8_t>> seeds;
    seeds.push_back(query(withTypeClass(serviceName, 255, 1)));   // ANY / IN
    seeds.push_back(query(withTypeClass(serviceName, 12, 1)));    // PTR / IN
    seeds.push_back(query(withTypeClass(instanceName, 33, 1)));   // SRV / IN
    seeds.push_back(query(withTypeClass(instanceName, 16, 1)));   // TXT / IN
    seeds.push_back(query(withTypeClass(instanceName, 255, 1)));  // ANY / IN
    seeds.push_back(query(withTypeClass(dnsSdName, 12, 1)));      // service listing
    seeds.push_back(query(withTypeClass(serviceName, 255, 255))); // ANY class
    // Unicast-response bit set in QClass.
    seeds.push_back(query(withTypeClass(serviceName, 255, 0x8001)));
    // A name that matches nothing advertised.
    seeds.push_back(query(withTypeClass({ 0x04, '_', 'h', 't', 't', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 }, 255, 1)));

    // Two questions in one packet.
    {
        std::vector<uint8_t> two = { 0x12, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        auto a                   = withTypeClass(serviceName, 12, 1);
        auto b                   = withTypeClass(instanceName, 33, 1);
        two.insert(two.end(), a.begin(), a.end());
        two.insert(two.end(), b.begin(), b.end());
        seeds.push_back(two);
    }

    // Header claiming more questions than the packet holds.
    seeds.push_back({ 0x12, 0x34, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    // Header only.
    seeds.push_back(header);
    // Truncated header.
    seeds.push_back({ 0x12, 0x34, 0x00 });

    return seeds;
}

// Runs each question in the received packet through the responder set.
class RespondingDelegate : public ParserDelegate
{
public:
    RespondingDelegate(ResponseSender & sender, const Inet::IPPacketInfo & source) : mSender(sender), mSource(source) {}

    void OnHeader(ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }

    void OnQuery(const QueryData & data) override
    {
        // Bound the work a single packet can trigger; a header may claim many
        // more questions than are worth answering here.
        if (++mQueries > 8)
        {
            return;
        }
        QueryData query = data;
        (void) mSender.Respond(mMessageId, query, &mSource, ResponseConfiguration());
    }

    void OnResource(ResourceType type, const ResourceData & data) override { (void) data.GetType(); }

private:
    ResponseSender & mSender;
    const Inet::IPPacketInfo & mSource;
    uint16_t mMessageId = 0;
    unsigned mQueries   = 0;
};

// Property: answering any received query packet neither crashes nor produces a
// reply the parser cannot read back.
void RespondToQueryNoCorruption(const std::vector<uint8_t> & queryPacket)
{
    EnsureInitialized();

    Advertisement advertisement;
    CapturingServer server;

    ResponseSender sender(&server);
    if (sender.AddQueryResponder(&advertisement.queryResponder) != CHIP_NO_ERROR)
    {
        return;
    }

    Inet::IPPacketInfo source;
    source.Clear();
    source.SrcPort  = 5353;
    source.DestPort = 5353;

    const BytesRange packet(queryPacket.data(), queryPacket.data() + queryPacket.size());
    RespondingDelegate delegate(sender, source);
    (void) ParsePacket(packet, &delegate);
}

FUZZ_TEST(QueryResponderPW, RespondToQueryNoCorruption)
    .WithDomains(Arbitrary<std::vector<uint8_t>>().WithSeeds(QuerySeeds()).WithMaxSize(512));

} // namespace
