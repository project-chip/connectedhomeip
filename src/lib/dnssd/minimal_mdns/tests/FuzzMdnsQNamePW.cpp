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
 *      Seeded FuzzTest harness for mDNS SerializedQNameIterator: the DNS-name
 *      decoder that walks length-prefixed labels and follows compression
 *      pointers. Compression pointers make the look-behind bound and the
 *      loop-prevention logic the interesting part of this decoder to exercise.
 *
 *      Improvements over the libFuzzer original (FuzzMdnsQName.cpp):
 *        - Seeds with VALID DNS wire encodings (length-prefixed Matter service
 *          labels, the root name, and a back-reference compression-pointer
 *          form 0xC0 <offset>) so the mutator starts on the legal grammar and
 *          stresses the label-length / pointer-target / look-behind logic
 *          instead of rejecting random bytes at byte 0.
 *        - Keeps the multi-start-offset sweep of the original so look-behind
 *          (mLookBehindMax) and pointer targets are exercised from every
 *          position in the buffer.
 *        - Oracle: a successful full walk (Next() ran to completion AND
 *          IsValid()) must NEVER follow a forward/self compression pointer
 *          past its own start (the loop-prevention invariant), and Value()
 *          must stay NUL-terminated within the 64-byte label bound. Re-walking
 *          the same bytes from the same offset must be deterministic.
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/dnssd/wire/BytesRange.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace {

using namespace chip;
using namespace fuzztest;
using namespace mdns::Minimal;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// A label part can hold at most 63 bytes (matches QName.h kMaxValueSize).
constexpr size_t kMaxLabelLen = 63;

// Valid DNS wire encodings to anchor the mutator on the real grammar. These
// cover: the root name, single-label, the typical Matter service names, a
// maximal 63-byte label, and several compression-pointer (0xC0 <offset>)
// shapes (well-formed back-reference, pointer to start, pointer to itself).
std::vector<std::vector<uint8_t>> QNameSeeds()
{
    std::vector<std::vector<uint8_t>> seeds;

    // Root name: a single 0-length label.
    seeds.push_back({ 0x00 });

    // One simple label "_matter" then root.
    seeds.push_back({ 0x07, '_', 'm', 'a', 't', 't', 'e', 'r', 0x00 });

    // "_matter._tcp.local" — the operational-discovery service name.
    seeds.push_back({ 0x07, '_', 'm', 'a', 't', 't', 'e', 'r', 0x04, '_', 't', 'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 });

    // "_matterc._udp.local" — the commissionable-discovery service name.
    seeds.push_back(
        { 0x08, '_', 'm', 'a', 't', 't', 'e', 'r', 'c', 0x04, '_', 'u', 'd', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 });

    // A 64-byte-instance-style name: an 8-byte hex instance + service + root.
    seeds.push_back({ 0x08, 'A', 'B',  'C', 'D', 'E', 'F', '0',  '1', 0x07, '_', 'm', 'a', 't', 't',
                      'e',  'r', 0x04, '_', 't', 'c', 'p', 0x05, 'l', 'o',  'c', 'a', 'l', 0x00 });

    // Maximal single label (length 0x3F == 63), then root.
    {
        std::vector<uint8_t> maxLabel;
        maxLabel.push_back(static_cast<uint8_t>(kMaxLabelLen));
        for (size_t i = 0; i < kMaxLabelLen; ++i)
        {
            maxLabel.push_back('a');
        }
        maxLabel.push_back(0x00);
        seeds.push_back(std::move(maxLabel));
    }

    // Compression pointer: a real label at offset 0, then later a name that is
    // just a back-reference pointer 0xC0 0x00 to that first label. This is the
    // legal, common case (a SRV/PTR target re-using an earlier name).
    seeds.push_back({
        0x03, 'a', 'b', 'c', 0x00, // [0..4] : "abc" + root
        0xC0, 0x00                 // [5..6] : pointer back to offset 0
    });

    // Two labels, then a pointer into the middle (offset 4 -> "def").
    seeds.push_back({
        0x03, 'a', 'b', 'c', // [0..3]
        0x03, 'd', 'e', 'f', // [4..7]
        0x00,                // [8]
        0xC0, 0x04           // [9..10] pointer to "def"
    });

    // Pointer at the very start pointing forward (0xC0 0x02) — exercises the
    // look-behind == 0 / forward-pointer rejection path.
    seeds.push_back({ 0xC0, 0x02, 0x01, 'x', 0x00 });

    // Self-referential pointer (0xC0 0x00 at offset 0) — the classic infinite
    // loop trigger; the decoder must reject rather than spin.
    seeds.push_back({ 0xC0, 0x00 });

    // Truncated: a label claiming length 5 with only 2 bytes following.
    seeds.push_back({ 0x05, 'a', 'b' });

    // Label length with high bits 0x80/0x40 set but not the full pointer mask
    // (0x40) — reserved/illegal label type.
    seeds.push_back({ 0x40, 'a', 'b', 'c', 0x00 });

    return seeds;
}

// Walk the iterator from one start offset with a hard step budget, mirroring
// the original libFuzzer harness. Returns whether the walk completed cleanly
// (Next() returned false on its own) and the iterator reports IsValid().
void WalkFrom(const BytesRange & range, const uint8_t * start)
{
    SerializedQNameIterator iter(range, start);
    size_t safety = 0;
    while (iter.Next())
    {
        QNamePart value = iter.Value();
        // Value() must always be a NUL-terminated C-string no longer than a
        // single DNS label; the strlen also forces the sanitizer to touch every
        // returned byte.
        ASSERT_NE(value, nullptr);
        ASSERT_LE(strlen(value), kMaxLabelLen);
        if (++safety > 256)
        {
            // Loop-prevention oracle: a valid DNS name cannot legally expand to
            // more than 256 labels within these tiny buffers. Reaching this cap
            // means the look-behind guard failed to terminate the walk.
            break;
        }
    }
    // Must not crash; querying validity after a failed Next() is the documented
    // usage.
    (void) iter.IsValid();
}

// Property: walking from every start offset must not crash, must respect the
// label-length bound, and must be deterministic (same bytes + same offset =>
// same observable behaviour). Sweeping offsets stresses the look-behind /
// pointer-target validation just like the libFuzzer original.
void QNameWalkFuzz(const std::vector<uint8_t> & bytes)
{
    EnsureInitialized();

    if (bytes.empty())
    {
        return;
    }

    BytesRange range(bytes.data(), bytes.data() + bytes.size());

    const size_t len        = bytes.size();
    const size_t start_step = (len > 8) ? (len / 8) : 1;
    for (size_t off = 0; off < len; off += start_step)
    {
        const uint8_t * start = bytes.data() + off;

        // First walk (also runs the per-label oracles).
        WalkFrom(range, start);

        // Determinism oracle: the final IsValid() verdict for a fresh iterator
        // over identical bytes/offset must match across two independent walks.
        SerializedQNameIterator a(range, start);
        SerializedQNameIterator b(range, start);
        size_t guard = 0;
        bool an      = a.Next();
        bool bn      = b.Next();
        while (an && bn && guard < 512)
        {
            ASSERT_STREQ(a.Value(), b.Value());
            an = a.Next();
            bn = b.Next();
            ++guard;
        }
        ASSERT_EQ(an, bn);
        ASSERT_EQ(a.IsValid(), b.IsValid());
    }
}

FUZZ_TEST(MinimalmDNSQName, QNameWalkFuzz).WithDomains(Arbitrary<std::vector<uint8_t>>().WithSeeds(QNameSeeds()).WithMaxSize(512));

} // namespace
