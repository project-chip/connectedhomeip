/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/core/OTAImageHeader.h>
#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

namespace {

// Magic: 1beef11e
// Total Size: 110
// Header Size: 82
// Header TLV:
//   [0] Vendor Id: 57005 (0xdead)
//   [1] Product Id: 48879 (0xbeef)
//   [2] Version: 4294967295 (0xffffffff)
//   [3] Version String: 1.0
//   [4] Payload Size: 12 (0xc)
//   [5] Min Version: 1 (0x1)
//   [6] Max Version: 2 (0x2)
//   [7] Release Notes Url: https://rn
//   [8] Digest Type: 1 (0x1)
//   [9] Digest: 813ca5285c28ccee5cab8b10ebda9c908fd6d78ed9dc94cc65ea6cb67a7f13ae
const uint8_t kOtaImage[] = { 0x1e, 0xf1, 0xee, 0x1b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
                              0x15, 0x25, 0x00, 0xad, 0xde, 0x25, 0x01, 0xef, 0xbe, 0x26, 0x02, 0xff, 0xff, 0xff, 0xff, 0x2c,
                              0x03, 0x03, 0x31, 0x2e, 0x30, 0x24, 0x04, 0x0c, 0x24, 0x05, 0x01, 0x24, 0x06, 0x02, 0x2c, 0x07,
                              0x0a, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f, 0x72, 0x6e, 0x24, 0x08, 0x01, 0x30, 0x09,
                              0x20, 0x81, 0x3c, 0xa5, 0x28, 0x5c, 0x28, 0xcc, 0xee, 0x5c, 0xab, 0x8b, 0x10, 0xeb, 0xda, 0x9c,
                              0x90, 0x8f, 0xd6, 0xd7, 0x8e, 0xd9, 0xdc, 0x94, 0xcc, 0x65, 0xea, 0x6c, 0xb6, 0x7a, 0x7f, 0x13,
                              0xae, 0x18, 0x74, 0x65, 0x73, 0x74, 0x20, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64 };

// Magic: 1beef11e
// Total Size: 68
// Header Size: 52
// Header TLV:
//   [0] Vendor Id: 1 (0x1)
//   [1] Product Id: 1 (0x1)
//   [2] Version: 1 (0x1)
//   [3] Version String: 1
//   [4] Payload Size: 0 (0x0)
//   [8] Digest Type: 9 (0x9)
//   [9] Digest: 6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7
const uint8_t kMinOtaImage[] = { 0x1e, 0xf1, 0xee, 0x1b, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00,
                                 0x00, 0x00, 0x15, 0x24, 0x00, 0x01, 0x24, 0x01, 0x01, 0x24, 0x02, 0x01, 0x2c, 0x03,
                                 0x01, 0x31, 0x24, 0x04, 0x00, 0x24, 0x08, 0x09, 0x30, 0x09, 0x1c, 0x6b, 0x4e, 0x03,
                                 0x42, 0x36, 0x67, 0xdb, 0xb7, 0x3b, 0x6e, 0x15, 0x45, 0x4f, 0x0e, 0xb1, 0xab, 0xd4,
                                 0x59, 0x7f, 0x9a, 0x1b, 0x07, 0x8e, 0x3f, 0x5b, 0x5a, 0x6b, 0xc7, 0x18 };

const uint8_t kMinOtaImageWithoutVendor[] = { 0x1e, 0xf1, 0xee, 0x1b, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31,
                                              0x00, 0x00, 0x00, 0x15, 0x24, 0x01, 0x01, 0x24, 0x02, 0x01, 0x2c, 0x03, 0x01,
                                              0x31, 0x24, 0x04, 0x00, 0x24, 0x08, 0x09, 0x30, 0x09, 0x1c, 0x6b, 0x4e, 0x03,
                                              0x42, 0x36, 0x67, 0xdb, 0xb7, 0x3b, 0x6e, 0x15, 0x45, 0x4f, 0x0e, 0xb1, 0xab,
                                              0xd4, 0x59, 0x7f, 0x9a, 0x1b, 0x07, 0x8e, 0x3f, 0x5b, 0x5a, 0x6b, 0xc7, 0x18 };

class TestOTAImageHeader : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestOTAImageHeader, TestHappyPath)
{
    ByteSpan buffer(kOtaImage);
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_NO_ERROR);
    EXPECT_TRUE(parser.IsInitialized());
    EXPECT_EQ(buffer.size(), strlen("test payload"));
    EXPECT_EQ(header.mVendorId, 0xDEAD);
    EXPECT_EQ(header.mProductId, 0xBEEF);
    EXPECT_EQ(header.mSoftwareVersion, 0xFFFFFFFF);
    EXPECT_TRUE(header.mSoftwareVersionString.data_equal("1.0"_span));
    EXPECT_EQ(header.mPayloadSize, strlen("test payload"));
    EXPECT_TRUE(header.mMinApplicableVersion.HasValue());
    EXPECT_EQ(header.mMinApplicableVersion.Value(), 1u);
    EXPECT_TRUE(header.mMaxApplicableVersion.HasValue());
    EXPECT_EQ(header.mMaxApplicableVersion.Value(), 2u);
    EXPECT_TRUE(header.mReleaseNotesURL.data_equal("https://rn"_span));
    EXPECT_EQ(header.mImageDigestType, OTAImageDigestType::kSha256);
    EXPECT_EQ(header.mImageDigest.size(), 256u / 8);
}

