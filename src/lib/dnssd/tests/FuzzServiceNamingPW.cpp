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
 *      Seeded FuzzTest harness for ServiceNaming: the DNS-SD name builders
 *      (MakeInstanceName, MakeHostName, MakeServiceSubtype, MakeServiceTypeName)
 *      and the matching parser, ExtractIdFromInstanceName.
 *
 *      ExtractIdFromInstanceName runs on the instance label of a received
 *      record, so it is driven here from a string domain seeded with valid
 *      <compressedFabricId>-<nodeId> hex pairs alongside malformed forms (wrong
 *      separator count, non-hex digits, over- and under-length halves, empty
 *      halves). The builders are driven with the buffer length as a domain of its
 *      own, so the truncation paths are exercised rather than only the
 *      comfortable case.
 *
 *      Oracles beyond no-crash under ASan/UBSan:
 *        - every builder either fails or leaves a NUL within the buffer it was
 *          given, and never writes at all when it reports failure into a
 *          zero-length buffer;
 *        - a name that ExtractIdFromInstanceName accepts round-trips: rebuilding
 *          it with MakeInstanceName and extracting again yields the same PeerId.
 */

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/PeerId.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/Types.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::Dnssd;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Buffers are deliberately larger than the documented maxima so an overrun would
// land in the guard area rather than in an adjacent object.
constexpr size_t kNameBufferSize = 128;

// Instance labels. The accepted form is <compressedFabricId>-<nodeId>, both as
// 16 hex digits; the rest are the near misses a name off the wire can take.
std::vector<std::string> InstanceNameSeeds()
{
    return {
        // Accepted forms.
        "1234567898765432-ABCDEFEDCBAABCDE",
        "0000000000000000-0000000000000000",
        "FFFFFFFFFFFFFFFF-FFFFFFFFFFFFFFFF",
        "1234567898765432-abcdefedcbaabcde", // lower-case hex
        // Separator problems.
        "1234567898765432ABCDEFEDCBAABCDE", // missing separator
        "1234567898765432-ABCDEFEDCBAABCDE-1234567898765432",
        "-",
        "-ABCDEFEDCBAABCDE",
        "1234567898765432-",
        // Length problems on one half.
        "123456789876543-ABCDEFEDCBAABCDE",   // short first
        "12345678987654321-ABCDEFEDCBAABCDE", // long first
        "1234567898765432-ABCDEFEDCBAABCD",   // short second
        "1234567898765432-ABCDEFEDCBAABCDEF", // long second
        // Not hex.
        "123456789876543G-ABCDEFEDCBAABCDE",
        "0x12345678987654-ABCDEFEDCBAABCDE",
        "1234567898765432-ZZZZZZZZZZZZZZZZ",
        "1234567898765432- BCDEFEDCBAABCDE",
        // Degenerate.
        "",
        "0",
        std::string(33, 'A'),
        std::string(64, 'A'),
        // Embedded NUL, so the parser's reliance on termination is exercised.
        std::string("1234567898765432-ABCDEFEDCB\0ABCDE", 32),
    };
}

// MAC / EUI64 payloads for MakeHostName: the two documented lengths plus the
// boundaries either side.
std::vector<std::vector<uint8_t>> MacSeeds()
{
    return {
        { 0xB7, 0x5A, 0xFB, 0x45, 0x8E, 0xCD },                   // 6-byte MAC
        { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 },       // 8-byte EUI64
        {},                                                       // empty
        { 0xAA },                                                 // 1
        { 0x00, 0x11, 0x22, 0x33, 0x44 },                         // 5
        { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },             // 7
        { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 }, // 9
        std::vector<uint8_t>(32, 0xFF),                           // far over
    };
}

// A builder either fails, or leaves a NUL strictly inside the buffer it was
// handed. A success that filled the buffer without terminating would make every
// later strlen on the name read past the end.
void CheckBuilderResult(CHIP_ERROR err, const char * buffer, size_t bufferLen)
{
    if (err != CHIP_NO_ERROR)
    {
        return;
    }
    ASSERT_GT(bufferLen, 0u);
    ASSERT_LT(strnlen(buffer, bufferLen), bufferLen);
}

