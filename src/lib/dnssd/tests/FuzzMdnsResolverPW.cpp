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
 *      Seeded/typed FuzzTest harness for the DNS-SD IncrementalResolver
 *      *orchestration* layer — the code that drives an mDNS discovery record
 *      set (SRV -> TXT -> A/AAAA) into the fixed-size NodeData buffers.
 *
 *      Coverage gap closed: FuzzPacketParsingPW stops at raw record parsing,
 *      and FuzzTxtFieldsPW exercises FillNodeDataFromTxt() in isolation with a
 *      single (key,value) pair. Neither drives the full resolver lifecycle:
 *        InitializeParsing(srv) -> OnRecord(TXT) -> OnRecord(AAAA) -> Take()
 *      which is what wires the SRV server/instance name, the multi-pair TXT
 *      delegate (TxtParser -> FillNodeDataFromTxt), and the IP accumulation
 *      together into CommonResolutionData / CommissionNodeData. The copies that
 *      land in deviceName[33], pairingInstruction[129], rotatingId[50],
 *      instanceName[] and hostName[] are only reached through this path.
 *
 *      Construction strategy mirrors TestIncrementalResolve.cpp: build typed
 *      ResourceRecords (Srv/Txt/IP) with a fuzzer-chosen QName, serialize them
 *      to wire bytes, re-parse into a ResourceData, and feed that to the real
 *      resolver, so the same decode the receive path performs is exercised here.
 *
 *      Oracle (beyond no-crash under ASan/UBSan): whenever the resolver hands
 *      back a NodeData, every fixed-size character buffer it produced must be
 *      NUL-terminated strictly within bounds (strnlen < sizeof), and the
 *      rotating-id length must never exceed its backing array.
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/dnssd/IncrementalResolve.h>
#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/Types.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/dnssd/wire/RecordWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace {

using namespace chip;
using namespace chip::Dnssd;
using namespace mdns::Minimal;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Service flavor selects the Matter suffix appended to the fuzzer-controlled
// instance label, so InitializeParsing classifies the name and selects the
// operational vs. commission specific-data variant.
enum class ServiceFlavor : uint8_t
{
    kOperational,    // <instance>._matter._tcp.local
    kCommissionable, // <instance>._matterc._udp.local
    kCommissioner,   // <instance>._matterd._udp.local
};

// Holds the QName-part strings stably so the QNamePart (const char *) array we
// hand to FullQName stays valid for the record's lifetime.
struct QNameHolder
{
    std::vector<std::string> storage;
    std::vector<QNamePart> parts;

    FullQName Full()
    {
        parts.clear();
        parts.reserve(storage.size());
        for (auto & s : storage)
        {
            parts.push_back(s.c_str());
        }
        FullQName q;
        q.names     = parts.data();
        q.nameCount = parts.size();
        return q;
    }
};

QNameHolder MakeServiceName(ServiceFlavor flavor, const std::string & instance)
{
    QNameHolder holder;
    holder.storage.push_back(instance);
    switch (flavor)
    {
    case ServiceFlavor::kOperational:
        holder.storage.push_back("_matter");
        holder.storage.push_back("_tcp");
        break;
    case ServiceFlavor::kCommissionable:
        holder.storage.push_back("_matterc");
        holder.storage.push_back("_udp");
        break;
    case ServiceFlavor::kCommissioner:
        holder.storage.push_back("_matterd");
        holder.storage.push_back("_udp");
        break;
    }
    holder.storage.push_back("local");
    return holder;
}

QNameHolder MakeHostName(const std::string & host)
{
    QNameHolder holder;
    holder.storage.push_back(host);
    holder.storage.push_back("local");
    return holder;
}

