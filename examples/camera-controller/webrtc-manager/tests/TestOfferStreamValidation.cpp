/*
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

#include <webrtc-manager/OfferStreamValidation.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>

#include <numeric>

namespace {

using namespace chip;

using StreamId   = Optional<app::DataModel::Nullable<uint16_t>>;
using StreamList = std::optional<std::vector<uint16_t>>;

constexpr bool kNewSession = true;
constexpr bool kReOffer    = false;

const StreamId kNoStreamId;
const StreamList kNoStreamList;

StreamId MakeStreamId(uint16_t id)
{
    return MakeOptional(app::DataModel::MakeNullable(id));
}

StreamList MakeStreamList(size_t length)
{
    std::vector<uint16_t> streams(length);
    std::iota(streams.begin(), streams.end(), static_cast<uint16_t>(1));
    return streams;
}

TEST(TestOfferStreamValidation, AcceptsSingleStreamIds)
{
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), kNoStreamId, kNoStreamList, kNoStreamList, kNewSession), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, MakeStreamId(2), kNoStreamList, kNoStreamList, kNewSession), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), MakeStreamId(2), kNoStreamList, kNoStreamList, kNewSession), CHIP_NO_ERROR);

    // A null stream ID asks the provider to choose the stream, so it counts as present.
    const StreamId nullStreamId = MakeOptional(app::DataModel::Nullable<uint16_t>());
    EXPECT_EQ(ValidateOfferStreams(nullStreamId, kNoStreamId, kNoStreamList, kNoStreamList, kNewSession), CHIP_NO_ERROR);
}

TEST(TestOfferStreamValidation, AcceptsStreamListsOfOneToSixteenEntries)
{
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(1), kNoStreamList, kNewSession), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, kNoStreamList, MakeStreamList(16), kNewSession), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(16), MakeStreamList(1), kNewSession), CHIP_NO_ERROR);
}

TEST(TestOfferStreamValidation, RejectsOfferWithoutStreamFields)
{
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, kNoStreamList, kNoStreamList, kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, kNoStreamList, kNoStreamList, kReOffer), CHIP_ERROR_INVALID_ARGUMENT);
}

// VideoStreams and AudioStreams are constrained to 1 to 16 entries, so each present list is checked even when the other
// stream field is valid.
TEST(TestOfferStreamValidation, RejectsEachStreamListOutsideOneToSixteenEntries)
{
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(0), kNoStreamList, kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, kNoStreamList, MakeStreamList(0), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(17), kNoStreamList, kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, kNoStreamList, MakeStreamList(17), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(1), MakeStreamList(0), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(17), MakeStreamList(1), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
}

// The provider rejects a new session that uses both the single stream IDs and the stream lists with INVALID_COMMAND.
TEST(TestOfferStreamValidation, RejectsMixedStreamFormatsForNewSession)
{
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), kNoStreamId, MakeStreamList(1), kNoStreamList, kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), kNoStreamId, kNoStreamList, MakeStreamList(1), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, MakeStreamId(2), MakeStreamList(1), kNoStreamList, kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, MakeStreamId(2), kNoStreamList, MakeStreamList(1), kNewSession),
              CHIP_ERROR_INVALID_ARGUMENT);
}

// The provider ignores the stream fields of a ProvideOffer re-offer, so the formats may be combined there.
TEST(TestOfferStreamValidation, AllowsMixedStreamFormatsOnReOffer)
{
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), kNoStreamId, MakeStreamList(1), kNoStreamList, kReOffer), CHIP_NO_ERROR);
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), MakeStreamId(2), MakeStreamList(1), MakeStreamList(1), kReOffer),
              CHIP_NO_ERROR);
}

TEST(TestOfferStreamValidation, ChecksStreamListLengthsOnReOffer)
{
    EXPECT_EQ(ValidateOfferStreams(kNoStreamId, kNoStreamId, MakeStreamList(0), kNoStreamList, kReOffer),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(ValidateOfferStreams(MakeStreamId(1), kNoStreamId, kNoStreamList, MakeStreamList(17), kReOffer),
              CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
