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
 *      Seeded/typed FuzzTest harness for FillNodeDataFromTxt — the DNS-SD TXT
 *      key/value parser that populates CommonResolutionData and
 *      CommissionNodeData during operational and commissionable-node
 *      discovery.
 *
 *      The libFuzzer variant had to carve a key and a value out of one flat
 *      buffer (1-byte length prefix + key + value), which meant most of the
 *      mutator's energy went into discovering the handful of legal two/three
 *      letter keys instead of stressing the per-field numeric/string parsers.
 *      Here the key and value are separate typed domains: the key domain is
 *      seeded with the exact TXT keys from TxtFields.h (so the interesting
 *      switch arms are hit immediately) and the value domain is seeded with
 *      realistic and boundary values (boundary numbers, overflow forms,
 *      leading zeros, oversize strings, malformed hex).
 *
 *      Oracle (beyond no-crash): the parser must never leave a struct field
 *      outside the range its accessor promises —
 *        - MRP idle/active intervals, when present, are <= kMaxRetryInterval;
 *        - rotatingIdLen never exceeds the rotatingId[] buffer;
 *        - the fixed-size deviceName / pairingInstruction buffers stay
 *          NUL-terminated within bounds.
 *      No-crash under ASan/MSan/UBSan is the floor; these invariants are the
 *      ceiling.
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/TxtFields.h>
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

// Exact TXT key set from TxtFields.h (Internal::txtFieldInfo), plus a few
// near-miss / unknown keys so the kUnknown switch arm and the case-insensitive
// matcher are exercised too. Keeping these literal means every interesting
// branch of GetTxtFieldKey() is reachable from a seed rather than only by the
// mutator stumbling onto the right two bytes.
std::vector<std::string> KeySeeds()
{
    return {
        // ---- Common (operational) keys ----
        "SII",
        "SAI",
        "SAT",
        "T",
        "ICD",
        // ---- Commissionable / commissioner keys ----
        "D",
        "VP",
        "CM",
        "DT",
        "DN",
        "RI",
        "PI",
        "PH",
        "CP",
        // ---- Behind CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC, harmless otherwise ----
        "JF",
        // ---- Case variants (matcher is case-insensitive) ----
        "d",
        "cm",
        "sii",
        // ---- Unknown / near-miss / empty (kUnknown path) ----
        "",
        "X",
        "DD",
        "CRI",
        "CRA",
        "PAA",
    };
}

// Realistic and boundary TXT values. Mix of in-range, boundary, and
// out-of-range forms so the numeric parsers (strtoul + range clamps) and the
// fixed-buffer string copies are both stressed.
std::vector<std::string> ValueSeeds()
{
    return {
        // ---- small / boundary integers ----
        "",
        "0",
        "1",
        "2",
        "15",
        // ---- leading-zero forms (parser rejects these -> default) ----
        "01",
        "00",
        "007",
        // ---- SII/SAI clamp boundary: kMaxRetryInterval == 3600000 ----
        "3599999",
        "3600000",
        "3600001",
        // ---- SAT / u16 boundary ----
        "65534",
        "65535",
        "65536",
        // ---- u8 boundary (CM) ----
        "254",
        "255",
        "256",
        // ---- VP "vendor+product" forms ----
        "65521",
        "65521+32769",
        "840+15",
        "+15",
        "1+",
        "+",
        "0+0",
        // ---- decimal overflow / way-too-long numbers ----
        "99999999999",
        "4294967295",
        "4294967296",
        "18446744073709551615",
        "999999999999999999999999999999",
        // ---- non-numeric / mixed ----
        "abc",
        "1a",
        "  1",
        "1 ",
        "-1",
        "+1",
        "0x10",
        "1.5",
        // ---- rotating-device-id hex (RI): HexToBytes, capped at kMaxRotatingIdLen ----
        "00112233445566778899AABBCCDDEEFF",
        "0011223344",
        "ZZ",
        "0",
        std::string(200, 'A'), // 100 hex bytes worth -> must clamp to 50
        // ---- device name / pairing instruction oversize (fixed buffers) ----
        std::string(32, 'n'),  // exactly kMaxDeviceNameLen
        std::string(33, 'n'),  // one over
        std::string(128, 'p'), // exactly kMaxPairingInstructionLen
        std::string(129, 'p'), // one over
        std::string(512, 'x'), // far over any buffer
        // ---- embedded NUL / control bytes ----
        std::string("a\0b", 3),
        "\n",
        "\t",
    };
}