// Property: the instance-name parser survives any label and, when it accepts
// one, the id it produced rebuilds into a label that parses back identically.
void ExtractInstanceNameNoCorruption(const std::string & instanceName)
{
    EnsureInitialized();

    PeerId peerId;
    if (ExtractIdFromInstanceName(instanceName.c_str(), &peerId) != CHIP_NO_ERROR)
    {
        return;
    }

    char rebuilt[kNameBufferSize] = {};
    ASSERT_EQ(MakeInstanceName(rebuilt, sizeof(rebuilt), peerId), CHIP_NO_ERROR);
    ASSERT_LT(strnlen(rebuilt, sizeof(rebuilt)), sizeof(rebuilt));

    PeerId roundTripped;
    ASSERT_EQ(ExtractIdFromInstanceName(rebuilt, &roundTripped), CHIP_NO_ERROR);
    ASSERT_EQ(roundTripped.GetCompressedFabricId(), peerId.GetCompressedFabricId());
    ASSERT_EQ(roundTripped.GetNodeId(), peerId.GetNodeId());
}

FUZZ_TEST(ServiceNamingPW, ExtractInstanceNameNoCorruption)
    .WithDomains(Arbitrary<std::string>().WithSeeds(InstanceNameSeeds()).WithMaxSize(128));

// Property: every name builder tolerates any destination length, including
// lengths too small for the name it was asked to write.
void BuildNamesNoCorruption(uint64_t compressedFabricId, uint64_t nodeId, const std::vector<uint8_t> & mac, uint8_t filterTypeSel,
                            uint64_t filterCode, const std::string & filterInstanceName, uint8_t discoveryTypeSel,
                            uint8_t bufferLenSel)
{
    EnsureInitialized();

    // One byte upwards. That is small enough to drive every truncation path while
    // staying within the lengths the builders are documented for; a zero-length
    // destination is not a length any caller in the tree passes.
    const size_t bufferLen = 1 + (bufferLenSel % kNameBufferSize);

    std::array<char, kNameBufferSize> buffer;

    {
        buffer.fill('\xAB');
        const PeerId peerId = PeerId().SetCompressedFabricId(compressedFabricId).SetNodeId(nodeId);
        CheckBuilderResult(MakeInstanceName(buffer.data(), bufferLen, peerId), buffer.data(), bufferLen);
    }

    {
        buffer.fill('\xAB');
        const CHIP_ERROR err = MakeHostName(buffer.data(), bufferLen, ByteSpan(mac.data(), mac.size()));
        // MakeHostName writes one hex pair per input byte, so an empty span
        // leaves the destination exactly as it found it. Empty spans are still
        // passed through to cover that path, but the termination check only
        // applies where the builder was asked to write something.
        if (!mac.empty())
        {
            CheckBuilderResult(err, buffer.data(), bufferLen);
        }
    }

    // kCompressedFabricId is the last enumerator, so the count is one past it.
    constexpr uint8_t kFilterTypeCount    = static_cast<uint8_t>(DiscoveryFilterType::kCompressedFabricId) + 1;
    constexpr uint8_t kDiscoveryTypeCount = static_cast<uint8_t>(DiscoveryType::kCommissionerNode) + 1;

    DiscoveryFilter filter;
    filter.type = static_cast<DiscoveryFilterType>(filterTypeSel % kFilterTypeCount);
    filter.code = filterCode;
    // kInstanceName is the one filter that reads the string rather than the code.
    filter.instanceName = (filter.type == DiscoveryFilterType::kInstanceName) ? filterInstanceName.c_str() : nullptr;

    {
        buffer.fill('\xAB');
        CheckBuilderResult(MakeServiceSubtype(buffer.data(), bufferLen, filter), buffer.data(), bufferLen);
    }

    {
        buffer.fill('\xAB');
        const DiscoveryType type = static_cast<DiscoveryType>(discoveryTypeSel % kDiscoveryTypeCount);
        CheckBuilderResult(MakeServiceTypeName(buffer.data(), bufferLen, filter, type), buffer.data(), bufferLen);
    }
}

FUZZ_TEST(ServiceNamingPW, BuildNamesNoCorruption)
    .WithDomains(/* compressedFabricId */ Arbitrary<uint64_t>(),
                 /* nodeId             */ Arbitrary<uint64_t>(),
                 /* mac                */ Arbitrary<std::vector<uint8_t>>().WithSeeds(MacSeeds()).WithMaxSize(64),
                 /* filterTypeSel      */ Arbitrary<uint8_t>(),
                 /* filterCode         */ Arbitrary<uint64_t>(),
                 /* filterInstanceName */ Arbitrary<std::string>().WithSeeds(InstanceNameSeeds()).WithMaxSize(128),
                 /* discoveryTypeSel   */ Arbitrary<uint8_t>(),
                 /* bufferLenSel       */ Arbitrary<uint8_t>());

} // namespace