TEST_F(TestOTAImageHeader, TestEmptyBuffer)
{
    ByteSpan buffer{};
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_TRUE(parser.IsInitialized());
}

TEST_F(TestOTAImageHeader, TestInvalidFileIdentifier)
{
    static const uint8_t otaImage[] = { 0x1e, 0xf1, 0xee, 0x1c, 0x10, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    ByteSpan buffer(otaImage);
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    EXPECT_FALSE(parser.IsInitialized());
}

TEST_F(TestOTAImageHeader, TestTooSmallHeader)
{
    ByteSpan buffer(kMinOtaImage);
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_NO_ERROR);

    buffer = ByteSpan(kMinOtaImage, sizeof(kMinOtaImage) - 1);
    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_TRUE(parser.IsInitialized());
}

TEST_F(TestOTAImageHeader, TestMissingMandatoryField)
{
    ByteSpan buffer(kMinOtaImageWithoutVendor);
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    parser.Init();
    EXPECT_EQ(parser.AccumulateAndDecode(buffer, header), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    EXPECT_FALSE(parser.IsInitialized());
}

TEST_F(TestOTAImageHeader, TestSmallBlocks)
{
    constexpr size_t kImageSize = sizeof(kOtaImage);

    CHIP_ERROR error;
    OTAImageHeader header;
    OTAImageHeaderParser parser;

    for (size_t blockSize : { 1u, 16u, 128u })
    {
        error = CHIP_ERROR_BUFFER_TOO_SMALL;
        parser.Init();

        for (size_t offset = 0; offset < kImageSize && error == CHIP_ERROR_BUFFER_TOO_SMALL; offset += blockSize)
        {
            ByteSpan block(&kOtaImage[offset], chip::min(kImageSize - offset, blockSize));
            error = parser.AccumulateAndDecode(block, header);
        }

        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_TRUE(parser.IsInitialized());
        EXPECT_EQ(header.mVendorId, 0xDEAD);
        EXPECT_EQ(header.mProductId, 0xBEEF);
        EXPECT_EQ(header.mSoftwareVersion, 0xFFFFFFFF);
        EXPECT_TRUE(header.mSoftwareVersionString.data_equal("1.0"_span));
        EXPECT_EQ(header.mPayloadSize, strlen("test payload"));
        EXPECT_TRUE(header.mMinApplicableVersion.HasValue());
        EXPECT_EQ(header.mMinApplicableVersion.Value(), 1u);
        EXPECT_TRUE(header.mMaxApplicableVersion.HasValue());
        EXPECT_EQ(header.mMaxApplicableVersion.Value(), 2u);
        EXPECT_TRUE(header.mReleaseNotesURL.data_equal("https://rn"_span));
        EXPECT_EQ(header.mImageDigestType, OTAImageDigestType::kSha256);
        EXPECT_EQ(header.mImageDigest.size(), 256u / 8);
    }
}
} // namespace
