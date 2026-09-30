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
 *      Seeded FuzzTest harness for the mDNS record-data parsers: ParseTxtRecord,
 *      SrvRecord::Parse, ParsePtrRecord, ParseARecord, ParseAAAARecord. All are
 *      used when handling mDNS replies during commissioning / operational
 *      discovery.
 *
 *      Improvements over the libFuzzer original (FuzzMdnsRecordData.cpp):
 *        - Seeds with VALID record payloads for each type (realistic TXT
 *          key/value pairs, a well-formed SRV header + name, A = 4 bytes,
 *          AAAA = 16 bytes, PTR = a compression-pointer name) so the mutator
 *          immediately reaches the success paths and the field extraction
 *          that follows a successful parse.
 *        - Oracles beyond "no crash": when SRV / A / AAAA parse OK, the
 *          getters / walked name are exercised so the sanitizer touches the
 *          parsed-out fields, covering records whose length check passed but
 *          whose embedded name extends past the end of the packet.
 *        - The single fuzz input is fanned out to every parser (each parser
 *          has different length/format expectations), maximizing coverage per
 *          executed input.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/wire/BytesRange.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/dnssd/wire/RecordData.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace {

using namespace chip;
using namespace fuzztest;
using namespace chip::Dnssd;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

class NoopTxt : public TxtRecordDelegate
{
public:
    void OnRecord(const BytesRange & name, const BytesRange & value) override
    {
        // Touch the reported ranges so the sanitizer validates that name/value
        // point inside the supplied buffer (a bad length would surface here).
        for (const uint8_t * p = name.Start(); p < name.End(); ++p)
        {
            mAcc = static_cast<uint8_t>(mAcc + *p);
        }
        for (const uint8_t * p = value.Start(); p < value.End(); ++p)
        {
            mAcc = static_cast<uint8_t>(mAcc + *p);
        }
    }

private:
    volatile uint8_t mAcc = 0;
};

// Valid record payloads to anchor the mutator. The same byte vector is fed to
// every parser, so we mix shapes that are individually well-formed for at
// least one parser.
std::vector<std::vector<uint8_t>> RecordSeeds()
{
    std::vector<std::vector<uint8_t>> seeds;

    // ---- TXT records (length-prefixed "key=value" strings) ----
    // Single "PI=0" pairing key (typical Matter TXT key).
    seeds.push_back({ 0x04, 'P', 'I', '=', '0' });
    // "VP=65521+32769" vendor/product pairing.
    seeds.push_back({ 0x0E, 'V', 'P', '=', '6', '5', '5', '2', '1', '+', '3', '2', '7', '6', '9' });
    // Two TXT entries: "D=840" and "CM=2".
    seeds.push_back({ 0x05, 'D', '=', '8', '4', '0', 0x04, 'C', 'M', '=', '2' });
    // A key with no '=' (legal: value treated empty) followed by a normal pair.
    seeds.push_back({ 0x02, 'T', '1', 0x04, 'S', 'I', '=', '5' });
    // Empty-value form "X=".
    seeds.push_back({ 0x02, 'X', '=' });

    // ---- SRV records: 2+2+2 byte priority/weight/port, then a target name ----
    // priority=0 weight=0 port=5540 (0x15A4) target "_matter._tcp.local"+root.
    seeds.push_back({ 0x00, 0x00, 0x00, 0x00, 0x15, 0xA4, 0x07, '_', 'm', 'a', 't', 't', 'e',
                      'r',  0x04, '_',  't',  'c',  'p',  0x05, 'l', 'o', 'c', 'a', 'l', 0x00 });
    // Minimal-length SRV: 6 header bytes + a single root label (size == 7).
    seeds.push_back({ 0x00, 0x05, 0x00, 0x0A, 0x15, 0xA4, 0x00 });

    // ---- A record: exactly 4 bytes (IPv4 192.168.1.10) ----
    seeds.push_back({ 0xC0, 0xA8, 0x01, 0x0A });

    // ---- AAAA record: exactly 16 bytes (fe80::1 link-local) ----
    seeds.push_back({ 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 });

    // ---- PTR record: a name. A real label then a back-reference pointer. ----
    seeds.push_back({ 0x07, '_', 'm', 'a', 't', 't', 'e', 'r', 0x04, '_', 't', 'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 });
    // PTR with a compression pointer (label at 0, pointer 0xC0 0x00 after root).
    seeds.push_back({ 0x03, 'a', 'b', 'c', 0x00, 0xC0, 0x00 });

    return seeds;
}

// Walk a name iterator to completion with a hard budget so a malformed /
// looping name in a parsed record cannot hang the fuzzer.
void DrainName(SerializedQNameIterator name)
{
    size_t safety = 0;
    while (name.Next())
    {
        (void) name.Value();
        if (++safety > 256)
        {
            break;
        }
    }
    (void) name.IsValid();
}

// Property: every parser must survive arbitrary (seeded + mutated) bytes
// without crashing or reading outside the supplied range; where a record parses
// OK, the extracted fields must be safe to read.
void RecordDataFuzz(const std::vector<uint8_t> & bytes)
{
    EnsureInitialized();

    BytesRange range(bytes.data(), bytes.data() + bytes.size());

    // TXT
    {
        NoopTxt sink;
        (void) ParseTxtRecord(range, &sink);
    }

    // SRV — on success, walk the embedded name (bounded by the same range) and
    // read the numeric getters. A length check that passed but a name that
    // runs off the packet would be caught walking the name here.
    {
        SrvRecord srv;
        if (srv.Parse(range, range))
        {
            (void) srv.GetPriority();
            (void) srv.GetWeight();
            (void) srv.GetPort();
            DrainName(srv.GetName());
        }
    }

    // PTR — the validity range bounds the name pointer; on success drain it.
    {
        SerializedQNameIterator name;
        if (ParsePtrRecord(range, range, &name))
        {
            DrainName(name);
        }
    }

    // A (IPv4) — on success, force a read of the parsed address.
    {
        Inet::IPAddress addr;
        if (ParseARecord(range, &addr))
        {
            (void) addr.IsIPv4();
        }
    }

    // AAAA (IPv6) — on success, force a read of the parsed address.
    {
        Inet::IPAddress addr;
        if (ParseAAAARecord(range, &addr))
        {
            (void) addr.IsIPv6();
        }
    }
}

FUZZ_TEST(MinimalmDNSRecordData, RecordDataFuzz)
    .WithDomains(Arbitrary<std::vector<uint8_t>>().WithSeeds(RecordSeeds()).WithMaxSize(512));

} // namespace
