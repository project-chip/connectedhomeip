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

TEST(CertificateChainRequestTracker, RejectsOversizedDocument)
{
    CertificateChainRequestTracker tracker;

    const uint8_t firstSegment[] = { 0x42 };
    EXPECT_EQ(tracker.HandleResponse(ByteSpan(firstSegment),
                                     MakeOptional<uint16_t>(static_cast<uint16_t>(
                                         CertificateChainRequestTracker::kMaxCertificateDocumentSize + 1)),
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
