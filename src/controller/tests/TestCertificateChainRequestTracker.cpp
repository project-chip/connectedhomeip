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

#include <pw_unit_test/framework.h>

#include <controller/CertificateChainRequestTracker.h>

using namespace chip;
using namespace chip::Controller;

namespace {

TEST(CertificateChainRequestTracker, HandlesSingleResponse)
{
    CertificateChainRequestTracker tracker;

    const uint8_t certificateBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(certificateBytes), NullOptional, NullOptional), CHIP_NO_ERROR);
    EXPECT_TRUE(tracker.IsComplete());
    EXPECT_FALSE(tracker.IsSegmentedTransfer());
    EXPECT_TRUE(tracker.GetCertificate().data_equal(ByteSpan(certificateBytes)));
}

TEST(CertificateChainRequestTracker, ReassemblesSegmentedResponse)
{
    CertificateChainRequestTracker tracker;

    const uint8_t firstSegment[]  = { 0x10, 0x11, 0x12 };
    const uint8_t secondSegment[] = { 0x13, 0x14 };

    ASSERT_EQ(tracker.HandleResponse(ByteSpan(firstSegment), MakeOptional<uint16_t>(static_cast<uint16_t>(5)),
                                     MakeOptional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);
    EXPECT_FALSE(tracker.IsComplete());
    ASSERT_TRUE(tracker.NextSegmentId().HasValue());
    EXPECT_EQ(tracker.NextSegmentId().Value(), 1);

    ASSERT_EQ(tracker.HandleResponse(ByteSpan(secondSegment), MakeOptional<uint16_t>(static_cast<uint16_t>(5)), NullOptional),
              CHIP_NO_ERROR);
    EXPECT_TRUE(tracker.IsComplete());
    EXPECT_FALSE(tracker.HasPendingSegment());

    const uint8_t expected[] = { 0x10, 0x11, 0x12, 0x13, 0x14 };
    EXPECT_TRUE(tracker.GetCertificate().data_equal(ByteSpan(expected)));
}

TEST(CertificateChainRequestTracker, ReassemblesCertificateLargerThanLegacyLimit)
{
    // A legacy PAI subject key may have an ML-DSA issuer signature. Response metadata,
    // rather than the request profile, determines how many segments to assemble.
    CertificateChainRequestTracker tracker;
    uint8_t certificate[1300];
    for (size_t i = 0; i < sizeof(certificate); ++i)
    {
        certificate[i] = static_cast<uint8_t>(i);
    }
    const ByteSpan document(certificate);
    constexpr uint16_t segmentSize = CertificateChainRequestTracker::kDefaultSegmentSize;
    for (uint16_t segment = 0; segment < 3; ++segment)
    {
        const size_t offset = static_cast<size_t>(segment) * segmentSize;
        const bool last     = segment == 2;
        ASSERT_EQ(tracker.HandleResponse(document.SubSpan(offset, last ? sizeof(certificate) - offset : segmentSize),
                                         MakeOptional<uint16_t>(static_cast<uint16_t>(sizeof(certificate))),
                                         last ? NullOptional : MakeOptional<uint16_t>(static_cast<uint16_t>(segment + 1))),
                  CHIP_NO_ERROR);
        EXPECT_EQ(tracker.IsComplete(), last);
    }
    EXPECT_TRUE(tracker.GetCertificate().data_equal(document));
}

TEST(CertificateChainRequestTracker, RejectsEmptyFirstSegmentWithoutChangingState)
{
    CertificateChainRequestTracker tracker;
    const auto totalSize = MakeOptional<uint16_t>(static_cast<uint16_t>(2));
    const auto nextId    = MakeOptional<uint16_t>(static_cast<uint16_t>(1));

    // A continuation must make progress instead of allowing an unbounded sequence of empty responses.
    EXPECT_EQ(tracker.HandleResponse(ByteSpan(), totalSize, nextId), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(tracker.IsComplete());
    EXPECT_FALSE(tracker.IsSegmentedTransfer());
    EXPECT_FALSE(tracker.HasPendingSegment());
    EXPECT_TRUE(tracker.GetCertificate().empty());

    const uint8_t certificate[] = { 0x10, 0x11 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(certificate), totalSize, NullOptional), CHIP_NO_ERROR);
    EXPECT_TRUE(tracker.IsComplete());
    EXPECT_TRUE(tracker.GetCertificate().data_equal(ByteSpan(certificate)));
}

TEST(CertificateChainRequestTracker, RejectsEmptyContinuationWithoutChangingState)
{
    CertificateChainRequestTracker tracker;
    const auto totalSize         = MakeOptional<uint16_t>(static_cast<uint16_t>(3));
    const uint8_t firstSegment[] = { 0x10 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(firstSegment), totalSize, MakeOptional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);

    EXPECT_EQ(tracker.HandleResponse(ByteSpan(), totalSize, MakeOptional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(tracker.IsComplete());
    EXPECT_TRUE(tracker.IsSegmentedTransfer());
    ASSERT_TRUE(tracker.NextSegmentId().HasValue());
    EXPECT_EQ(tracker.NextSegmentId().Value(), 1);
    EXPECT_TRUE(tracker.GetCertificate().empty());

    // Continuing at the same segment ID proves that the rejected response consumed neither an ID nor bytes.
    const uint8_t secondSegment[] = { 0x11 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(secondSegment), totalSize, MakeOptional<uint16_t>(static_cast<uint16_t>(2))),
              CHIP_NO_ERROR);
    const uint8_t finalSegment[] = { 0x12 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(finalSegment), totalSize, NullOptional), CHIP_NO_ERROR);
    EXPECT_TRUE(tracker.IsComplete());
    EXPECT_FALSE(tracker.HasPendingSegment());
    const uint8_t expected[] = { 0x10, 0x11, 0x12 };
    EXPECT_TRUE(tracker.GetCertificate().data_equal(ByteSpan(expected)));
}

TEST(CertificateChainRequestTracker, RejectsInconsistentSegmentedResponse)
{
    CertificateChainRequestTracker tracker;

    const uint8_t firstSegment[] = { 0x20, 0x21, 0x22 };
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(firstSegment), MakeOptional<uint16_t>(static_cast<uint16_t>(5)),
                                     MakeOptional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);

    const uint8_t badSecondSegment[] = { 0x23 };
    EXPECT_EQ(tracker.HandleResponse(ByteSpan(badSecondSegment), MakeOptional<uint16_t>(static_cast<uint16_t>(6)), NullOptional),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(CertificateChainRequestTracker, AcceptsMaximumSupportedCertificateSize)
{
    CertificateChainRequestTracker tracker;
    uint8_t certificate[Credentials::kMaxDERCertLengthMlDsa65] = {};
    ASSERT_EQ(tracker.HandleResponse(ByteSpan(certificate), MakeOptional<uint16_t>(static_cast<uint16_t>(sizeof(certificate))),
                                     NullOptional),
              CHIP_NO_ERROR);
    EXPECT_TRUE(tracker.IsComplete());
    EXPECT_TRUE(tracker.GetCertificate().data_equal(ByteSpan(certificate)));
}

TEST(CertificateChainRequestTracker, EnforcesSubjectAndIssuerBoundsBeforeAcceptingResponse)
{
    using Profile = CertificateChainRequestTracker::CryptoProfile;
    struct TestCase
    {
        Profile subject;
        Profile issuer;
        uint16_t limit;
    };
    const TestCase cases[] = {
        { Profile::kEcdsaMatterLegacy, Profile::kEcdsaMatterLegacy, 600 },
        { Profile::kEcdsaMatterLegacy, Profile::kMlDsa44, 4732 },
        { Profile::kMlDsa44, Profile::kEcdsaMatterLegacy, 4732 },
        { Profile::kMlDsa44, Profile::kMlDsa44, 4732 },
        { Profile::kEcdsaMatterLegacy, Profile::kMlDsa65, 6261 },
        { Profile::kMlDsa44, Profile::kMlDsa65, 6261 },
        { Profile::kMlDsa65, Profile::kMlDsa44, 6261 },
        { Profile::kMlDsa65, Profile::kMlDsa65, 6261 },
        { Profile::kUnknownEnumValue, Profile::kMlDsa44, 6261 },
        { Profile::kMlDsa44, Profile::kUnknownEnumValue, 6261 },
    };
    uint8_t certificate[Credentials::kMaxDERCertLengthMlDsa65 + 1] = {};
    for (const auto & test : cases)
    {
        for (const bool segmented : { false, true })
        {
            CertificateChainRequestTracker tracker;
            tracker.Reset(test.subject, test.issuer);
            const auto total = segmented ? MakeOptional<uint16_t>(static_cast<uint16_t>(test.limit + 1)) : NullOptional;
            // Reject the advertised total on the first byte, before accepting any state.
            EXPECT_EQ(tracker.HandleResponse(ByteSpan(certificate, segmented ? 1 : test.limit + 1), total,
                                             segmented ? MakeOptional<uint16_t>(static_cast<uint16_t>(1)) : NullOptional),
                      CHIP_ERROR_MESSAGE_TOO_LONG);
            EXPECT_FALSE(tracker.IsComplete());
            EXPECT_FALSE(tracker.IsSegmentedTransfer());
            EXPECT_FALSE(tracker.HasPendingSegment());
            EXPECT_TRUE(tracker.GetCertificate().empty());
            ASSERT_EQ(tracker.HandleResponse(ByteSpan(certificate, test.limit), segmented ? MakeOptional(test.limit) : NullOptional,
                                             NullOptional),
                      CHIP_NO_ERROR);
            EXPECT_EQ(tracker.GetCertificate().size(), test.limit);
        }
    }
}

TEST(CertificateChainRequestTracker, ResetRestoresDefaultBound)
{
    using Profile = CertificateChainRequestTracker::CryptoProfile;
    CertificateChainRequestTracker tracker;
    tracker.Reset(Profile::kEcdsaMatterLegacy, Profile::kEcdsaMatterLegacy);
    tracker.Reset();
    const uint8_t firstByte[] = { 1 };
    EXPECT_EQ(tracker.HandleResponse(ByteSpan(firstByte), MakeOptional<uint16_t>(static_cast<uint16_t>(6261)),
                                     MakeOptional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_NO_ERROR);
}

TEST(CertificateChainRequestTracker, RejectsOversizedDocument)
{
    CertificateChainRequestTracker tracker;

    const uint8_t firstSegment[] = { 0x42 };
    EXPECT_EQ(tracker.HandleResponse(
                  ByteSpan(firstSegment),
                  MakeOptional<uint16_t>(static_cast<uint16_t>(CertificateChainRequestTracker::kMaxCertificateDocumentSize + 1)),
                  MakeOptional<uint16_t>(static_cast<uint16_t>(1))),
              CHIP_ERROR_MESSAGE_TOO_LONG);
}

TEST(CertificateChainRequestTracker, RejectsOversizedSingleResponse)
{
    CertificateChainRequestTracker tracker;
    uint8_t oversizedCertificate[CertificateChainRequestTracker::kMaxCertificateDocumentSize + 1] = {};

    EXPECT_EQ(tracker.HandleResponse(ByteSpan(oversizedCertificate), NullOptional, NullOptional), CHIP_ERROR_MESSAGE_TOO_LONG);
}

} // namespace
