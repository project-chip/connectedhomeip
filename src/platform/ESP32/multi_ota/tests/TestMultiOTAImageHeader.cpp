/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdint>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

#include "../MultiOTAImageHeader.h"

namespace {

using namespace chip;

struct TestEntry
{
    uint32_t imageId;
    uint32_t version;
    uint32_t offset;
    uint32_t length;
};

void AppendU32LE(std::vector<uint8_t> & buf, uint32_t value)
{
    buf.push_back(static_cast<uint8_t>(value & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

// Serialize a MultiOTAImageHeader (fixed preamble + SubImageHeader entries) in the on-wire
// little-endian layout. numImagesOverride / magic / reservedByte let callers craft malformed inputs.
std::vector<uint8_t> BuildHeader(const std::vector<TestEntry> & entries, uint32_t magic = kMultiOTAImageFileIdentifier,
                                 uint8_t reservedByte = 0, int numImagesOverride = -1)
{
    std::vector<uint8_t> buf;
    AppendU32LE(buf, magic);
    const size_t numImages = (numImagesOverride >= 0) ? static_cast<size_t>(numImagesOverride) : entries.size();
    buf.push_back(static_cast<uint8_t>(numImages));
    buf.push_back(reservedByte);
    buf.push_back(0);
    buf.push_back(0);
    for (const auto & entry : entries)
    {
        AppendU32LE(buf, entry.imageId);
        AppendU32LE(buf, entry.version);
        AppendU32LE(buf, entry.offset);
        AppendU32LE(buf, entry.length);
        for (uint8_t i = 0; i < Crypto::kSHA256_Hash_Length; i++)
        {
            buf.push_back(static_cast<uint8_t>(entry.imageId + i)); // arbitrary but deterministic digest
        }
    }
    return buf;
}

CHIP_ERROR DecodeAtOnce(const std::vector<uint8_t> & blob, MultiOTAImageHeaderParser & parser, MultiOTAImageHeader & header)
{
    ByteSpan span(blob.data(), blob.size());
    return parser.AccumulateAndDecode(span, header);
}

TEST(TestMultiOTAImageHeader, ValidSingleImage)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    EXPECT_TRUE(parser.IsInitialized());
    EXPECT_FALSE(parser.IsHeaderParsed());

    // offset for a single entry is the header size: 8 + 1 * 48 = 56
    const std::vector<uint8_t> blob = BuildHeader({ { kAppImageProcessorTag, 5, 56, 1024 } });

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_NO_ERROR);
    EXPECT_TRUE(parser.IsHeaderParsed());
    EXPECT_EQ(header.numImages, 1);
    ASSERT_EQ(header.subImages.size(), 1u);
    EXPECT_EQ(header.subImages[0].imageId, kAppImageProcessorTag);
    EXPECT_EQ(header.subImages[0].version, 5u);
    EXPECT_EQ(header.subImages[0].offset, 56u);
    EXPECT_EQ(header.subImages[0].length, 1024u);

    parser.Clear();
    EXPECT_FALSE(parser.IsInitialized());
}

TEST(TestMultiOTAImageHeader, ValidMultipleImages)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();

    // header size for 2 entries: 8 + 2 * 48 = 104
    const std::vector<uint8_t> blob = BuildHeader({ { 0x100, 2, 104, 512 }, { kAppImageProcessorTag, 7, 616, 2048 } });

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_NO_ERROR);
    ASSERT_EQ(header.subImages.size(), 2u);
    EXPECT_EQ(header.subImages[0].imageId, 0x100u);
    EXPECT_EQ(header.subImages[0].length, 512u);
    EXPECT_EQ(header.subImages[1].imageId, kAppImageProcessorTag);
    EXPECT_EQ(header.subImages[1].length, 2048u);
    parser.Clear();
}

TEST(TestMultiOTAImageHeader, InvalidMagicRejected)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { kAppImageProcessorTag, 1, 56, 128 } }, /*magic=*/0xDEADBEEF);

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    // A hard error clears the parser so it cannot be fed further.
    EXPECT_FALSE(parser.IsInitialized());
}

TEST(TestMultiOTAImageHeader, ZeroNumImagesRejected)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({}, kMultiOTAImageFileIdentifier, /*reservedByte=*/0, /*numImagesOverride=*/0);

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestMultiOTAImageHeader, NonZeroReservedRejected)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { kAppImageProcessorTag, 1, 56, 128 } }, kMultiOTAImageFileIdentifier,
                                                  /*reservedByte=*/0x01);

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestMultiOTAImageHeader, ZeroImageIdRejected)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { /*imageId=*/0, 1, 56, 128 } });

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestMultiOTAImageHeader, ZeroLengthRejected)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { kAppImageProcessorTag, 1, 56, /*length=*/0 } });

    MultiOTAImageHeader header;
    EXPECT_EQ(DecodeAtOnce(blob, parser, header), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestMultiOTAImageHeader, SplitAcrossSingleByteChunks)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { 0x100, 2, 104, 512 }, { kAppImageProcessorTag, 7, 616, 2048 } });

    MultiOTAImageHeader header;
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;
    for (size_t i = 0; i < blob.size(); i++)
    {
        ByteSpan chunk(blob.data() + i, 1);
        err = parser.AccumulateAndDecode(chunk, header);
        if (i + 1 < blob.size())
        {
            // Every partial feed needs more data.
            EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
            EXPECT_FALSE(parser.IsHeaderParsed());
        }
    }
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(parser.IsHeaderParsed());
    ASSERT_EQ(header.subImages.size(), 2u);
    EXPECT_EQ(header.subImages[1].imageId, kAppImageProcessorTag);
    parser.Clear();
}

TEST(TestMultiOTAImageHeader, PreambleAndBodyInSeparateChunks)
{
    MultiOTAImageHeaderParser parser;
    parser.Init();
    const std::vector<uint8_t> blob = BuildHeader({ { kAppImageProcessorTag, 1, 56, 4096 } });

    MultiOTAImageHeader header;
    // First chunk: just the 8-byte preamble -> needs the sub-image body.
    ByteSpan first(blob.data(), kFixedHeaderSize);
    EXPECT_EQ(parser.AccumulateAndDecode(first, header), CHIP_ERROR_BUFFER_TOO_SMALL);
    // Second chunk: the remaining sub-image bytes -> completes.
    ByteSpan second(blob.data() + kFixedHeaderSize, blob.size() - kFixedHeaderSize);
    EXPECT_EQ(parser.AccumulateAndDecode(second, header), CHIP_NO_ERROR);
    ASSERT_EQ(header.subImages.size(), 1u);
    EXPECT_EQ(header.subImages[0].length, 4096u);
    parser.Clear();
}

TEST(TestMultiOTAImageHeader, ReusableAfterClear)
{
    MultiOTAImageHeaderParser parser;

    parser.Init();
    const std::vector<uint8_t> first = BuildHeader({ { kAppImageProcessorTag, 1, 56, 100 } });
    MultiOTAImageHeader header1;
    EXPECT_EQ(DecodeAtOnce(first, parser, header1), CHIP_NO_ERROR);
    parser.Clear();

    parser.Init();
    const std::vector<uint8_t> second = BuildHeader({ { 0x200, 9, 56, 200 } });
    MultiOTAImageHeader header2;
    EXPECT_EQ(DecodeAtOnce(second, parser, header2), CHIP_NO_ERROR);
    ASSERT_EQ(header2.subImages.size(), 1u);
    EXPECT_EQ(header2.subImages[0].imageId, 0x200u);
    EXPECT_EQ(header2.subImages[0].length, 200u);
    parser.Clear();
}

} // namespace