ByteSpan AsSpan(const std::string & s)
{
    return ByteSpan(reinterpret_cast<const uint8_t *>(s.data()), s.size());
}

// Shared invariants on the CommonResolutionData portion of any parsed struct.
// These are the promises the field accessors make; violating one means the
// parser left a field outside its documented range.
void CheckCommonInvariants(const CommonResolutionData & data)
{
    // SII / SAI: out-of-range input must collapse to nullopt, never a stored
    // value above the documented maximum.
    if (data.mrpRetryIntervalIdle.has_value())
    {
        ASSERT_LE(data.mrpRetryIntervalIdle->count(), kMaxRetryInterval.count());
    }
    if (data.mrpRetryIntervalActive.has_value())
    {
        ASSERT_LE(data.mrpRetryIntervalActive->count(), kMaxRetryInterval.count());
    }
    // SAT: 0 maps to nullopt, so a present value is always strictly positive.
    if (data.mrpRetryActiveThreshold.has_value())
    {
        ASSERT_NE(data.mrpRetryActiveThreshold->count(), 0);
    }
    // hostName is a fixed buffer; the parser never touches it here, but assert
    // it stayed NUL-terminated within bounds (cheap, catches stray writes).
    ASSERT_LT(strnlen(data.hostName, sizeof(data.hostName)), sizeof(data.hostName));
}

// Property: parsing any (key, value) into CommonResolutionData never crashes
// and never violates the field-range invariants.
void FillCommonResolutionDataNoCorruption(const std::string & key, const std::string & value)
{
    EnsureInitialized();

    CommonResolutionData data;
    FillNodeDataFromTxt(AsSpan(key), AsSpan(value), data);
    CheckCommonInvariants(data);
}

FUZZ_TEST(TxtFieldsPW, FillCommonResolutionDataNoCorruption)
    .WithDomains(Arbitrary<std::string>().WithSeeds(KeySeeds()).WithMaxSize(64),
                 Arbitrary<std::string>().WithSeeds(ValueSeeds()).WithMaxSize(1024));

// Property: parsing any (key, value) into CommissionNodeData never crashes and
// never violates the field-range invariants — including the rotating-device-id
// length bound and the fixed string-buffer bounds.
void FillCommissionNodeDataNoCorruption(const std::string & key, const std::string & value)
{
    EnsureInitialized();

    CommissionNodeData data;
    FillNodeDataFromTxt(AsSpan(key), AsSpan(value), data);

    // CommissionNodeData is-a CommonResolutionData; the common arms are reached
    // via the default case of the commission overload.
    CheckCommonInvariants(data);

    // RI: HexToBytes must never report more bytes than the destination holds.
    ASSERT_LE(data.rotatingIdLen, sizeof(data.rotatingId));

    // DN / PI: Platform::CopyString must keep these NUL-terminated in-bounds.
    ASSERT_LT(strnlen(data.deviceName, sizeof(data.deviceName)), sizeof(data.deviceName));
    ASSERT_LT(strnlen(data.pairingInstruction, sizeof(data.pairingInstruction)), sizeof(data.pairingInstruction));
}

FUZZ_TEST(TxtFieldsPW, FillCommissionNodeDataNoCorruption)
    .WithDomains(Arbitrary<std::string>().WithSeeds(KeySeeds()).WithMaxSize(64),
                 Arbitrary<std::string>().WithSeeds(ValueSeeds()).WithMaxSize(1024));

} // namespace