// Serializes a ResourceRecord to wire bytes and re-parses it into a
// ResourceData, exactly as the network receive path does. Owns the byte
// buffer so the parsed ResourceData / QName iterators remain valid.
class WireRecord
{
public:
    bool Build(const ResourceRecord & record)
    {
        uint8_t headerBuffer[HeaderRef::kSizeBytes] = {};
        HeaderRef dummyHeader(headerBuffer);

        chip::Encoding::BigEndian::BufferWriter output(mBuffer.data(), mBuffer.size());
        RecordWriter writer(&output);

        if (!record.Append(dummyHeader, ResourceType::kAnswer, writer) || !writer.Fit())
        {
            return false;
        }

        const uint8_t * ptr = mBuffer.data();
        BytesRange packet(mBuffer.data(), mBuffer.data() + mBuffer.size());
        return mResource.Parse(packet, &ptr);
    }

    const ResourceData & Resource() const { return mResource; }
    BytesRange Packet() const { return BytesRange(mBuffer.data(), mBuffer.data() + mBuffer.size()); }

private:
    std::array<uint8_t, 1024> mBuffer = {};
    ResourceData mResource;
};

// After the resolver yields a NodeData, the fixed-size buffers it populated must
// be NUL-terminated within bounds, so a later strlen stays inside the array, and
// the rotating-id length must fit its backing array.
void CheckCommonBuffersInBounds(const CommonResolutionData & data)
{
    ASSERT_LT(strnlen(data.hostName, sizeof(data.hostName)), sizeof(data.hostName));
}

void CheckCommissionBuffersInBounds(const CommissionNodeData & data)
{
    CheckCommonBuffersInBounds(data);
    ASSERT_LE(data.rotatingIdLen, sizeof(data.rotatingId));
    ASSERT_LT(strnlen(data.instanceName, sizeof(data.instanceName)), sizeof(data.instanceName));
    ASSERT_LT(strnlen(data.deviceName, sizeof(data.deviceName)), sizeof(data.deviceName));
    ASSERT_LT(strnlen(data.pairingInstruction, sizeof(data.pairingInstruction)), sizeof(data.pairingInstruction));
}

// Number of A/AAAA records a single input may feed. CommonResolutionData holds
// kMaxIPAddresses of them, so allowing more than that lets the accumulation run
// past the end of the array and exercise the full/refusal path.
constexpr unsigned kMaxIpRecordsPerInput = CommonResolutionData::kMaxIPAddresses + 3;

