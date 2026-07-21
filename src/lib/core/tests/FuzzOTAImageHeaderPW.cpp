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
 *      Streaming FuzzTest harness for OTAImageHeaderParser::AccumulateAndDecode.
 *
 *      The OTA image header is decoded incrementally from image chunks streamed
 *      over BDX during an OTA download (the Requestor feeds each downloaded block
 *      into AccumulateAndDecode). The parser accumulates into a heap buffer
 *      (reallocated to a header-declared TLV size), so both the byte content and
 *      the chunk boundaries vary per input. This harness drives a fuzzer-chosen
 *      sequence of chunks through the parser.
 *
 *      Seeds carry the valid file-identifier magic (0x1BEEF11E); without them the
 *      mutator never passes DecodeFixed's magic gate and DecodeTlv is unreachable.
 */

#include <cstdint>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/OTAImageHeader.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Valid Matter OTA headers (magic 0x1BEEF11E) taken verbatim from TestOTAImageHeader.cpp.
const std::vector<uint8_t> kMinOtaImage = { 0x1e, 0xf1, 0xee, 0x1b, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00,
                                            0x00, 0x00, 0x15, 0x24, 0x00, 0x01, 0x24, 0x01, 0x01, 0x24, 0x02, 0x01, 0x2c, 0x03,
                                            0x01, 0x31, 0x24, 0x04, 0x00, 0x24, 0x08, 0x09, 0x30, 0x09, 0x1c, 0x6b, 0x4e, 0x03,
                                            0x42, 0x36, 0x67, 0xdb, 0xb7, 0x3b, 0x6e, 0x15, 0x45, 0x4f, 0x0e, 0xb1, 0xab, 0xd4,
                                            0x59, 0x7f, 0x9a, 0x1b, 0x07, 0x8e, 0x3f, 0x5b, 0x5a, 0x6b, 0xc7, 0x18 };

const std::vector<uint8_t> kOtaImage = { 0x1e, 0xf1, 0xee, 0x1b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x00,
                                         0x00, 0x00, 0x15, 0x25, 0x00, 0xad, 0xde, 0x25, 0x01, 0xef, 0xbe, 0x26, 0x02, 0xff,
                                         0xff, 0xff, 0xff, 0x2c, 0x03, 0x03, 0x31, 0x2e, 0x30, 0x24, 0x04, 0x0c, 0x24, 0x05,
                                         0x01, 0x24, 0x06, 0x02, 0x2c, 0x07, 0x0a, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                                         0x2f, 0x72, 0x6e, 0x24, 0x08, 0x01, 0x30, 0x09, 0x20, 0x81, 0x3c, 0xa5, 0x28, 0x5c,
                                         0x28, 0xcc, 0xee, 0x5c, 0xab, 0x8b, 0x10, 0xeb, 0xda, 0x9c, 0x90, 0x8f, 0xd6, 0xd7,
                                         0x8e, 0xd9, 0xdc, 0x94, 0xcc, 0x65, 0xea, 0x6c, 0xb6, 0x7a, 0x7f, 0x13, 0xae, 0x18,
                                         0x74, 0x65, 0x73, 0x74, 0x20, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64 };

std::vector<uint8_t> Slice(const std::vector<uint8_t> & v, size_t begin, size_t end)
{
    return std::vector<uint8_t>(v.begin() + static_cast<long>(begin), v.begin() + static_cast<long>(end));
}

// Seed the corpus with the valid headers whole and split at the fixed-header / TLV boundary
// (offset 16 = kFixedHeaderSize) so the mutator explores the streaming Append path from a
// state that already passes the magic + size gates.
std::vector<std::vector<std::vector<uint8_t>>> OtaSeedChunkSeqs()
{
    return {
        { kMinOtaImage },
        { Slice(kMinOtaImage, 0, 16), Slice(kMinOtaImage, 16, kMinOtaImage.size()) },
        { Slice(kMinOtaImage, 0, 8), Slice(kMinOtaImage, 8, 20), Slice(kMinOtaImage, 20, kMinOtaImage.size()) },
        { kOtaImage },
        { Slice(kOtaImage, 0, 16), Slice(kOtaImage, 16, kOtaImage.size()) },
    };
}

// Property: any sequence of image chunks fed to the streaming decoder must not crash.
void OtaHeaderDecodeNoCrash(const std::vector<std::vector<uint8_t>> & chunks)
{
    EnsureInitialized();

    OTAImageHeaderParser parser;
    parser.Init();
    OTAImageHeader header;

    for (const auto & chunk : chunks)
    {
        if (chunk.empty())
        {
            // A 0-byte chunk is a no-op for the parser; skip it (an empty
            // std::vector's data() may be null).
            continue;
        }
        ByteSpan buffer(chunk.data(), chunk.size());
        const CHIP_ERROR err = parser.AccumulateAndDecode(buffer, header);
        // BUFFER_TOO_SMALL means "feed the next chunk"; anything else (success, or a hard error
        // that Clear()s the parser) means this decode is finished.
        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            break;
        }
    }

    parser.Clear();
}

FUZZ_TEST(FuzzOTAImageHeaderPW, OtaHeaderDecodeNoCrash)
    .WithDomains(VectorOf(Arbitrary<std::vector<uint8_t>>()).WithMaxSize(16).WithSeeds(OtaSeedChunkSeqs()));

} // namespace