// Drive the full IncrementalResolver lifecycle from fuzzer-controlled inputs.
void ResolverLifecycleNoCorruption(uint8_t flavorSel, const std::string & instance, const std::string & host, uint16_t port,
                                   uint64_t ttl, const std::vector<std::string> & txtEntries, uint8_t ipRecordCount,
                                   const std::array<uint8_t, 16> & ipBytes, uint8_t addressing, bool addPtr)
{
    EnsureInitialized();

    const ServiceFlavor flavor = static_cast<ServiceFlavor>(flavorSel % 3);

    // Send a record to the wrong name on roughly one input in four. A 50/50 split
    // starves the correctly-addressed path, which is the one that reaches the TXT
    // and IP handling behind the name check.
    const bool misaddressTxt = (addressing & 0x03) == 0;
    const bool misaddressIp  = (addressing & 0x0C) == 0;

    // ---- Build + parse the SRV record (the first record the lifecycle needs) ----
    QNameHolder serviceName = MakeServiceName(flavor, instance);
    QNameHolder hostName    = MakeHostName(host);

    SrvResourceRecord srvBuilder(serviceName.Full(), hostName.Full(), port);
    WireRecord srvWire;
    if (!srvWire.Build(srvBuilder))
    {
        return; // record too large to serialize into the wire buffer; not interesting
    }

    SrvRecord srv;
    if (!srv.Parse(srvWire.Resource().GetData(), srvWire.Packet()))
    {
        return;
    }

    IncrementalResolver resolver;
    CHIP_ERROR err = resolver.InitializeParsing(srvWire.Resource().GetName(), ttl, srv);
    if (err != CHIP_NO_ERROR || !resolver.IsActive())
    {
        // Non-Matter / unparseable name (e.g. instance too long for storage).
        return;
    }

    const Inet::InterfaceId interface = Inet::InterfaceId::Null();

    // ---- Feed a TXT record carrying the fuzzer's key=value pairs ----
    // A TXT is only consumed when addressed to the record (service) name;
    // misaddressTxt sends it to the host name instead so the name-mismatch
    // rejection in OnRecord is exercised as well.
    {
        std::vector<const char *> entryPtrs;
        entryPtrs.reserve(txtEntries.size());
        bool tooLong = false;
        for (const auto & e : txtEntries)
        {
            // The Txt builder rejects entries > 63 bytes (returns false from
            // WriteData); skip building rather than waste the input.
            if (e.size() > 63)
            {
                tooLong = true;
                break;
            }
            entryPtrs.push_back(e.c_str());
        }

        if (!tooLong && !entryPtrs.empty())
        {
            TxtResourceRecord txtBuilder(misaddressTxt ? hostName.Full() : serviceName.Full(), entryPtrs.data(), entryPtrs.size());
            WireRecord txtWire;
            if (txtWire.Build(txtBuilder))
            {
                (void) resolver.OnRecord(interface, txtWire.Resource(), txtWire.Packet());
            }
        }
    }

    // ---- Feed a PTR record ----
    // PTR is not one of the types the resolver consumes, so this drives the
    // ignored-record arm of the OnRecord type switch.
    if (addPtr)
    {
        PtrResourceRecord ptrBuilder(serviceName.Full(), hostName.Full());
        WireRecord ptrWire;
        if (ptrWire.Build(ptrBuilder))
        {
            (void) resolver.OnRecord(interface, ptrWire.Resource(), ptrWire.Packet());
        }
    }

    // ---- Feed A/AAAA records for the host so the resolver can complete ----
    // Each record gets a distinct address so they accumulate; feeding more than
    // kMaxIPAddresses reaches the point where the resolver stops accepting them.
    // misaddressIp addresses them to the service name instead of the host name,
    // exercising the name-mismatch rejection.
    for (unsigned n = 0; n < ipRecordCount % (kMaxIpRecordsPerInput + 1); ++n)
    {
        std::array<uint8_t, 16> bytes = ipBytes;
        bytes[15]                     = static_cast<uint8_t>(bytes[15] ^ n);

        Inet::IPAddress addr;
        memcpy(addr.Addr, bytes.data(), sizeof(addr.Addr));

        IPResourceRecord ipBuilder(misaddressIp ? serviceName.Full() : hostName.Full(), addr);
        WireRecord ipWire;
        if (ipWire.Build(ipBuilder))
        {
            (void) resolver.OnRecord(interface, ipWire.Resource(), ipWire.Packet());
        }
    }

    // Whatever arrived, the resolver must be able to report what it still needs
    // without tripping over the partially populated state.
    (void) resolver.GetMissingRequiredInformation();

    // ---- Take whatever was accumulated and validate the fill-sink buffers ----
    if (resolver.IsActiveOperationalParse())
    {
        ResolvedNodeData nodeData;
        if (resolver.Take(nodeData) == CHIP_NO_ERROR)
        {
            CheckCommonBuffersInBounds(nodeData.resolutionData);
        }
    }
    else if (resolver.IsActiveCommissionParse())
    {
        DiscoveredNodeData nodeData;
        if (resolver.Take(nodeData) == CHIP_NO_ERROR && nodeData.Is<CommissionNodeData>())
        {
            CheckCommissionBuffersInBounds(nodeData.Get<CommissionNodeData>());
        }
    }
}

// Instance labels: real Matter forms (operational <fabric>-<node>, commission
// 16-hex instance) plus boundary/oversize labels that stress the SRV name
// storage and instanceName[] copy.
std::vector<std::string> InstanceSeeds()
{
    return {
        "1234567898765432-ABCDEFEDCBAABCDE", // operational instance
        "C5038835313B8B98",                  // commissionable/commissioner instance
        "0000000000000000-0000000000000000",
        std::string(33, 'I'), // one over kInstanceNameMaxLength(32)
        std::string(63, 'I'), // max DNS label
        std::string(64, 'I'), // over the StoredServerName 64-byte budget
        "abcd",
    };
}

std::vector<std::string> HostSeeds()
{
    return {
        "B75AFB458ECD", // typical 802.15.4 / MAC hex host
        "0011223344556677",
        std::string(16, 'h'), // kHostNameMaxLength
        std::string(17, 'h'), // one over
        std::string(63, 'h'), // max DNS label
        "abcd",
    };
}

// Whole TXT records (multiple key=value pairs), seeded with real discovery
// payloads and with boundary values for the fixed-size destination fields.
std::vector<std::vector<std::string>> TxtSeeds()
{
    return {
        // Realistic operational TXT.
        { "SII=23", "SAI=321", "T=1" },
        // Realistic commissionable TXT.
        { "D=22345", "VP=321+654", "CM=2", "DT=22", "DN=mytest", "PI=hello", "RI=00112233445566778899AABBCCDDEEFF" },
        // deviceName at / one over / well over kMaxDeviceNameLen(32).
        { std::string("DN=") + std::string(32, 'n') },
        { std::string("DN=") + std::string(33, 'n') },
        { std::string("DN=") + std::string(200, 'n') },
        // pairingInstruction at / one over / well over kMaxPairingInstructionLen(128).
        // Single TXT entry must stay <= 63 bytes on the wire, so split the long
        // PI across the limit-relevant boundary the builder allows.
        { std::string("PI=") + std::string(60, 'p') },
        // rotatingId hex at / one over / well over kMaxRotatingIdLen(50) bytes
        // (each byte = 2 hex chars; 50 bytes = 100 chars, capped by the 63-byte
        // TXT entry limit, so this stresses the cap logic up to the wire limit).
        { std::string("RI=") + std::string(60, 'A') },
        { std::string("RI=") + std::string(50, 'F') },
        { "RI=ZZ", "RI=0" }, // malformed / odd-length hex
        // discriminator / numeric boundaries.
        { "D=4095", "D=4096", "CM=255", "CM=256", "DT=65535" },
        // empty value forms.
        { "DN=", "PI=", "RI=", "D=" },
        // duplicate keys (later overwrites earlier in the fill).
        { "DN=first", "DN=second" },
    };
}

FUZZ_TEST(MdnsResolverPW, ResolverLifecycleNoCorruption)
    .WithDomains(
        /* flavorSel  */ Arbitrary<uint8_t>(),
        /* instance   */ Arbitrary<std::string>().WithSeeds(InstanceSeeds()).WithMaxSize(80),
        /* host       */ Arbitrary<std::string>().WithSeeds(HostSeeds()).WithMaxSize(80),
        /* port       */ Arbitrary<uint16_t>(),
        /* ttl        */ Arbitrary<uint64_t>(),
        // Inner max comfortably exceeds the largest seed entry (203 bytes); the
        // harness itself enforces the 63-byte on-wire TXT-entry limit before
        // building, so oversize entries exercise that skip path rather than
        // being rejected as invalid seeds.
        /* txtEntries */ VectorOf(Arbitrary<std::string>().WithMaxSize(256)).WithSeeds(TxtSeeds()).WithMaxSize(16),
        // Number of A/AAAA records fed; taken modulo kMaxIpRecordsPerInput + 1 so
        // some inputs overrun the resolver's address array.
        /* ipRecordCount */ Arbitrary<uint8_t>(),
        /* ipBytes       */ Arbitrary<std::array<uint8_t, 16>>(),
        // Bit pairs select whether the TXT / IP records are addressed to the
        // wrong name; see the derivation in the fuzz function.
        /* addressing    */ Arbitrary<uint8_t>(),
        /* addPtr        */ Arbitrary<bool>());

} // namespace
